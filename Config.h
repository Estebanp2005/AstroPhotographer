#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- PINES ---
#define PIN_SCL D1
#define PIN_SDA D2
#define PIN_ENC_A D5
#define PIN_ENC_B D6
#define PIN_ENC_SW D7
#define PIN_RELAY_SHUTTER D3
#define PIN_RELAY_FOCUS D4
#define PIN_BUZZER D8

// --- ESTRUCTURAS ---
struct Perfil {
  char nombre[15];
  float tiempoEntreFotos;
  float tiempoObturacion;
  int limiteFotos;
};

struct Estadisticas {
  unsigned long totalFotosHistorico;
  unsigned long tiempoTotalFunc;
};

// --- ESTADOS ---
enum SystemState { 
  ESTADO_INICIO, 
  ESTADO_MENU_PRINCIPAL, 
  ESTADO_CONFIG, 
  ESTADO_MENU_PERFILES, 
  ESTADO_CARGAR_PERFIL,
  ESTADO_GUARDAR_PERFIL,
  ESTADO_EDITAR_NOMBRE,
  ESTADO_ESTADISTICAS 
};

enum CameraState { 
  CAM_DETENIDO,
  CAM_ESPERA_INICIAL, 
  CAM_ENFOCANDO, 
  CAM_OBTURANDO, 
  CAM_INTERVALO 
};

// --- VARIABLES GLOBALES (externas) ---
extern SystemState currentState;
extern CameraState camState;
extern Perfil perfilActual;
extern Estadisticas statsGlobales;
extern int menuIndex;
extern unsigned long tiempoInicioSesion;
extern int configIndex;

const char ALFABETO[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-/.";

#endif
