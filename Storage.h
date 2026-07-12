#ifndef STORAGE_H
#define STORAGE_H

void initStorage();
void cargarEstadisticas();
void guardarEstadisticas();
void cargarPerfil(int slot);
void guardarPerfil(int slot);
void obtenerNombrePerfil(int slot, char* buffer);

#endif
