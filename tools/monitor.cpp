#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Pines SPI por defecto en ESP32 (Asumimos que el fabricante usó estos)
#define TFT_MOSI 23
#define TFT_SCLK 18

// Pines candidatos para CS, DC y RST (excluyendo los botones ya descubiertos y entradas)
int posiblesPines[] = {2, 4, 5, 12, 14, 16, 17, 19, 21, 22, 32};
int numPines = 11;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- INICIANDO FUZZING DE PANTALLA ---");
  Serial.println("¡MIRA ATENTAMENTE LA PANTALLA DEL BADGE!");
  Serial.println("El script probará combinaciones. Si la pantalla se pone ROJA,");
  Serial.println("mira rápidamente el Monitor Serie para ver qué pines fueron.");
  Serial.println("Empezando en 3 segundos...");
  delay(3000);
}

void loop() {
  for (int i = 0; i < numPines; i++) {
    for (int j = 0; j < numPines; j++) {
      for (int k = 0; k < numPines; k++) {
        
        // Evitar que CS, DC y RST intenten usar el mismo pin a la vez
        if (i == j || i == k || j == k) continue;

        int cs = posiblesPines[i];
        int dc = posiblesPines[j];
        int rst = posiblesPines[k];

        Serial.print("Disparando -> CS: "); Serial.print(cs);
        Serial.print(" | DC: "); Serial.print(dc);
        Serial.print(" | RST: "); Serial.println(rst);

        // Creamos el objeto de la pantalla de forma temporal con estos pines
        Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);

        // Inicializamos la pantalla (INITR_BLACKTAB es el estándar para estas pantallas de 1.77")
        tft.initR(INITR_BLACKTAB); 
        
        // ¡FUEGO! Intentamos pintar toda la pantalla de rojo puro
        tft.fillScreen(ST77XX_RED);

        // Dejamos un cuarto de segundo para que te dé tiempo a ver el flasheo rojo con tus ojos
        delay(250);
        
        // Devolvemos los pines a su estado normal para no interferir con el siguiente intento
        pinMode(cs, INPUT);
        pinMode(dc, INPUT);
        pinMode(rst, INPUT);
      }
    }
  }
  
  Serial.println("--- BUCLE COMPLETADO. REINICIANDO TODAS LAS COMBINACIONES ---");
  delay(3000);
}