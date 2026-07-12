#include "Display.h"
#include "Config.h"
#include "Input.h"
#include "Camera.h"
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);

const int NUM_OPCIONES_MENU = 5;
bool redraw = true;
unsigned long ultimoCambioDisplay = 0;
bool mostrarProgreso = false;

void initDisplay() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(" AstroTracker ");
  lcd.setCursor(0, 1);
  lcd.print(" Iniciando... ");
  delay(1500); 
  lcd.clear();
}

void imprimirTiempo(unsigned long milisegundos, int col, int row) {
  unsigned long segundosTotales = milisegundos / 1000;
  int h = segundosTotales / 3600;
  int m = (segundosTotales % 3600) / 60;
  int s = segundosTotales % 60;
  
  char buffer[10];
  sprintf(buffer, "%d:%02d:%02d", h, m, s);
  lcd.setCursor(col, row);
  lcd.print(buffer);
}

void dibujarMenuPrincipal() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(">");
  
  if (menuIndex == 0) lcd.print("1.Inicio");
  else if (menuIndex == 1) {
    if (camState == CAM_DETENIDO) lcd.print("2.Iniciar");
    else lcd.print("2.Detener");
  }
  else if (menuIndex == 2) lcd.print("3.Configuracion");
  else if (menuIndex == 3) lcd.print("4.Cargar perfil");
  else if (menuIndex == 4) lcd.print("5.Estadisticas");

  if (menuIndex < NUM_OPCIONES_MENU - 1) {
    lcd.setCursor(1, 1);
    int nextIndex = menuIndex + 1;
    if (nextIndex == 1) {
      if (camState == CAM_DETENIDO) lcd.print("2.Iniciar");
      else lcd.print("2.Detener");
    }
    else if (nextIndex == 2) lcd.print("3.Configuracion");
    else if (nextIndex == 3) lcd.print("4.Cargar perfil");
    else if (nextIndex == 4) lcd.print("5.Estadisticas");
  }
}

