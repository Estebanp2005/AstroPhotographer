#include "Display.h"
#include "Config.h"
#include "Input.h"
#include "Camera.h"
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2); // Tu ajuste para que la pantalla encienda

const int NUM_OPCIONES_MENU = 5;
bool redraw = true;
unsigned long ultimoCambioDisplay = 0;
bool mostrarProgreso = false; // Alternar info en pantalla de inicio

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

// Formatea milisegundos a h:mm:ss y lo imprime
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
  
  // Opción superior (Actual) respetando tu formato de texto
  if (menuIndex == 0) lcd.print("1.Inicio");
  else if (menuIndex == 1) {
    if (camState == CAM_DETENIDO) lcd.print("2.Iniciar");
    else lcd.print("2.Detener");
  }
  else if (menuIndex == 2) lcd.print("3.Configuracion");
  else if (menuIndex == 3) lcd.print("4.Cargar perfil");
  else if (menuIndex == 4) lcd.print("5.Estadisticas");

  // Opción inferior (Siguiente)
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
        // Ejecutar Iniciar/Detener
        lcd.clear();
        lcd.setCursor(4, 0);
        if (camState == CAM_DETENIDO) {
          lcd.print("INICIADO");
          iniciarSecuencia(); // Llama a Camera.cpp
          tiempoInicioSesion = millis(); // Reinicia contador
        } else {
          lcd.print("DETENIDO");
          detenerSecuencia(); // Llama a Camera.cpp
        }
        delay(1000); // Muestra mensaje por 1 segundo
        currentState = ESTADO_INICIO; // Vuelve al inicio
      }
        else if (currentState == ESTADO_CONFIG) {
    if (encoderDelta != 0) {
      // Si giramos rápido (delta >= 2 o <= -2), saltamos de a 1.0. Si no, de a 0.1
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
      configIndex++; // Pasamos a la siguiente opción
      if (configIndex > 2) {
        configIndex = 0; // Reiniciamos el índice
        currentState = ESTADO_MENU_PRINCIPAL; // Volvemos al menú
      }
      redraw = true;
    }
  }

      else if (menuIndex == 3) currentState = ESTADO_MENU_PERFILES;
      else if (menuIndex == 4) currentState = ESTADO_ESTADISTICAS;
      redraw = true;
    }
  } 
  
  else if (currentState == ESTADO_INICIO) {
    if (encoderDelta < 0) {
      // Girar izquierda: Tomar foto sin delay (Implementar en Camera)
      // forzarFoto();
    } else if (encoderDelta > 0) {
      // Girar derecha: Enfocar 1 seg (Implementar en Camera)
      // forzarFoco();
    }
    
    if (buttonPressed) {
      currentState = ESTADO_MENU_PRINCIPAL;
      menuIndex = 0;
      redraw = true;
    }

    // Lógica para alternar pantalla cada 2 segundos si hay límite
    if (perfilActual.limiteFotos > 0 && (currentMillis - ultimoCambioDisplay >= 2000)) {
      mostrarProgreso = !mostrarProgreso;
      ultimoCambioDisplay = currentMillis;
      redraw = true;
    }
    
    // Forzar actualización del reloj cada segundo
    static unsigned long ultimoSeg = 0;
    if (currentMillis - ultimoSeg >= 1000) {
      ultimoSeg = currentMillis;
      if (camState != CAM_DETENIDO) redraw = true;
    }
  }
  
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
            // Acá podemos dibujar una barrita en la fila 1 después
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
          // Modo infinito (sin límite)
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
          lcd.print(perfilActual.tiempoEntreFotos, 1); // 1 decimal
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