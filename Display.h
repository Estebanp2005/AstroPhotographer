/**
 * @file Display.h
 * @brief Header file for the LCD User Interface module.
 * * Contains the function prototypes required to initialize and 
 * update the 16x2 I2C LCD screen and menu system.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

// --- Function Prototypes ---

/** * @brief Initializes the I2C LCD screen and displays the startup splash screen. 
 */
void initDisplay();

/** * @brief Main UI state machine loop that processes inputs and renders the screen.
 * Must be called continuously in the main loop().
 */
void updateDisplay();

#endif // DISPLAY_H