void updateDisplay() {
  unsigned long currentMillis = millis();

  // --- 1. LÓGICA DE NAVEGACIÓN Y ENTRADAS ---
  
  // A. Estando en el Menú Principal
  if (currentState == ESTADO_MENU_PRINCIPAL) {
    if (encoderDelta != 0) {
      menuIndex += (encoderDelta > 0) ? 1 : -1;
      if (menuIndex < 0) menuIndex = 0;
      if (menuIndex >= NUM_OPCIONES_MENU) menuIndex = NUM_OPCIONES_MENU - 1;
      redraw = true;
    }
    
    if (buttonPressed) {
      if (menuIndex == 0) currentState = ESTADO_INICIO;
      else if (menuIndex == 1) {
        lcd.clear();
        lcd.setCursor(4, 0);
        if (camState == CAM_DETENIDO) {
          lcd.print("INICIADO");
          iniciarSecuencia();
          tiempoInicioSesion = millis();
        } else {
          lcd.print("DETENIDO");
          detenerSecuencia();
        }
        delay(1000); 
        currentState = ESTADO_INICIO; 
      }
      else if (menuIndex == 2) currentState = ESTADO_CONFIG; // ¡Esta línea faltaba!
      else if (menuIndex == 3) currentState = ESTADO_MENU_PERFILES;
      else if (menuIndex == 4) currentState = ESTADO_ESTADISTICAS;
      redraw = true;
    }
  } 
  
  // B. Estando en Configuración (Este bloque estaba mal pegado)
  else if (currentState == ESTADO_CONFIG) {
    if (encoderDelta != 0) {
      float incrementoFloat = (abs(encoderDelta) >= 2) ? 1.0 : 0.1;
      int incrementoInt = (abs(encoderDelta) >= 2) ? 10 : 1; 
      
      if (encoderDelta < 0) {
        incrementoFloat = -incrementoFloat;
        incrementoInt = -incrementoInt;
      }

      if (configIndex == 0) {
        perfilActual.tiempoEntreFotos += incrementoFloat;
        if (perfilActual.tiempoEntreFotos < 0) perfilActual.tiempoEntreFotos = 0;
      } 
      else if (configIndex == 1) {
        perfilActual.tiempoObturacion += incrementoFloat;
        if (perfilActual.tiempoObturacion < 0) perfilActual.tiempoObturacion = 0;
      } 
      else if (configIndex == 2) {
        perfilActual.limiteFotos += incrementoInt;
        if (perfilActual.limiteFotos < 0) perfilActual.limiteFotos = 0;
      }
      redraw = true;
    }

    if (buttonPressed) {
      configIndex++; 
      if (configIndex > 2) {
        configIndex = 0; 
        currentState = ESTADO_MENU_PRINCIPAL; 
      }
      redraw = true;
    }
  }

  // C. Estando en Inicio (Dashboard)
  else if (currentState == ESTADO_INICIO) {
    if (encoderDelta < 0) {
      // forzarFoto();
    } else if (encoderDelta > 0) {
      // forzarFoco();
    }
    
    if (buttonPressed) {
      currentState = ESTADO_MENU_PRINCIPAL;
      menuIndex = 0;
      redraw = true;
    }

    if (perfilActual.limiteFotos > 0 && (currentMillis - ultimoCambioDisplay >= 2000)) {
      mostrarProgreso = !mostrarProgreso;
      ultimoCambioDisplay = currentMillis;
      redraw = true;
    }
    
    static unsigned long ultimoSeg = 0;
    if (currentMillis - ultimoSeg >= 1000) {
      ultimoSeg = currentMillis;
      if (camState != CAM_DETENIDO) redraw = true;
    }
  }
  
  // D. Estando en Estadísticas
  else if (currentState == ESTADO_ESTADISTICAS) {
    if (buttonPressed) {
      currentState = ESTADO_MENU_PRINCIPAL;
      redraw = true;
    }
  }

  // --- 2. DIBUJO DE PANTALLA ---
  
  if (redraw) {
    switch (currentState) {
      case ESTADO_MENU_PRINCIPAL:
        dibujarMenuPrincipal();
        break;

      case ESTADO_INICIO:
        lcd.clear();
        if (perfilActual.limiteFotos > 0) {
          if (mostrarProgreso) {
            lcd.setCursor(0, 0);
            lcd.print("Fts: "); lcd.print(fotosTomadasSesion);
            lcd.print("/"); lcd.print(perfilActual.limiteFotos);
            lcd.setCursor(0, 1);
            lcd.print("[=====>      ]"); 
          } else {
            lcd.setCursor(0, 0);
            lcd.print("Fts: "); lcd.print(fotosTomadasSesion);
            lcd.setCursor(0, 1);
            lcd.print("t uso:");
            imprimirTiempo(millis() - tiempoInicioSesion, 7, 1);
          }
        } else {
          lcd.setCursor(0, 0);
          lcd.print("Fts: "); lcd.print(fotosTomadasSesion);
          lcd.setCursor(0, 1);
          lcd.print("t uso: ");
          imprimirTiempo((camState == CAM_DETENIDO) ? 0 : (millis() - tiempoInicioSesion), 7, 1);
        }
        break;

      case ESTADO_ESTADISTICAS:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("tot fotos:");
        lcd.print(statsGlobales.totalFotosHistorico);
        lcd.setCursor(0, 1);
        lcd.print("func:");
        imprimirTiempo(statsGlobales.tiempoTotalFunc, 6, 1);
        break;
        
      case ESTADO_CONFIG:
        lcd.clear();
        lcd.setCursor(0, 0);
        
        if (configIndex == 0) {
          lcd.print("1.T. entre fotos");
          lcd.setCursor(0, 1);
          lcd.print(perfilActual.tiempoEntreFotos, 1); 
          lcd.print(" seg");
        } 
        else if (configIndex == 1) {
          lcd.print("2.T. obturacion");
          lcd.setCursor(0, 1);
          lcd.print(perfilActual.tiempoObturacion, 1);
          lcd.print(" seg");
        } 
        else if (configIndex == 2) {
          lcd.print("3.Corte limite");
          lcd.setCursor(0, 1);
          lcd.print(perfilActual.limiteFotos);
          lcd.print(" fotos");
        }
        break;
        
      case ESTADO_MENU_PERFILES:
        lcd.clear();
        lcd.print("- PERFILES -");
        break;
    }
    redraw = false;
  }
}
