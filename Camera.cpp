#include "Camera.h"
#include "Config.h"
#include "Storage.h"

unsigned long previousMillis = 0;
unsigned long tiempoInicioSesion = 0; 
int fotosTomadasSesion = 0;

#ifndef LED_BUILTIN
#define LED_BUILTIN 2 
#endif

// Variables para manejar los pulsos manuales de calibración
unsigned long timerManualFoco = 0;
bool focoManualActivo = false;
unsigned long timerManualFoto = 0;
bool fotoManualActiva = false;

void initCamera() {
  pinMode(PIN_RELAY_SHUTTER, OUTPUT);
  pinMode(PIN_RELAY_FOCUS, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(PIN_RELAY_SHUTTER, HIGH); // Active LOW -> Apagado
  digitalWrite(PIN_RELAY_FOCUS, HIGH);
  digitalWrite(LED_BUILTIN, LOW); 
}

void iniciarSecuencia() {
  camState = CAM_OBTURANDO; // Arrancamos directo a sacar foto, sin delay inicial
  previousMillis = millis();
  tiempoInicioSesion = millis(); 
  fotosTomadasSesion = 0;
  
  digitalWrite(PIN_RELAY_SHUTTER, LOW); 
  digitalWrite(LED_BUILTIN, HIGH); 
}

void detenerSecuencia() {
  if (camState == CAM_DETENIDO) return;

  camState = CAM_DETENIDO;
  digitalWrite(PIN_RELAY_SHUTTER, HIGH);
  digitalWrite(PIN_RELAY_FOCUS, HIGH);
  digitalWrite(LED_BUILTIN, LOW); 
  
  statsGlobales.totalFotosHistorico += fotosTomadasSesion;
  unsigned long segundosCorriendo = (millis() - tiempoInicioSesion) / 1000UL;
  statsGlobales.tiempoTotalFunc += segundosCorriendo;
  guardarEstadisticas();
}

// --- FUNCIONES DE CALIBRACIÓN MANUAL ---
void forzarFoto() {
  if (camState != CAM_DETENIDO) return; // Por seguridad, solo funciona si la secuencia está detenida
  digitalWrite(PIN_RELAY_SHUTTER, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
  fotoManualActiva = true;
  timerManualFoto = millis();
}

void forzarFoco() {
  if (camState != CAM_DETENIDO) return;
  digitalWrite(PIN_RELAY_FOCUS, LOW);
  focoManualActivo = true;
  timerManualFoco = millis();
}

void updateCamera() {
  unsigned long currentMillis = millis();

  // 1. Lógica Automática (Secuencia)
  if (camState != CAM_DETENIDO) {
    switch (camState) {
      case CAM_OBTURANDO:
        if (currentMillis - previousMillis >= (perfilActual.tiempoObturacion * 1000UL)) {
          camState = CAM_INTERVALO;
          digitalWrite(PIN_RELAY_SHUTTER, HIGH); 
          digitalWrite(LED_BUILTIN, LOW); 
          
          fotosTomadasSesion++;
          previousMillis = currentMillis;

          if (perfilActual.limiteFotos > 0 && fotosTomadasSesion >= perfilActual.limiteFotos) {
            detenerSecuencia();
          }
        }
        break;

      case CAM_INTERVALO:
        if (currentMillis - previousMillis >= (perfilActual.tiempoEntreFotos * 1000UL)) {
          camState = CAM_OBTURANDO;
          digitalWrite(PIN_RELAY_SHUTTER, LOW);
          digitalWrite(LED_BUILTIN, HIGH); 
          previousMillis = currentMillis;
        }
        break;
    }
  }

  // 2. Lógica Manual (Apaga los relés tras 2 segundos)
  if (focoManualActivo && (currentMillis - timerManualFoco >= 2000)) {
    digitalWrite(PIN_RELAY_FOCUS, HIGH);
    focoManualActivo = false;
  }
  
  if (fotoManualActiva && (currentMillis - timerManualFoto >= 2000)) {
    digitalWrite(PIN_RELAY_SHUTTER, HIGH);
    digitalWrite(LED_BUILTIN, LOW);
    fotoManualActiva = false;
  }
}
