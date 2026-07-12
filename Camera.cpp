#include "Camera.h"
#include "Config.h"
#include "Storage.h"
#include "Display.h"

// Variables globales declaradas en Config.h
CameraState camState = CAM_DETENIDO;
int fotosTomadasSesion = 0;
unsigned long tiempoInicioSesion = 0;

// Variables locales para el control de tiempos
unsigned long ultimoEventoCamera = 0;

void initCamera() {
  pinMode(PIN_RELAY_SHUTTER, OUTPUT);
  pinMode(PIN_RELAY_FOCUS, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Apagamos los relays al arrancar (Nivel LOW)
  digitalWrite(PIN_RELAY_SHUTTER, LOW);
  digitalWrite(PIN_RELAY_FOCUS, LOW);
  
  // Apagamos el LED (En los ESP, el LED integrado suele funcionar con lógica inversa, HIGH es apagado)
  digitalWrite(LED_BUILTIN, HIGH); 
}

void iniciarSecuencia() {
  fotosTomadasSesion = 0;
  tiempoInicioSesion = millis();
  ultimoEventoCamera = millis();
  
  // Arrancamos en el estado de espera para evitar vibraciones
  camState = CAM_ESPERA_INICIAL; 
}

void detenerSecuencia() {
  // 1. Apagamos todo por seguridad física
  digitalWrite(PIN_RELAY_SHUTTER, LOW);
  digitalWrite(PIN_RELAY_FOCUS, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
  
  // 2. Pasamos a estado detenido
  camState = CAM_DETENIDO;

  // 3. Calculamos y actualizamos las estadísticas globales
  unsigned long tiempoUso = millis() - tiempoInicioSesion;
  statsGlobales.tiempoTotalFunc += tiempoUso;
  statsGlobales.totalFotosHistorico += fotosTomadasSesion;
  
  // 4. Escribimos en la memoria permanente
  guardarEstadisticas();
}

void updateCamera() {
  if (camState == CAM_DETENIDO) return;

  unsigned long currentMillis = millis();

  switch (camState) {
    case CAM_ESPERA_INICIAL:
      // Esperamos 3 segundos fijos al iniciar (1 seg se lo come el cartel del display, 
      // quedan 2 seg netos sin tocar nada para que pasen las vibraciones).
      if (currentMillis - ultimoEventoCamera >= 3000) {
        camState = CAM_OBTURANDO;
        ultimoEventoCamera = currentMillis;
        
        digitalWrite(PIN_RELAY_SHUTTER, HIGH);
        digitalWrite(LED_BUILTIN, LOW); // Prende LED
      }
      break;

    case CAM_OBTURANDO:
      // Dejamos el gatillo presionado el tiempo configurado en el perfil
      if (currentMillis - ultimoEventoCamera >= (perfilActual.tiempoObturacion * 1000UL)) {
        
        // Soltamos el gatillo
        digitalWrite(PIN_RELAY_SHUTTER, LOW);
        digitalWrite(LED_BUILTIN, HIGH); // Apaga LED
        
        fotosTomadasSesion++;
        ultimoEventoCamera = currentMillis;

        // Verificamos si llegamos al límite
        if (perfilActual.limiteFotos > 0 && fotosTomadasSesion >= perfilActual.limiteFotos) {
          detenerSecuencia(); // ¡Terminó la sesión!
        } else {
          camState = CAM_INTERVALO; // Pasamos a la pausa
        }
      }
      break;

    case CAM_INTERVALO:
      // Esperamos el tiempo entre fotos configurado
      if (currentMillis - ultimoEventoCamera >= (perfilActual.tiempoEntreFotos * 1000UL)) {
        camState = CAM_OBTURANDO;
        ultimoEventoCamera = currentMillis;
        
        // Apretamos el gatillo para la siguiente foto
        digitalWrite(PIN_RELAY_SHUTTER, HIGH);
        digitalWrite(LED_BUILTIN, LOW); // Prende LED
      }
      break;
      
    // CAM_ENFOCANDO lo eliminamos de la rotación automática por pedido tuyo ;)
  }
}

void forzarFoto() {
  if (camState != CAM_DETENIDO) return; // Bloqueo de seguridad
  
  digitalWrite(PIN_RELAY_SHUTTER, HIGH);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500); // Bloquea medio segundo, pero solo es manual así que no molesta
  digitalWrite(PIN_RELAY_SHUTTER, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
}

void forzarFoco() {
  if (camState != CAM_DETENIDO) return; // Bloqueo de seguridad
  
  digitalWrite(PIN_RELAY_FOCUS, HIGH);
  delay(500); 
  digitalWrite(PIN_RELAY_FOCUS, LOW);
}
