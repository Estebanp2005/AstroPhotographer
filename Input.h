#ifndef INPUT_H
#define INPUT_H

void initInput();
void updateInput();
extern int encoderDelta;   // Cuánto giró (-1, 0, 1)
extern bool buttonPressed; // Si se apretó el botón

#endif