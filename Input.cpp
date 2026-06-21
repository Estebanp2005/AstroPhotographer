#include "Input.h"
#include "Config.h"
#include <AiEsp32RotaryEncoder.h>

// Parámetros: Pin A, Pin B, Pin Botón, Pin VCC (negativo si no usa), Pasos por muesca
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(PIN_ENC_A, PIN_ENC_B, PIN_ENC_SW, -1, 4);

int encoderDelta = 0;
bool buttonPressed = false;
int lastEncoderValue = 0;

void IRAM_ATTR readEncoderISR() {
    rotaryEncoder.readEncoder_ISR();
}

void initInput() {
    rotaryEncoder.begin();
    rotaryEncoder.setup(readEncoderISR);
    rotaryEncoder.setBoundaries(-99999, 99999, false); // Límites infinitos
    rotaryEncoder.setAcceleration(250); // Ajustar para giro rápido
    lastEncoderValue = rotaryEncoder.readEncoder();
}

void updateInput() {
    buttonPressed = false;
    encoderDelta = 0;

    if (rotaryEncoder.encoderChanged()) {
        int currentValue = rotaryEncoder.readEncoder();
        encoderDelta = currentValue - lastEncoderValue;
        lastEncoderValue = currentValue;
    }

    if (rotaryEncoder.isEncoderButtonClicked()) {
        buttonPressed = true;
    }
}