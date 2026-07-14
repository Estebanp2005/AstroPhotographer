/**
 * @file Astrophotographer.ino
 * @brief Main entry point for the AstroTracker firmware.
 * * This file handles the core initialization and the main execution loop
 * of the astrophotography intervalometer. It delegates specific tasks 
 * (input processing, camera relay control, display UI, and memory storage) 
 * to their respective modular components.
 */

#include "Config.h"
#include "Input.h"
#include "Camera.h"
#include "Display.h"
#include "Storage.h"

// --- Global Variables Instantiation ---

/**
 * @brief Tracks the current operational state of the system 
 * (e.g., IDLE, MENU, CONFIG, RUNNING_SEQUENCE).
 */
SystemState currentState = ESTADO_INICIO;

/**
 * @brief Holds the currently active shooting profile loaded in RAM
 * (includes interval, exposure time, photo limit, and profile name).
 */
Perfil perfilActual;

/**
 * @brief Stores the global usage statistics tracked across reboots 
 * (total device runtime and total historical photos taken).
 */
Estadisticas statsGlobales;

/**
 * @brief Navigation index for the primary menu selection.
 */
int menuIndex = 0;

/**
 * @brief Navigation index for the configuration sub-menus.
 */
int configIndex = 0;

// --- Core Functions ---

/**
 * @brief System initialization routine.
 * * Sets up serial communication, initializes the non-volatile storage, 
 * loads the default profile, and prepares all hardware peripherals.
 */
void setup() {
  // Initialize serial communication for debugging purposes
  Serial.begin(115200);
  
  // Initialize LittleFS/Preferences and load historical stats from EEPROM
  initStorage(); 
  
  // Load the default shooting profile (Slot 1 / Index 0) into RAM on startup
  cargarPerfil(0); 
  
  // Initialize hardware peripherals and internal software modules
  initInput();
  initCamera();
  initDisplay();
}

/**
 * @brief Main execution loop.
 * * Continuously polls the encoder input, updates the camera relay logic 
 * based on the active sequence, and refreshes the LCD display UI. 
 * This loop relies on non-blocking state machines.
 */
void loop() {
  updateInput();
  updateCamera();
  updateDisplay();
}
