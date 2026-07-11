#include "Config.h"
#include "Input.h"
#include "Camera.h"
#include "Display.h"
#include "Storage.h" // <--- Agregamos esto

// Instanciación de variables
SystemState currentState = ESTADO_INICIO;
CameraState camState = CAM_DETENIDO;
Perfil perfilActual; // Ahora se cargará desde la EEPROM
Estadisticas statsGlobales;
int menuIndex = 0;
int configIndex = 0;

void setup() {
  Serial.begin(115200);
  
  initStorage(); // <--- 1. Inicializa EEPROM y carga estadísticas históricas
  cargarPerfil(0); // <--- 2. Carga el perfil del Slot 1 por defecto al arrancar
  
  initInput();
  initCamera();
  initDisplay();
}

void loop() {
  updateInput();
  updateCamera();
  updateDisplay();
}