/**
 * @file Camera.cpp
 * @brief Implementation of the camera control and state machine.
 * * This file handles the physical triggering of the camera (focus and shutter)
 * via hardware relays. It uses a non-blocking state machine based on millis()
 * to ensure the microcontroller remains responsive during long exposures.
 */

#include "Camera.h"
#include "Config.h"
#include "Storage.h"
#include "Display.h"

// --- Global Variables ---

/** @brief Current state of the camera sequence (e.g., STOPPED, WAITING, EXPOSING). */
CameraState camState = CAM_DETENIDO;

/** @brief Counter for the number of photos successfully taken in the current session. */
int fotosTomadasSesion = 0;

/** @brief Timestamp (in ms) when the current shooting session started. */
unsigned long tiempoInicioSesion = 0;

/** @brief Total duration (in ms) of the last completed session. */
unsigned long duracionUltimaSesion = 0; 

/** @brief Timestamp (in ms) of the last state change in the camera sequence. */
unsigned long ultimoEventoCamera = 0;

// --- Core Functions ---

/**
 * @brief Initializes the GPIO pins for the camera relays and buzzer.
 */
void initCamera() {
  pinMode(PIN_RELAY_SHUTTER, OUTPUT);
  pinMode(PIN_RELAY_FOCUS, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT); 
  
  // INVERSE LOGIC: Standard relay modules activate on LOW and deactivate on HIGH.
  digitalWrite(PIN_RELAY_SHUTTER, HIGH); 
  digitalWrite(PIN_RELAY_FOCUS, HIGH);
  digitalWrite(LED_BUILTIN, HIGH); 
  
  // NORMAL LOGIC: Buzzer is active HIGH.
  digitalWrite(PIN_BUZZER, LOW); 
}

/**
 * @brief Starts a new astrophotography sequence.
 * Resets session counters and transitions to the initial wait state.
 */
void iniciarSecuencia() {
  fotosTomadasSesion = 0;
  duracionUltimaSesion = 0; 
  tiempoInicioSesion = millis();
  ultimoEventoCamera = millis();
  
  camState = CAM_ESPERA_INICIAL; 
}

/**
 * @brief Safely halts the current sequence and turns off all relays.
 * Calculates session duration, updates global statistics, and saves them to NVRAM.
 */
void detenerSecuencia() {
  // Ensure relays and LEDs are explicitly turned OFF (HIGH for inverse logic)
  digitalWrite(PIN_RELAY_SHUTTER, HIGH);
  digitalWrite(PIN_RELAY_FOCUS, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  
  camState = CAM_DETENIDO;

  // Calculate session duration and update global stats
  duracionUltimaSesion = (unsigned long)(millis() - tiempoInicioSesion); 
  statsGlobales.tiempoTotalFunc += duracionUltimaSesion;
  statsGlobales.totalFotosHistorico += fotosTomadasSesion;
  
  // Persist new totals to EEPROM/LittleFS
  guardarEstadisticas();
}

/**
 * @brief Triggers an audible alert to signal the end of a sequence.
 * Note: Uses blocking delay() which is acceptable here since the sequence is over.
 */
void sonarAlarmaFin() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(PIN_BUZZER, HIGH); // Beep ON
    delay(150);                     
    digitalWrite(PIN_BUZZER, LOW);  // Beep OFF
    delay(100);                     
  }
}

/**
 * @brief Non-blocking state machine that processes the camera sequence.
 * Must be called continuously inside the main loop().
 */
void updateCamera() {
  if (camState == CAM_DETENIDO) return;

  unsigned long currentMillis = millis();

  switch (camState) {
    case CAM_ESPERA_INICIAL:
      // Initial 3-second delay to avoid camera shake after pressing the encoder
      if (currentMillis - ultimoEventoCamera >= 3000) {
        camState = CAM_OBTURANDO;
        ultimoEventoCamera = currentMillis;
        
        // Activate shutter relay (LOW)
        digitalWrite(PIN_RELAY_SHUTTER, LOW);
        digitalWrite(LED_BUILTIN, LOW); 
      }
      break;

    case CAM_OBTURANDO:
      // Wait for the exposure time to complete
      if (currentMillis - ultimoEventoCamera >= (perfilActual.tiempoObturacion * 1000UL)) {
        
        // Deactivate shutter relay (HIGH)
        digitalWrite(PIN_RELAY_SHUTTER, HIGH);
        digitalWrite(LED_BUILTIN, HIGH); 
        
        fotosTomadasSesion++;
        ultimoEventoCamera = currentMillis;

        // Check if the target photo limit has been reached (0 means infinite)
        if (perfilActual.limiteFotos > 0 && fotosTomadasSesion >= perfilActual.limiteFotos) {
          detenerSecuencia(); 
          sonarAlarmaFin(); 
        } else {
          camState = CAM_INTERVALO; 
        }
      }
      break;

    case CAM_INTERVALO:
      // Wait for the interval time between shots to complete
      if (currentMillis - ultimoEventoCamera >= (perfilActual.tiempoEntreFotos * 1000UL)) {
        camState = CAM_OBTURANDO;
        ultimoEventoCamera = currentMillis;
        
        // Activate shutter relay again (LOW)
        digitalWrite(PIN_RELAY_SHUTTER, LOW);
        digitalWrite(LED_BUILTIN, LOW); 
      }
      break;
  }
}

// --- Hardware Testing Functions ---

/**
 * @brief Forces a 500ms test shutter trigger. 
 * Only works if the camera is currently stopped.
 */
void forzarFoto() {
  if (camState != CAM_DETENIDO) return; 
  digitalWrite(PIN_RELAY_SHUTTER, LOW);  // Turn ON
  digitalWrite(LED_BUILTIN, LOW);
  delay(500); 
  digitalWrite(PIN_RELAY_SHUTTER, HIGH); // Turn OFF
  digitalWrite(LED_BUILTIN, HIGH);
}

/**
 * @brief Forces a 500ms test focus trigger.
 * Only works if the camera is currently stopped.
 */
void forzarFoco() {
  if (camState != CAM_DETENIDO) return; 
  digitalWrite(PIN_RELAY_FOCUS, LOW);  // Turn ON
  delay(500); 
  digitalWrite(PIN_RELAY_FOCUS, HIGH); // Turn OFF
}
