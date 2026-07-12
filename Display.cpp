#include "Display.h"
#include "Config.h"
#include "Input.h"
#include "Camera.h"
#include "Storage.h"
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);

const int NUM_OPCIONES_MENU = 5;
bool redraw = true;
unsigned long ultimoCambioDisplay = 0;
bool mostrarProgreso = false;

int subMenuIndex = 0; 
int editSlot = 0;
char editBuffer[15];
int cursorEdit = 2; 
int charIndex = 0;
char listaNombres[6][15]; 

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
  else if (menuIndex == 3) lcd.print("4.Perfiles");
  else if (menuIndex == 4) lcd.print("5.Estadisticas");

  if (menuIndex < NUM_OPCIONES_MENU - 1) {
    lcd.setCursor(1, 1);
    int nextIndex = menuIndex + 1;
    if (nextIndex == 1) {
      if (camState == CAM_DETENIDO) lcd.print("2.Iniciar");
      else lcd.print("2.Detener");
    }
    else if (nextIndex == 2) lcd.print("3.Configuracion");
    else if (nextIndex == 3) lcd.print("4.Perfiles");
    else if (nextIndex == 4) lcd.print("5.Estadisticas");
  }
}

void cargarCacheNombres() {
  for(int i = 0; i < 6; i++) {
    obtenerNombrePerfil(i, listaNombres[i]);
  }
}

