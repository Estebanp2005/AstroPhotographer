#include "Display.h"
#include "Config.h"
#include "Input.h"
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // Ajustar a 0x3F si la pantalla no enciende

const int NUM_OPCIONES_MENU = 5;
String menuOpciones[NUM_OPCIONES_MENU] = {
  "1. Inicio", 
  "2. Inic/Detener", 
  "3. Configuracion", 
  "4. Cargar perfil", 
  "5. Estadisticas"
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
  // Lógica global de navegación
  if (encoderDelta != 0) {
    if (currentState == ESTADO_INICIO) { // Asumiendo que ESTADO_INICIO es el Menú Principal por ahora
      menuIndex += (encoderDelta > 0) ? 1 : -1;
      // Limitar el índice entre 0 y el máximo de opciones
      if (menuIndex < 0) menuIndex = 0;
      if (menuIndex >= NUM_OPCIONES_MENU) menuIndex = NUM_OPCIONES_MENU - 1;
      redraw = true;
    }
  }

  if (buttonPressed) {
    // Aquí iría la lógica para entrar a cada submenú dependiendo de 'menuIndex'
    lcd.clear();
    lcd.print("Seleccion: ");
    lcd.setCursor(0,1);
    lcd.print(menuIndex + 1);
    delay(1000); // Solo para visualizar en la etapa de desarrollo
    redraw = true;
  }

  // Máquina de estados de la pantalla
  switch (currentState) {
    case ESTADO_INICIO:
      dibujarMenuPrincipal();
      break;
    // case ESTADO_CONFIG: ...
  }
}