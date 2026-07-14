# AstroTracker

**AstroTracker** es un intervalómetro avanzado y automatizado diseñado específicamente para la astrofotografía. Basado en el microcontrolador ESP8266, este dispositivo permite el control preciso del obturador y el enfoque de cámaras DSLR o Mirrorless mediante un sistema de relés, ofreciendo una gestión integral de sesiones fotográficas largas.

## Características Principales

* **Control Preciso:** Gestión de tiempos de obturación e intervalos entre fotos a través de un módulo de relés de 2 canales.
* **Gestión de Perfiles:** Almacenamiento no volátil de hasta 6 perfiles de disparo personalizables (nombre, tiempo de obturación, intervalo y límite de fotografías).
* **Interfaz de Usuario Intuitiva:** Navegación por menús mediante un encoder rotativo y visualización en tiempo real a través de una pantalla LCD 16x2 (I2C).
* **Estadísticas Globales:** Registro persistente en memoria EEPROM (LittleFS/Preferences) del total histórico de fotografías capturadas y el tiempo total de funcionamiento del equipo.
* **Notificaciones Acústicas:** Integración de un buzzer activo para alertas de finalización de secuencia.
* **Lógica Adaptativa:** Compensación de lógica inversa nativa para módulos de relés estándar y debounce gestionado por hardware/software para el encoder rotativo.

---

## Hardware Requerido

* Microcontrolador ESP8266 (NodeMCU V2/V3 o Wemos D1 Mini).
* Pantalla LCD 16x2 con módulo adaptador I2C.
* Encoder Rotativo (módulo estándar de 5 pines o encoder "pelado" con resistencias Pull-Up internas activadas por software).
* Módulo de Relés de 2 Canales (Lógica Inversa en mi caso).
* Buzzer Activo.
* Cable disparador compatible con el puerto de la cámara fotográfica a utilizar.
* Level Shifter 3V3 to 5V

---

## Esquema de Conexiones (Pinout)

La siguiente tabla describe la conexión de los periféricos a los pines GPIO del ESP8266 según la configuración del firmware:

| Componente | Pin del Componente | Pin ESP8266 |
| :--- | :--- | :--- |
| **Pantalla LCD (I2C)** | SCL | D1 |
| | SDA | D2 |
| **Relés de Cámara** | Foco (Focus) IN | D0 |
| | Obturador (Shutter) IN | D3 |
| **Encoder Rotativo** | Pin A (DT) | D5 |
| | Pin B (CLK) | D6 |
| | Switch (SW) | D7 |
| **Zumbador (Buzzer)**| Positivo (VCC/IN) | D8 |

> **Nota:** Asegúrese de proveer alimentación (VCC 3.3V/5V y GND) a la pantalla LCD, al encoder rotativo y al módulo de relés utilizando las salidas de alimentación de la placa o una fuente externa regulada.

---

## Dependencias y Librerías de Software

Para compilar este proyecto en el Arduino IDE, es necesario tener instalado el soporte para placas ESP8266 y las siguientes librerías:

1.  **LiquidCrystal_I2C** V2.0.0 by Igor Antolic, marcmerlin: Para la gestión del display.
2.  **Ai Esp32 Rotary Encoder** V1.7 by Martin Kubovčík, Frank de Brabander, John Rickman: Para la lectura asíncrona y aceleración del encoder. Aunque el nombre menciona ESP32, es totalmente compatible con ESP8266.
3.  **Preferences** V2.2.2 by Volodymyr Shymanskyy: Utilizada para emular la escritura segura en EEPROM a través del sistema de archivos LittleFS.


## Uso y Navegación

El sistema se opera enteramente a través del encoder rotativo (Girar para navegar/ajustar, Presionar para seleccionar).

* **Menú Principal:** Acceso al inicio de secuencia, configuración manual, carga/guardado de perfiles y visualización de estadísticas.
* **Inicio de Secuencia:** Permite forzar el enfoque (giro horario) o tomar una fotografía de prueba (giro antihorario) antes de comenzar la automatización. Durante la sesión, la pantalla muestra una barra de progreso y el tiempo transcurrido.
* **Configuración:** Ajuste en tiempo real del tiempo de obturación, intervalo entre fotos y límite de capturas.
* **Perfiles:** El sistema permite editar el nombre de cada perfil carácter por carácter para una fácil identificación de los setups fotográficos (ej. "Via Lactea", "Startrails").

---

## Instalación

1. Clonar este repositorio.
2. Abrir el archivo principal `Astrophotographer.ino` en el Arduino IDE.
3. Instalar las librerías mencionadas en la sección de dependencias.
4. Seleccionar la placa ESP8266 correspondiente en `Herramientas > Placa`.
5. Compilar y subir el firmware al microcontrolador.
