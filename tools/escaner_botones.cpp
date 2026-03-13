#include <Arduino.h>

// Lista de pines sospechosos. Incluimos los de entrada pura (34-39) 
// y otros GPIOs comunes donde se suelen conectar botones.
const int numPins = 18;
int pinsToTest[numPins] = {0, 2, 4, 12, 13, 14, 15, 25, 26, 27, 32, 33, 34, 35, 36, 37, 38, 39};

// Array para guardar el estado anterior de cada pin y comparar
int lastState[numPins];

void setup() {
  // Iniciamos la comunicación serie a 115200 baudios
  Serial.begin(115200);
  delay(1000); // Pequeña pausa para que abra el puerto
  Serial.println("\n--- ESCÁNER DE BOTONES INICIADO ---");
  Serial.println("Pulsa los botones de la cruceta y los demás...");

  // Configuramos los pines
  for (int i = 0; i < numPins; i++) {
    // Los pines 34 a 39 no tienen pull-up interno según el datasheet
    if (pinsToTest[i] >= 34 && pinsToTest[i] <= 39) {
      pinMode(pinsToTest[i], INPUT);
    } else {
      // El resto los configuramos con pull-up por si acaso
      pinMode(pinsToTest[i], INPUT_PULLUP);
    }
    // Leemos el estado inicial en reposo
    lastState[i] = digitalRead(pinsToTest[i]);
  }
}

void loop() {
  // Bucle infinito interrogando a los pines
  for (int i = 0; i < numPins; i++) {
    int currentState = digitalRead(pinsToTest[i]);
    
    // Si el estado actual es diferente al que tenía antes... ¡Alguien ha pulsado!
    if (currentState != lastState[i]) {
      Serial.print("¡BINGO! Cambio detectado en el PIN: ");
      Serial.print(pinsToTest[i]);
      Serial.print(" | Nuevo estado eléctrico: ");
      Serial.println(currentState);
      
      lastState[i] = currentState; // Actualizamos el estado
      delay(200); // Un pequeño "debounce" (pausa) para evitar rebotes eléctricos
    }
  }
  delay(10); // Pausa mínima para no saturar el procesador
}