#include "Storage.h"
#include "Config.h"
#include <EEPROM.h>

// Definimos dónde empieza cada cosa en la memoria emulada
const int ADDR_STATS = 0;
const int ADDR_PROFILES_START = sizeof(Estadisticas); // Arranca justo después de las estadísticas

void initStorage() {
  EEPROM.begin(512); // Reservamos 512 bytes de la flash (más que suficiente)
  cargarEstadisticas();
}

void cargarEstadisticas() {
  EEPROM.get(ADDR_STATS, statsGlobales);
  
  // Si la placa es completamente nueva, la flash viene llena de "255" (0xFFFFFFFF).
  // Si detectamos esto, inicializamos las estadísticas en cero.
  if (statsGlobales.totalFotosHistorico == 0xFFFFFFFF) {
    statsGlobales.totalFotosHistorico = 0;
    statsGlobales.tiempoTotalFunc = 0;
    guardarEstadisticas();
  }
}

void guardarEstadisticas() {
  EEPROM.put(ADDR_STATS, statsGlobales);
  EEPROM.commit(); // ¡Obligatorio en ESP8266 para guardar físicamente!
}

void cargarPerfil(int slot) {
  if (slot < 0 || slot >= 6) return; // Validación de seguridad (6 slots: 0 al 5)
  
  int direccion = ADDR_PROFILES_START + (slot * sizeof(Perfil));
  EEPROM.get(direccion, perfilActual);
  
  // Si el slot está vacío o corrupto, le cargamos valores por defecto seguros
  if (perfilActual.nombre[0] == 0xFF || perfilActual.nombre[0] == '\0') {
    snprintf(perfilActual.nombre, sizeof(perfilActual.nombre), "%d. Vacio", slot + 1);
    perfilActual.tiempoEntreFotos = 5.0;
    perfilActual.tiempoObturacion = 2.0;
    perfilActual.limiteFotos = 0;
  }
}

void guardarPerfil(int slot) {
  if (slot < 0 || slot >= 6) return;
  
  int direccion = ADDR_PROFILES_START + (slot * sizeof(Perfil));
  
  // Forzamos que los dos primeros caracteres sean el slot ("1.", "2.", etc.) como pediste
  perfilActual.nombre[0] = '1' + slot;
  perfilActual.nombre[1] = '.';
  
  EEPROM.put(direccion, perfilActual);
  EEPROM.commit(); // Guardamos en la flash
}