void updateDisplay() {
  unsigned long currentMillis = millis();

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
        } else {
          lcd.print("DETENIDO");
          detenerSecuencia();
        }
        delay(1000); 
        currentState = ESTADO_INICIO; 
      }
      else if (menuIndex == 2) currentState = ESTADO_CONFIG;
      else if (menuIndex == 3) {
        subMenuIndex = 0;
        currentState = ESTADO_MENU_PERFILES;
      }
      else if (menuIndex == 4) currentState = ESTADO_ESTADISTICAS;
      redraw = true;
    }
  } 
  
  else if (currentState == ESTADO_CONFIG) {
    if (encoderDelta != 0) {
      float incrementoFloat = (abs(encoderDelta) >= 3) ? 1.0 : 0.1;
      int incrementoInt = (abs(encoderDelta) >= 3) ? 10 : 1; 
      
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

  else if (currentState == ESTADO_INICIO) {
    if (encoderDelta < 0) {
      forzarFoto();
    } else if (encoderDelta > 0) {
      forzarFoco();
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
  
  else if (currentState == ESTADO_ESTADISTICAS) {
    if (buttonPressed) {
      currentState = ESTADO_MENU_PRINCIPAL;
      redraw = true;
    }
  }

  // --- NAVEGACIÓN MENÚ PERFILES (Cargar / Guardar / Volver) ---
  else if (currentState == ESTADO_MENU_PERFILES) {
    if (encoderDelta != 0) {
      subMenuIndex += (encoderDelta > 0) ? 1 : -1;
      if (subMenuIndex < 0) subMenuIndex = 0;
      if (subMenuIndex > 2) subMenuIndex = 2; // 0, 1, 2
      redraw = true;
    }
    
    if (buttonPressed) {
      if (subMenuIndex == 0) {
        cargarCacheNombres();
        subMenuIndex = 0;
        currentState = ESTADO_CARGAR_PERFIL;
      } 
      else if (subMenuIndex == 1) {
        cargarCacheNombres();
        subMenuIndex = 0;
        currentState = ESTADO_GUARDAR_PERFIL;
      }
      else if (subMenuIndex == 2) {
        currentState = ESTADO_MENU_PRINCIPAL;
        menuIndex = 3; // Volvemos a quedar parados sobre "4.Perfiles"
      }
      redraw = true;
    }
  }

  // --- NAVEGACIÓN CARGAR PERFIL ---
  else if (currentState == ESTADO_CARGAR_PERFIL) {
    if (encoderDelta != 0) {
      subMenuIndex += (encoderDelta > 0) ? 1 : -1;
      if (subMenuIndex < 0) subMenuIndex = 0;
      if (subMenuIndex > 6) subMenuIndex = 6; // 0 al 5 (slots) + 6 (Volver)
      redraw = true;
    }
    
    if (buttonPressed) {
      if (subMenuIndex == 6) { // Seleccionó "Volver"
        currentState = ESTADO_MENU_PERFILES;
        subMenuIndex = 0; 
      } else {
        cargarPerfil(subMenuIndex);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Perfil cargado!");
        delay(1000);
        currentState = ESTADO_MENU_PRINCIPAL;
      }
      redraw = true;
    }
  }

  // --- NAVEGACIÓN GUARDAR PERFIL ---
  else if (currentState == ESTADO_GUARDAR_PERFIL) {
    if (encoderDelta != 0) {
      subMenuIndex += (encoderDelta > 0) ? 1 : -1;
      if (subMenuIndex < 0) subMenuIndex = 0;
      if (subMenuIndex > 6) subMenuIndex = 6; // 0 al 5 (slots) + 6 (Volver)
      redraw = true;
    }
    
    if (buttonPressed) {
      if (subMenuIndex == 6) { // Seleccionó "Volver"
        currentState = ESTADO_MENU_PERFILES;
        subMenuIndex = 1; // Lo dejamos parado sobre "Guardar"
      } else {
        editSlot = subMenuIndex;
        cursorEdit = 2;
        charIndex = 0;
        
        snprintf(editBuffer, 15, "%d.            ", editSlot + 1);
        editBuffer[cursorEdit] = ALFABETO[charIndex]; 
        
        currentState = ESTADO_EDITAR_NOMBRE;
      }
      redraw = true;
    }
  }

  // --- NAVEGACIÓN EDITAR NOMBRE ---
  else if (currentState == ESTADO_EDITAR_NOMBRE) {
    if (encoderDelta != 0) {
      charIndex += (encoderDelta > 0) ? 1 : -1;
      int maxChar = strlen(ALFABETO) - 1;
      if (charIndex < 0) charIndex = maxChar - 1;
      if (charIndex >= maxChar) charIndex = 0;
      
      editBuffer[cursorEdit] = ALFABETO[charIndex];
      redraw = true;
    }
    
    if (buttonPressed) {
      cursorEdit++;
      
      if (cursorEdit >= 14) { 
        editBuffer[14] = '\0';
        strncpy(perfilActual.nombre, editBuffer, 15);
        guardarPerfil(editSlot);
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Guardado ok!");
        delay(1000);
        
        currentState = ESTADO_MENU_PRINCIPAL;
      } else {
        charIndex = 0;
        editBuffer[cursorEdit] = ALFABETO[charIndex];
      }
      redraw = true;
    }
  }

  // --- DIBUJADO DE LA PANTALLA ---
  if (redraw) {
    switch (currentState) {
      case ESTADO_MENU_PRINCIPAL:
        dibujarMenuPrincipal();
        break;

      case ESTADO_INICIO:
        lcd.clear();
        {
          // Calculamos el tiempo de uso real (siempre en 0 si está detenido)
          unsigned long tUso = (camState == CAM_DETENIDO) ? 0 : (millis() - tiempoInicioSesion);
          
          if (perfilActual.limiteFotos > 0) {
            if (mostrarProgreso) {
              lcd.setCursor(0, 0);
              lcd.print("Fts: "); lcd.print(fotosTomadasSesion);
              lcd.print("/"); lcd.print(perfilActual.limiteFotos);
              
              lcd.setCursor(0, 1);
              lcd.print("[");
              int numBloques = 0;
              if (perfilActual.limiteFotos > 0) {
                numBloques = (fotosTomadasSesion * 14) / perfilActual.limiteFotos;
              }
              if (numBloques > 14) numBloques = 14; 
              
              for (int i = 0; i < 14; i++) {
                if (i < numBloques) lcd.print("=");
                else if (i == numBloques && numBloques < 14 && camState != CAM_DETENIDO) lcd.print(">");
                else lcd.print(" ");
              }
              lcd.print("]");
            } else {
              lcd.setCursor(0, 0);
              lcd.print("Fts: "); lcd.print(fotosTomadasSesion);
              lcd.setCursor(0, 1);
              lcd.print("t uso:");
              imprimirTiempo(tUso, 7, 1);
            }
          } else {
            lcd.setCursor(0, 0);
            lcd.print("Fts: "); lcd.print(fotosTomadasSesion);
            lcd.setCursor(0, 1);
            lcd.print("t uso: ");
            imprimirTiempo(tUso, 7, 1);
          }
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
          if (perfilActual.limiteFotos == 0) {
            lcd.print("Apagado         "); 
          } else {
            lcd.print(perfilActual.limiteFotos);
            lcd.print(" fotos          ");
          }
        }
        break;
        
      case ESTADO_MENU_PERFILES:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(">");
        if (subMenuIndex == 0) lcd.print("1.Cargar");
        else if (subMenuIndex == 1) lcd.print("2.Guardar");
        else if (subMenuIndex == 2) lcd.print("3.Volver");

        if (subMenuIndex < 2) {
          lcd.setCursor(1, 1);
          if (subMenuIndex + 1 == 1) lcd.print("2.Guardar");
          else if (subMenuIndex + 1 == 2) lcd.print("3.Volver");
        }
        break;

      case ESTADO_CARGAR_PERFIL:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Cargar en slot:");
        lcd.setCursor(0, 1);
        lcd.print(">");
        if (subMenuIndex == 6) lcd.print("7. Volver");
        else lcd.print(listaNombres[subMenuIndex]);
        break;

      case ESTADO_GUARDAR_PERFIL:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Guardar en slot:");
        lcd.setCursor(0, 1);
        lcd.print(">");
        if (subMenuIndex == 6) lcd.print("7. Volver");
        else lcd.print(listaNombres[subMenuIndex]);
        break;

      case ESTADO_EDITAR_NOMBRE:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Renombrar:");
        lcd.setCursor(0, 1);
        lcd.print(editBuffer);
        
        lcd.setCursor(cursorEdit, 1);
        lcd.cursor(); 
        lcd.noBlink(); 
        break;
    }
    
    if (currentState != ESTADO_EDITAR_NOMBRE) {
      lcd.noBlink();
      lcd.noCursor();
    }
    
    redraw = false;
  }
}
