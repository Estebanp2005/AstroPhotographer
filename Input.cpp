#include "Input.h"
#include "Config.h"
#include <AiEsp32RotaryEncoder.h>

// Parámetros: Pin A, Pin B, Pin Botón, Pin VCC (negativo si no usa), Pasos por muesca
// Como tu encoder es "pelado" y no tiene VCC propio conectado a un pin digital, le pasamos -1
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(PIN_ENC_A, PIN_ENC_B, PIN_ENC_SW, -1, 4);

int encoderDelta = 0;
bool buttonPressed = false;
int lastEncoderValue = 0;

// Función de interrupción
void IRAM_ATTR readEncoderISR() {
    rotaryEncoder.readEncoder_ISR();
}

void initInput() {
    // 1. FORZAMOS LOS PULL-UPS INTERNOS PARA EL ENCODER PELADO
    pinMode(PIN_ENC_A, INPUT_PULLUP);
    pinMode(PIN_ENC_B, INPUT_PULLUP);
    pinMode(PIN_ENC_SW, INPUT_PULLUP);

    // 2. Inicializamos la librería
    rotaryEncoder.begin();
    rotaryEncoder.setup(readEncoderISR);
    
    // Configuramos límites infinitos para poder girar la ruleta de menús y tiempos
    rotaryEncoder.setBoundaries(-99999, 99999, false); 
    
    // Aceleración: Si girás rápido los números cambian más deprisa
    rotaryEncoder.setAcceleration(250); 
    
    lastEncoderValue = rotaryEncoder.readEncoder();
}

void updateInput() {
    buttonPressed = false;
    encoderDelta = 0;

    // Leemos si el encoder giró
    if (rotaryEncoder.encoderChanged()) {
        int currentValue = rotaryEncoder.readEncoder();
        encoderDelta = currentValue - lastEncoderValue; // Retorna positivo o negativo
        lastEncoderValue = currentValue;
    }

    // Leemos si se apretó el botón del encoder
    if (rotaryEncoder.isEncoderButtonClicked()) {
        buttonPressed = true;
    }
}