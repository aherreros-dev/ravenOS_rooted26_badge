#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Definiciones de la Pantalla (según tus capturas)
#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_CS     5   // ¡Era el 5!
#define TFT_DC    16
#define TFT_RST    4
#define TFT_BL    12   // Luz en el 12 (Activo Bajo)
#define SCREEN_EN 21   // Pin de habilitación (Activo Alto)

// Definiciones de Botones (según tus capturas y tu aclaración)
#define BTN_UP     27
#define BTN_DOWN   15
#define BTN_LEFT   25
#define BTN_RIGHT  26
#define BTN_SELECT 13
#define BTN_EXTRA  33  // El sexto botón

// Usamos el constructor que permite reasignar pines SPI
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);
  Serial.println("Inicializando Badge con pinout oficial...");

  // 1. Configurar Pines de Energía y Luz
  pinMode(SCREEN_EN, OUTPUT);
  digitalWrite(SCREEN_EN, HIGH); // Habilitar pantalla
  
  // Pequeña pausa para que la energía se estabilice
  delay(100);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);    // LOW = Luz ON (Activo Bajo)

  // 2. Configurar Pines de Botones (INPUT_PULLUP, Active LOW)
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BTN_EXTRA, INPUT_PULLUP); // Sexto botón

  // 3. Inicializar Pantalla
  tft.initR(INITR_BLACKTAB);    // Blacktab suele ir mejor en estas pantallas
  tft.setRotation(3);           // Orientación oficial según tu captura
  tft.fillScreen(ST77XX_BLACK);

  // 4. Implementar Petición Específica: "redr4ven" en ROJO
  tft.setTextSize(2);           // Texto de buen tamaño
  tft.setTextColor(ST77XX_RED); // Color rojo puro

  // Centrar el texto
  String texto = "redr4ven";
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(texto, 0, 0, &x1, &y1, &w, &h); // Calcular dimensiones del texto
  int posX = (tft.width() - w) / 2;
  int posY = (tft.height() - h) / 2;
  
  tft.setCursor(posX, posY);
  tft.println(texto);

  // Línea decorativa
  tft.drawFastHLine(posX, posY + h + 5, w, ST77XX_WHITE);
  
  Serial.println("Inicialización completada.");
}

void loop() {
  // Zona para mostrar el estado de los botones
  tft.fillRect(0, 0, tft.width(), 30, ST77XX_BLACK); // Limpiar zona de cabecera
  tft.setCursor(5, 5);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("STATUS: ONLINE | PROUDLY PWNED BY");

  // Sección de Botones en la parte inferior
  tft.fillRect(0, 100, tft.width(), 28, ST77XX_BLACK); // Limpiar zona de botones
  tft.setCursor(5, 105);
  tft.setTextColor(ST77XX_YELLOW);

  String botonesPresionados = "";
  if (digitalRead(BTN_UP) == LOW)    botonesPresionados += "UP ";
  if (digitalRead(BTN_DOWN) == LOW)  botonesPresionados += "DOWN ";
  if (digitalRead(BTN_LEFT) == LOW)  botonesPresionados += "LEFT ";
  if (digitalRead(BTN_RIGHT) == LOW) botonesPresionados += "RIGHT ";
  if (digitalRead(BTN_SELECT) == LOW) botonesPresionados += "SELECT ";
  if (digitalRead(BTN_EXTRA) == LOW)  botonesPresionados += "EXTRA ";

  if (botonesPresionados != "") {
    tft.print("BOTON(ES): ");
    tft.println(botonesPresionados);
  }

  delay(100); 
}