/**
 * @file Config.h
 * @brief Global configuration, hardware pinout, data structures, and state machines.
 * * This header acts as the central source of truth for the entire project, 
 * defining hardware connections, UI states, camera sequence states, and data models.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- HARDWARE PINOUT ---

// I2C Display Pins
#define PIN_SCL D1
#define PIN_SDA D2

// Rotary Encoder Pins
#define PIN_ENC_A D5     // DT (Data)
#define PIN_ENC_B D6     // CLK (Clock)
#define PIN_ENC_SW D7    // Switch/Button

// Relay Module & Buzzer Pins
#define PIN_RELAY_SHUTTER D3
#define PIN_RELAY_FOCUS D0
#define PIN_BUZZER D8

// --- DATA STRUCTURES ---

/**
 * @brief Represents a single astrophotography shooting profile.
 */
struct Perfil {
  char nombre[15];          /**< Profile name (max 14 chars + null terminator) */
  float tiempoEntreFotos;   /**< Interval between shots in seconds */
  float tiempoObturacion;   /**< Exposure time in seconds */
  int limiteFotos;          /**< Number of photos to take (0 = infinite) */
};

/**
 * @brief Global device statistics persisted in non-volatile memory (EEPROM/LittleFS).
 */
struct Estadisticas {
  unsigned long totalFotosHistorico; /**< Total photos taken across all sessions */
  unsigned long tiempoTotalFunc;     /**< Total device runtime in milliseconds */
};

// --- STATE MACHINES ---

/**
 * @brief Enumerates the possible states of the user interface (UI).
 */
enum SystemState { 
  ESTADO_INICIO,         /**< Idle / Home screen */
  ESTADO_MENU_PRINCIPAL, /**< Main menu navigation */
  ESTADO_CONFIG,         /**< Adjusting current profile parameters */
  ESTADO_MENU_PERFILES,  /**< Selecting profile management options */
  ESTADO_CARGAR_PERFIL,  /**< Loading a profile from storage */
  ESTADO_GUARDAR_PERFIL, /**< Selecting a slot to save the current profile */
  ESTADO_EDITAR_NOMBRE,  /**< Character-by-character name editing */
  ESTADO_ESTADISTICAS    /**< Viewing historical stats */
};

/**
 * @brief Enumerates the possible states of the camera trigger sequence.
 */
enum CameraState { 
  CAM_DETENIDO,       /**< Sequence stopped / inactive */
  CAM_ESPERA_INICIAL, /**< Initial 3-second delay before starting the sequence */
  CAM_ENFOCANDO,      /**< Active focus trigger (if used in sequence) */
  CAM_OBTURANDO,      /**< Active shutter trigger (exposure) */
  CAM_INTERVALO       /**< Waiting interval between exposures */
};

// --- GLOBAL VARIABLES (External Declarations) ---

extern SystemState currentState;
extern CameraState camState;
extern Perfil perfilActual;
extern Estadisticas statsGlobales;
extern int menuIndex;
extern unsigned long tiempoInicioSesion;
extern int configIndex;

// --- CONSTANTS ---

/** * @brief Available characters for naming profiles in the UI. 
 * Note the first character is a blank space.
 */
const char ALFABETO[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-/.";

#endif // CONFIG_H
