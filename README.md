# 👻 RootedCON 2026 CTF Badge - Hardware Pwn 

![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![ESP32](https://img.shields.io/badge/ESP32-E7352C?style=for-the-badge&logo=espressif&logoColor=white)
![PlatformIO](https://img.shields.io/badge/PlatformIO-F6822B?style=for-the-badge&logo=PlatformIO&logoColor=white)
![RootedCON](https://img.shields.io/badge/RootedCON-2026-8A2BE2?style=for-the-badge)

Este repositorio documenta el proceso de ingeniería inversa, *hardware hacking* y desarrollo de un firmware personalizado para el *badge* oficial del CTF de la **RootedCON 2026**.

El objetivo es sustituir el juego de "Pac-Man" que trae por defecto por un *firmware* propio, superando el reto de no disponer de los esquemas originales (*datasheets* de la placa) mediante técnicas de *fuzzing* físico y reversing.

## 🛠️ Especificaciones del Hardware (Reconocimiento)

A través de la extracción del firmware original y el análisis de la placa, hemos identificado los siguientes componentes:

* **Cerebro:** ESP32-D0WD-V3 (Doble núcleo a 240MHz, Wi-Fi + Bluetooth).
* **Memoria:** 4MB Flash.
* **Pantalla:** LCD ST7735 de 1.77" (160x128 RGB) comunicada por SPI.
* **Controles:** Cruceta direccional + 2 botones de acción.

## 🗺️ Hoja de Ruta (Roadmap)

- [x] **Fase 1: Extracción y Backup:** Volcado de la memoria Flash de 4MB (`esptool`) para salvaguardar el firmware original (Pac-Man).
- [x] **Fase 2: Análisis Estático:** Búsqueda de credenciales y *pinout* oculto mediante `binwalk`, `strings` y `hexdump`.
- [ ] **Fase 3: Fuzzing Físico:** Desarrollo de un *script* en C++ para descubrir el mapeo de los GPIO (botones y pantalla) por fuerza bruta interactiva.
- [ ] **Fase 4: Pwn / Custom Firmware:** Desarrollo del firmware final interactivo usando librerías TFT y funciones nativas del ESP32.

## ⚙️ Entorno de Desarrollo

Para replicar o compilar este proyecto necesitarás:
* Visual Studio Code
* Extensión PlatformIO
* Drivers CP210x / CH340 (dependiendo del conversor USB-Serial de tu placa)

---
*Inspirado por los aportes de la comunidad de Hardware Hacking de la RootedCON.*
