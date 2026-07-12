#ifndef CAMERA_H
#define CAMERA_H

#include <Arduino.h>

extern int fotosTomadasSesion;

void initCamera();
void iniciarSecuencia();
void detenerSecuencia();
void updateCamera();
void forzarFoto();
void forzarFoco();

#endif
