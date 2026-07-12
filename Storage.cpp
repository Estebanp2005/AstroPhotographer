#include "Storage.h"
#include "Config.h"
#include <Preferences.h>

Preferences prefs;

void initStorage() {
  // Abre el "espacio" de memoria llamado "astro". 
  // El false significa modo Lectura/Escritura.
  prefs.begin("astro", false); 
  cargarEstadisticas();
}

void cargarEstadisticas() {
  // Si no encuentra la clave, devuelve 0 por defecto
  statsGlobales.totalFotosHistorico = prefs.getULong("totFotos", 0);
  statsGlobales.tiempoTotalFunc = prefs.getULong("tFunc", 0);
}

void guardarEstadisticas() {
  prefs.putULong("totFotos", statsGlobales.totalFotosHistorico);
  prefs.putULong("tFunc", statsGlobales.tiempoTotalFunc);
  // Preferences guarda automáticamente sin necesidad de "commit"
}

void cargarPerfil(int slot) {
  if (slot < 0 || slot >= 6) return; // Validación de seguridad (slots 0 al 5)
  
  char key[10];
  snprintf(key, sizeof(key), "prf%d", slot); // Genera la clave: prf0, prf1...

  // Si la clave no existe, cargamos un perfil limpio por defecto
  if (!prefs.isKey(key)) {
    snprintf(perfilActual.nombre, sizeof(perfilActual.nombre), "%d. Vacio", slot + 1);
    perfilActual.tiempoEntreFotos = 5.0;
    perfilActual.tiempoObturacion = 2.0;
    perfilActual.limiteFotos = 0;
  } else {
    // Si existe, leemos los bytes directos y rellenamos la estructura
    prefs.getBytes(key, &perfilActual, sizeof(Perfil));
  }
}

void guardarPerfil(int slot) {
  if (slot < 0 || slot >= 6) return;
  
  char key[10];
  snprintf(key, sizeof(key), "prf%d", slot);
  
  // Forzamos que los dos primeros caracteres sean el número de slot ("1.", "2.")
  perfilActual.nombre[0] = '1' + slot;
  perfilActual.nombre[1] = '.';
  
  // Guardamos toda la estructura de una sola vez
  prefs.putBytes(key, &perfilActual, sizeof(Perfil));
}

void obtenerNombrePerfil(int slot, char* buffer) {
  if (slot < 0 || slot >= 6) return;
  
  char key[10];
  snprintf(key, sizeof(key), "prf%d", slot);
  
  if (!prefs.isKey(key)) {
    // Si no hay nada guardado, devuelve "1. Vacio", etc.
    snprintf(buffer, 15, "%d. Vacio", slot + 1);
  } else {
    // Si hay datos, leemos el perfil temporalmente y robamos el nombre
    Perfil temp;
    prefs.getBytes(key, &temp, sizeof(Perfil));
    strncpy(buffer, temp.nombre, 15);
    buffer[14] = '\0'; // Terminador de seguridad
  }
}
