#include "Camera.h"
#include "Config.h"
#include "Storage.h" // <--- Importante para guardar al detener

unsigned long previousMillis = 0;
unsigned long tiempoInicioSesion = 0; // <--- Para medir cuánto duró la sesión activa
int fotosTomadasSesion = 0;

void initCamera() {
  pinMode(PIN_RELAY_SHUTTER, OUTPUT);
  pinMode(PIN_RELAY_FOCUS, OUTPUT);
  digitalWrite(PIN_RELAY_SHUTTER, HIGH); // Active LOW -> Apagado
  digitalWrite(PIN_RELAY_FOCUS, HIGH);
}

void iniciarSecuencia() {
  camState = CAM_ESPERA_INICIAL;
  previousMillis = millis();
  tiempoInicioSesion = millis(); // Guardamos el momento exacto de inicio
  fotosTomadasSesion = 0;
}

void detenerSecuencia() {
  if (camState == CAM_DETENIDO) return;

  camState = CAM_DETENIDO;
  digitalWrite(PIN_RELAY_SHUTTER, HIGH);
  digitalWrite(PIN_RELAY_FOCUS, HIGH);
  
  // --- ACTUALIZACIÓN DE ESTADÍSTICAS HISTÓRICAS ---
  // 1. Sumamos las fotos de esta sesión al total histórico
  statsGlobales.totalFotosHistorico += fotosTomadasSesion;
  
  // 2. Calculamos los segundos que estuvo corriendo y los sumamos
  unsigned long segundosCorriendo = (millis() - tiempoInicioSesion) / 1000UL;
  statsGlobales.tiempoTotalFunc += segundosCorriendo;
  
  // 3. Guardamos en EEPROM una sola vez al terminar la noche
  guardarEstadisticas();
}

void updateCamera() {
  if (camState == CAM_DETENIDO) return;

  unsigned long currentMillis = millis();

  switch (camState) {
    case CAM_ESPERA_INICIAL:
      if (currentMillis - previousMillis >= 3000) { // 3 segundos de espera inicial
        camState = CAM_ENFOCANDO;
        digitalWrite(PIN_RELAY_FOCUS, LOW); // Enciende foco
        previousMillis = currentMillis;
      }
      break;

    case CAM_ENFOCANDO:
      if (currentMillis - previousMillis >= 1000) { // 1 segundo de foco
        camState = CAM_OBTURANDO;
        digitalWrite(PIN_RELAY_SHUTTER, LOW); // Dispara foto
        previousMillis = currentMillis;
      }
      break;

    case CAM_OBTURANDO:
      if (currentMillis - previousMillis >= (perfilActual.tiempoObturacion * 1000UL)) {
        camState = CAM_INTERVALO;
        digitalWrite(PIN_RELAY_SHUTTER, HIGH); // Apaga disparo
        digitalWrite(PIN_RELAY_FOCUS, HIGH);   // Apaga foco
        fotosTomadasSesion++;
        previousMillis = currentMillis;

        // Comprobar límite de fotos
        if (perfilActual.limiteFotos > 0 && fotosTomadasSesion >= perfilActual.limiteFotos) {
          detenerSecuencia();
        }
      }
      break;

    case CAM_INTERVALO:
      if (currentMillis - previousMillis >= (perfilActual.tiempoEntreFotos * 1000UL)) {
        camState = CAM_ENFOCANDO;
        digitalWrite(PIN_RELAY_FOCUS, LOW);
        previousMillis = currentMillis;
      }
      break;
  }
}