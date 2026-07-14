/**
 * @file Input.h
 * @brief Header file for the Rotary Encoder input module.
 * * Defines the external variables and function prototypes required
 * to handle physical user interactions via the rotary encoder.
 */

#ifndef INPUT_H
#define INPUT_H

// --- External Global Variables ---

/** * @brief Relative steps rotated since the last cycle. 
 * Positive values indicate clockwise rotation, negative values counter-clockwise.
 */
extern int encoderDelta;

/** * @brief State flag indicating if the encoder button was clicked during the current loop.
 */
extern bool buttonPressed;

// --- Function Prototypes ---

/** * @brief Configures GPIO pins with pull-ups and initializes the encoder hardware interrupts.
 */
void initInput();

/** * @brief Polls the encoder, calculates the rotation delta, and checks for button clicks.
 * Must be called continuously in the main loop().
 */
void updateInput();

#endif // INPUT_H
