/**
 * @file Display.cpp
 * @brief Implementation of the LCD User Interface and Menu System.
 * * This file manages the 16x2 I2C LCD screen. It handles UI rendering,
 * interprets encoder inputs to navigate menus, and executes state transitions.
 * It uses a 'redraw' flag to update the screen only when necessary, preventing flicker.
 */

#include "Display.h"
#include "Config.h"
#include "Input.h"
#include "Camera.h"
#include "Storage.h"
#include <LiquidCrystal_I2C.h>

// --- Hardware Instantiation ---

/** @brief I2C LCD object (Address 0x3F, 16 columns, 2 rows). */
LiquidCrystal_I2C lcd(0x3F, 16, 2);

// --- Global UI Variables ---

const int NUM_OPCIONES_MENU = 5;

/** @brief Flag to trigger an LCD update only when the UI state changes. */
bool redraw = true;

/** @brief Timestamp for toggling the progress bar display. */
unsigned long ultimoCambioDisplay = 0;

/** @brief Toggles between showing the progress bar and the session timer. */
bool mostrarProgreso = false;

// Sub-menu navigation variables
int subMenuIndex = 0; 
int editSlot = 0;

// Profile naming variables
char editBuffer[15];
int cursorEdit = 2; 
int charIndex = 0;

/** @brief Cache array to store profile names for smooth menu scrolling. */
char listaNombres[6][15]; 

// --- Helper Functions ---

/**
 * @brief Initializes the LCD and shows the boot splash screen.
 */
void initDisplay() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(" AstroTracker ");
  lcd.setCursor(0, 1);
  lcd.print("By: Estebanp2005");
  delay(1500); 
  lcd.clear();
}

/**
 * @brief Formats and prints a millisecond timestamp as HH:MM:SS on the LCD.
 * * @param milisegundos Time in milliseconds.
 * @param col Starting LCD column.
 * @param row LCD row (0 or 1).
 */
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

/**
 * @brief Renders the main menu, dynamically showing the selected item and the next one.
 */
void dibujarMenuPrincipal() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(">");
  
  // Render current selected item
  if (menuIndex == 0) lcd.print("1.Inicio");
  else if (menuIndex == 1) {
    if (camState == CAM_DETENIDO) lcd.print("2.Iniciar");
    else lcd.print("2.Detener");
  }
  else if (menuIndex == 2) lcd.print("3.Configuracion");
  else if (menuIndex == 3) lcd.print("4.Perfiles");
  else if (menuIndex == 4) lcd.print("5.Estadisticas");

  // Render the next available item on the second line
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

/**
 * @brief Loads the names of all 6 stored profiles into RAM to speed up menu rendering.
 */
void cargarCacheNombres() {
  for(int i = 0; i < 6; i++) {
    obtenerNombrePerfil(i, listaNombres[i]);
  }
}

// --- Main UI Loop ---

/**
 * @brief Core UI state machine. Evaluates encoder inputs and handles screen rendering.
 * Must be called continuously in the main loop().
 */
void updateDisplay() {
  unsigned long currentMillis = millis();

  // ==========================================
  // INPUT HANDLING & LOGIC PER STATE
  // ==========================================

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
      // Dynamic acceleration: turn encoder fast to increment by larger steps
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

    // Toggle between progress bar and stats every 2 seconds if active and limited
    if (perfilActual.limiteFotos > 0 && (currentMillis - ultimoCambioDisplay >= 2000)) {
      mostrarProgreso = !mostrarProgreso;
      ultimoCambioDisplay = currentMillis;
      redraw = true;
    }
    
    // Force a 1-second refresh for the active timer
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

  else if (currentState == ESTADO_MENU_PERFILES) {
    if (encoderDelta != 0) {
      subMenuIndex += (encoderDelta > 0) ? 1 : -1;
      if (subMenuIndex < 0) subMenuIndex = 0;
      if (subMenuIndex > 2) subMenuIndex = 2;
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
        menuIndex = 3; 
      }
      redraw = true;
    }
  }

  else if (currentState == ESTADO_CARGAR_PERFIL) {
    if (encoderDelta != 0) {
      subMenuIndex += (encoderDelta > 0) ? 1 : -1;
      if (subMenuIndex < 0) subMenuIndex = 0;
      if (subMenuIndex > 6) subMenuIndex = 6; 
      redraw = true;
    }
    
    if (buttonPressed) {
      if (subMenuIndex == 6) { 
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

  else if (currentState == ESTADO_GUARDAR_PERFIL) {
    if (encoderDelta != 0) {
      subMenuIndex += (encoderDelta > 0) ? 1 : -1;
      if (subMenuIndex < 0) subMenuIndex = 0;
      if (subMenuIndex > 6) subMenuIndex = 6; 
      redraw = true;
    }
    
    if (buttonPressed) {
      if (subMenuIndex == 6) { 
        currentState = ESTADO_MENU_PERFILES;
        subMenuIndex = 1; 
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

  // ==========================================
  // SCREEN RENDERING (Only if redraw == true)
  // ==========================================

  if (redraw) {
    switch (currentState) {
      case ESTADO_MENU_PRINCIPAL:
        dibujarMenuPrincipal();
        break;

      case ESTADO_INICIO:
        lcd.clear();
        {
          unsigned long tUsoMostrado = 0;
          
          if (camState != CAM_DETENIDO) {
            tUsoMostrado = millis() - tiempoInicioSesion;
          } else {
            // Read the EXACT stored time if the camera is stopped
            tUsoMostrado = duracionUltimaSesion;
          }

          // Show the animated progress bar only if limited and active
          bool mostrarBarra = (perfilActual.limiteFotos > 0) && (camState != CAM_DETENIDO) && mostrarProgreso;

          if (mostrarBarra) {
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
            lcd.print("Fts:"); lcd.print(fotosTomadasSesion);
            if (perfilActual.limiteFotos > 0) {
              lcd.print("/"); lcd.print(perfilActual.limiteFotos);
            }
            lcd.setCursor(0, 1);
            lcd.print("t uso:");
            imprimirTiempo(tUsoMostrado, 7, 1);
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
        
        // Show the cursor block under the active character
        lcd.setCursor(cursorEdit, 1);
        lcd.cursor(); 
        lcd.noBlink(); 
        break;
    }
    
    // Ensure the cursor is hidden when exiting edit mode
    if (currentState != ESTADO_EDITAR_NOMBRE) {
      lcd.noBlink();
      lcd.noCursor();
    }
    
    redraw = false;
  }
}
