#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Pines que NO cambian (Luz y Datos)
#define TFT_BL   21
#define TFT_MOSI 23
#define TFT_SCLK 18

// Candidatos (hemos añadido el 32 y otros que suelen ser usados)
int p[] = {2, 4, 5, 12, 14, 16, 17, 19, 22, 27, 32, 33};
int n = 12;

void setup() {
  Serial.begin(115200);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH); // Luz siempre encendida
  delay(1000);
  Serial.println("--- INICIANDO ESCANEO MAESTRO ---");
}

void loop() {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      for (int k = 0; k < n; k++) {
        if (i == j || i == k || j == k) continue;

        int cs = p[i];
        int dc = p[j];
        int rst = p[k];

        // Imprimimos en el PC qué estamos intentando
        Serial.printf("PROBANDO -> CS:%d DC:%d RST:%d\n", cs, dc, rst);

        // Creamos la pantalla
        Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, TFT_MOSI, TFT_SCLK, rst);
        
        // Inicialización lenta
        tft.initR(INITR_BLACKTAB);
        
        // Intentamos pintar y escribir
        tft.fillScreen(ST77XX_BLUE); 
        tft.setCursor(5, 40);
        tft.setTextColor(ST77XX_WHITE);
        tft.setTextSize(2);
        tft.printf("CS:%d\nDC:%d\nRST:%d", cs, dc, rst);

        // Esperamos medio segundo. Si la pantalla funciona, 
        // verás los números escritos en blanco sobre fondo azul.
        delay(500);

        // Limpieza total de pines para evitar conflictos en la siguiente vuelta
        pinMode(cs, INPUT);
        pinMode(dc, INPUT);
        pinMode(rst, INPUT);
      }
    }
  }
}