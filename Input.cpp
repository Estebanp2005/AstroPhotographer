/**
 * @file Input.cpp
 * @brief Implementation of the Rotary Encoder input module.
 * * This file manages the physical rotary encoder (rotation and button clicks).
 * It utilizes hardware interrupts mapped directly to internal RAM (IRAM_ATTR)
 * to guarantee responsive and lag-free input capture.
 */

#include "Input.h"
#include "Config.h"
#include <AiEsp32RotaryEncoder.h>

// --- Hardware Instantiation ---

/**
 * @brief Rotary Encoder instance.
 * Parameters: Pin A, Pin B, Pin Button, Pin VCC (-1 if unused), Steps per notch.
 * Passing -1 for VCC since the encoder relies on the board's physical power rails.
 */
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(PIN_ENC_A, PIN_ENC_B, PIN_ENC_SW, -1, 4);

// --- Global Input Variables ---

/** @brief The change in encoder position since the last read. Positive = CW, Negative = CCW. */
int encoderDelta = 0;

/** @brief Flag indicating if the encoder button was pressed. Reset on every loop. */
bool buttonPressed = false;

/** @brief Keeps track of the absolute encoder value to calculate the relative delta. */
int lastEncoderValue = 0;

// --- Interrupt Service Routine (ISR) ---

/**
 * @brief Hardware Interrupt Service Routine for reading encoder state changes.
 * * Attributes: IRAM_ATTR places this function in the fast Internal RAM of the ESP,
 * ensuring high-speed processing without latency.
 */
void IRAM_ATTR readEncoderISR() {
    rotaryEncoder.readEncoder_ISR();
}

// --- Core Functions ---

/**
 * @brief Initializes the GPIO pins and registers the interrupt routine for the encoder.
 */
void initInput() {
    // 1. Force internal pull-up resistors for reliable state detection on raw encoders
    pinMode(PIN_ENC_A, INPUT_PULLUP);
    pinMode(PIN_ENC_B, INPUT_PULLUP);
    pinMode(PIN_ENC_SW, INPUT_PULLUP);

    // 2. Initialize library and register the ISR callback
    rotaryEncoder.begin();
    rotaryEncoder.setup(readEncoderISR);
    
    // Set virtually infinite boundaries to handle free menu scrolling
    rotaryEncoder.setBoundaries(-99999, 99999, false); 
    
    // Enable dynamic acceleration (turning faster yields larger increments)
    rotaryEncoder.setAcceleration(250); 
    
    lastEncoderValue = rotaryEncoder.readEncoder();
}

/**
 * @brief Polls the encoder status, updates the delta step, and checks the button state.
 * Must be called continuously at the start of the main loop().
 */
void updateInput() {
    // Reset flags at the beginning of each cycle
    buttonPressed = false;
    encoderDelta = 0;

    // Evaluate encoder rotation
    if (rotaryEncoder.encoderChanged()) {
        int currentValue = rotaryEncoder.readEncoder();
        encoderDelta = currentValue - lastEncoderValue; 
        lastEncoderValue = currentValue;
    }

    // Evaluate encoder click
    if (rotaryEncoder.isEncoderButtonClicked()) {
        buttonPressed = true;
    }
}
