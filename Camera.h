/**
 * @file Camera.h
 * @brief Header file for the camera control module.
 * * Contains the function prototypes and external variables 
 * required to manage the camera relay state machine.
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <Arduino.h>

// --- External Global Variables ---

/** @brief Counter for photos taken during the active session. */
extern int fotosTomadasSesion;

/** @brief Total duration (in ms) of the last completed session. */
extern unsigned long duracionUltimaSesion;

// --- Function Prototypes ---

/** @brief Initializes the hardware pins for relays and buzzer. */
void initCamera();

/** @brief Starts a new intervalometer sequence. */
void iniciarSecuencia();

/** @brief Safely stops the current sequence and saves statistics. */
void detenerSecuencia();

/** @brief Non-blocking state machine loop (must be called in main loop). */
void updateCamera();

/** @brief Triggers a 500ms test shutter actuation. */
void forzarFoto();

/** @brief Triggers a 500ms test focus actuation. */
void forzarFoco();

#endif // CAMERA_H
