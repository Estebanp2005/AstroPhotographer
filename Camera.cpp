#include "Camera.h"
#include "Config.h"
#include "Storage.h"
#include "Display.h"

CameraState camState = CAM_DETENIDO;
int fotosTomadasSesion = 0;
unsigned long tiempoInicioSesion = 0;
unsigned long duracionUltimaSesion = 0; // <--- NUEVA VARIABLE GLOBAL

unsigned long ultimoEventoCamera = 0;

void initCamera() {
  pinMode(PIN_RELAY_SHUTTER, OUTPUT);
  pinMode(PIN_RELAY_FOCUS, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  digitalWrite(PIN_RELAY_SHUTTER, LOW);
  digitalWrite(PIN_RELAY_FOCUS, LOW);
  digitalWrite(LED_BUILTIN, HIGH); 
}

void iniciarSecuencia() {
  fotosTomadasSesion = 0;
  duracionUltimaSesion = 0; // <--- REINICIAMOS EL CRONOMETRO FINAL
  tiempoInicioSesion = millis();
  ultimoEventoCamera = millis();
  
  camState = CAM_ESPERA_INICIAL; 
}

void detenerSecuencia() {
  digitalWrite(PIN_RELAY_SHUTTER, LOW);
  digitalWrite(PIN_RELAY_FOCUS, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
  
  camState = CAM_DETENIDO;

  // Calculamos el tiempo de uso al momento EXACTO de frenar
  duracionUltimaSesion = millis() - tiempoInicioSesion; // <--- GUARDAMOS EL FINAL AQUI
  
  statsGlobales.tiempoTotalFunc += duracionUltimaSesion;
  statsGlobales.totalFotosHistorico += fotosTomadasSesion;
  
  guardarEstadisticas();
}

void updateCamera() {
  if (camState == CAM_DETENIDO) return;

  unsigned long currentMillis = millis();

  switch (camState) {
    case CAM_ESPERA_INICIAL:
      if (currentMillis - ultimoEventoCamera >= 3000) {
        camState = CAM_OBTURANDO;
        ultimoEventoCamera = currentMillis;
        
        digitalWrite(PIN_RELAY_SHUTTER, HIGH);
        digitalWrite(LED_BUILTIN, LOW); 
      }
      break;

    case CAM_OBTURANDO:
      if (currentMillis - ultimoEventoCamera >= (perfilActual.tiempoObturacion * 1000UL)) {
        
        digitalWrite(PIN_RELAY_SHUTTER, LOW);
        digitalWrite(LED_BUILTIN, HIGH); 
        
        fotosTomadasSesion++;
        ultimoEventoCamera = currentMillis;

        if (perfilActual.limiteFotos > 0 && fotosTomadasSesion >= perfilActual.limiteFotos) {
          detenerSecuencia(); 
        } else {
          camState = CAM_INTERVALO; 
        }
      }
      break;

    case CAM_INTERVALO:
      if (currentMillis - ultimoEventoCamera >= (perfilActual.tiempoEntreFotos * 1000UL)) {
        camState = CAM_OBTURANDO;
        ultimoEventoCamera = currentMillis;
        
        digitalWrite(PIN_RELAY_SHUTTER, HIGH);
        digitalWrite(LED_BUILTIN, LOW); 
      }
      break;
  }
}

void forzarFoto() {
  if (camState != CAM_DETENIDO) return; 
  digitalWrite(PIN_RELAY_SHUTTER, HIGH);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500); 
  digitalWrite(PIN_RELAY_SHUTTER, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
}

void forzarFoco() {
  if (camState != CAM_DETENIDO) return; 
  digitalWrite(PIN_RELAY_FOCUS, HIGH);
  delay(500); 
  digitalWrite(PIN_RELAY_FOCUS, LOW);
}
