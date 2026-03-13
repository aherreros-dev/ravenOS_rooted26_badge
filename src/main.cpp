#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_BL   21
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   2
#define TFT_DC   16
#define TFT_RST  4

bool done = false;

void setup() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  delay(1000);
}

void loop() {
  if (done) return;
  done = true;

  // Exactamente igual que el escaneo que funcionó
  Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLUE);
  tft.setCursor(5, 40);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.printf("CS:%d\nDC:%d\nRST:%d", TFT_CS, TFT_DC, TFT_RST);

  delay(500);
}