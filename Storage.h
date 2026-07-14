/**
 * @file Storage.h
 * @brief Header file for the non-volatile storage module.
 * * Contains function prototypes for managing NVS (Non-Volatile Storage) 
 * on the ESP8266, handling both session statistics and user profiles.
 */

#ifndef STORAGE_H
#define STORAGE_H

// --- Function Prototypes ---

/**
 * @brief Initializes the NVS namespace and restores historical device stats on boot.
 */
void initStorage();

/**
 * @brief Restores cumulative statistics (total photos, total runtime) from flash memory.
 */
void cargarEstadisticas();

/**
 * @brief Persists cumulative statistics (total photos, total runtime) to flash memory.
 */
void guardarEstadisticas();

/**
 * @brief Loads a specific shooting profile (slot 0 to 5) from flash memory into RAM.
 * * @param slot The target profile slot index (0-based).
 */
void cargarPerfil(int slot);

/**
 * @brief Serializes and writes the active RAM profile to a designated flash slot.
 * * @param slot The target profile slot index (0-based).
 */
void guardarPerfil(int slot);

/**
 * @brief Safely fetches the name of a stored profile without loading its settings.
 * * @param slot The target profile slot index (0-based).
 * @param buffer Output character array to write the name into (must be >= 15 bytes).
 */
void obtenerNombrePerfil(int slot, char* buffer);

#endif // STORAGE_H
