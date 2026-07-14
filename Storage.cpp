/**
 * @file Storage.cpp
 * @brief Non-volatile storage management using the ESP32 Preferences library.
 * * This file manages persisting user shooting profiles and historical device 
 * usage statistics across power cycles. It abstracts reads and writes directly 
 * into the ESP32 NVS (Non-Volatile Storage) flash partition.
 */

#include "Storage.h"
#include "Config.h"
#include <Preferences.h>

// --- Hardware Instantiation ---

/** @brief ESP32 Preferences instance used to access NVS storage. */
Preferences prefs;

// --- Initialization ---

/**
 * @brief Opens the NVS namespace "astro" and triggers the stats restore.
 */
void initStorage() {
  // Open the "astro" namespace in Read/Write mode (false)
  prefs.begin("astro", false); 
  cargarEstadisticas();
}

// --- Device Statistics Persistence ---

/**
 * @brief Restores historical stats from non-volatile storage. 
 * If keys do not exist, default fallback values (0) are assigned.
 */
void cargarEstadisticas() {
  statsGlobales.totalFotosHistorico = prefs.getULong("totFotos", 0);
  statsGlobales.tiempoTotalFunc = prefs.getULong("tFunc", 0);
}

/**
 * @brief Saves current session runtime and photos count into global totals.
 * Note: Preferences writes immediately to Flash without requiring a manual commit.
 */
void guardarEstadisticas() {
  prefs.putULong("totFotos", statsGlobales.totalFotosHistorico);
  prefs.putULong("tFunc", statsGlobales.tiempoTotalFunc);
}

// --- Shooting Profile Persistence ---

/**
 * @brief Loads a specific user profile (Slots 0 to 5) from storage into RAM.
 * * If the requested slot is empty in NVS, it populates it with standard 
 * default settings (5s interval, 2s exposure, unlimited photos).
 * * @param slot The profile slot index to load (0-based, mapping to Slots 1-6).
 */
void cargarPerfil(int slot) {
  if (slot < 0 || slot >= 6) return; // Safety boundary check
  
  char key[10];
  snprintf(key, sizeof(key), "prf%d", slot); // Generates NVS key: "prf0", "prf1"...

  // If the key doesn't exist, build a fresh placeholder profile
  if (!prefs.isKey(key)) {
    snprintf(perfilActual.nombre, sizeof(perfilActual.nombre), "%d. Vacio", slot + 1);
    perfilActual.tiempoEntreFotos = 5.0;
    perfilActual.tiempoObturacion = 2.0;
    perfilActual.limiteFotos = 0;
  } else {
    // Read raw bytes directly from NVS and unpack them into the active struct
    prefs.getBytes(key, &perfilActual, sizeof(Perfil));
  }
}

/**
 * @brief Serializes and writes the active RAM profile to a specific slot in NVS.
 * * @param slot The profile slot index to write to (0 to 5).
 */
void guardarPerfil(int slot) {
  if (slot < 0 || slot >= 6) return;
  
  char key[10];
  snprintf(key, sizeof(key), "prf%d", slot);
  
  // Force the first two characters of the display name to match the physical slot number
  perfilActual.nombre[0] = '1' + slot;
  perfilActual.nombre[1] = '.';
  
  // Write the entire struct into memory as raw bytes in a single call
  prefs.putBytes(key, &perfilActual, sizeof(Perfil));
}

/**
 * @brief Extracts only the name of a stored profile without loading its settings into RAM.
 * Useful for building menu lists quickly without overwriting active session parameters.
 * * @param slot The profile slot index to read (0 to 5).
 * @param buffer Output char array to store the profile's name (must be >= 15 bytes).
 */
void obtenerNombrePerfil(int slot, char* buffer) {
  if (slot < 0 || slot >= 6) return;
  
  char key[10];
  snprintf(key, sizeof(key), "prf%d", slot);
  
  if (!prefs.isKey(key)) {
    // Default placeholder if the slot is empty
    snprintf(buffer, 15, "%d. Vacio", slot + 1);
  } else {
    // Read NVS bytes into a temporary struct, extract name, and safely null-terminate
    Perfil temp;
    prefs.getBytes(key, &temp, sizeof(Perfil));
    strncpy(buffer, temp.nombre, 15);
    buffer[14] = '\0'; 
  }
}
