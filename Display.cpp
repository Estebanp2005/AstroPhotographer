#include "Display.h"
#include "Config.h"
#include "Input.h"
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2); // Ajustar a 0x3F si la pantalla no enciende

const int NUM_OPCIONES_MENU = 5;
String menuOpciones[NUM_OPCIONES_MENU] = {
  "1.Inicio", 
  "2.Inic/Detener", 
  "3.Configuracion", 
  "4.Cargar perfil", 
  "5.Estadisticas"
};

bool redraw = true; // Solo repintamos la pantalla cuando hay cambios

void initDisplay() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(" AstroTracker ");
  lcd.setCursor(0, 1);
  lcd.print(" Iniciando... ");
  delay(1500); // Único delay del setup
  lcd.clear();
}

void dibujarMenuPrincipal() {
  if (!redraw) return;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(">"); // Cursor indicador
  lcd.print(menuOpciones[menuIndex]);
  
  // Mostrar la siguiente opción en la línea inferior si existe
  if (menuIndex < NUM_OPCIONES_MENU - 1) {
    lcd.setCursor(1, 1);
    lcd.print(menuOpciones[menuIndex + 1]);
  }
  redraw = false;
}

void updateDisplay() {
  // 1. Lógica de Navegación (Giro del encoder)
  if (encoderDelta != 0) {
    if (currentState == ESTADO_INICIO) { 
      menuIndex += (encoderDelta > 0) ? 1 : -1;
      
      if (menuIndex < 0) menuIndex = 0;
      if (menuIndex >= NUM_OPCIONES_MENU) menuIndex = NUM_OPCIONES_MENU - 1;
      
      redraw = true;
    }
    // Más adelante agregaremos qué pasa al girar el encoder DENTRO de los submenús
  }

  // 2. Lógica de Selección (Clic del botón)
  if (buttonPressed) {
    if (currentState == ESTADO_INICIO) {
      // Entramos al submenú correspondiente
      if (menuIndex == 0) currentState = ESTADO_INICIO; // Se queda donde está
      else if (menuIndex == 1) currentState = ESTADO_INICIAR_DETENER;
      else if (menuIndex == 2) currentState = ESTADO_CONFIG;
      else if (menuIndex == 3) currentState = ESTADO_PERFILES;
      else if (menuIndex == 4) currentState = ESTADO_ESTADISTICAS;
      
      redraw = true; 
    } 
    else {
      // Si hacemos clic en CUALQUIER otro estado, por ahora volvemos al menú principal
      currentState = ESTADO_INICIO;
      redraw = true;
    }
  }

  // 3. Dibujo de la pantalla según el estado actual
  if (redraw) {
    switch (currentState) {
      case ESTADO_INICIO:
        dibujarMenuPrincipal();
        break;
        
      case ESTADO_INICIAR_DETENER:
        lcd.clear();
        lcd.print("--- CAMARA ---");
        lcd.setCursor(0,1);
        lcd.print("Click p/ volver");
        break;
        
      case ESTADO_CONFIG:
        lcd.clear();
        lcd.print("--- CONFIG ---");
        lcd.setCursor(0,1);
        lcd.print("Click p/ volver");
        break;
        
      case ESTADO_PERFILES:
        lcd.clear();
        lcd.print("-- PERFILES --");
        lcd.setCursor(0,1);
        lcd.print("Click p/ volver");
        break;
        
      case ESTADO_ESTADISTICAS:
        lcd.clear();
        lcd.print("- ESTADISTICAS -");
        lcd.setCursor(0,1);
        lcd.print("Click p/ volver");
        break;
    }
    
    // Si no estamos en el menú de inicio, bajamos la bandera de redibujo
    // (dibujarMenuPrincipal() ya se encarga de bajar la suya propia)
    if (currentState != ESTADO_INICIO) {
      redraw = false; 
    }
  }
}