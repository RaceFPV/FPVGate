#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Shared SPI mutex for Waveshare ESP32-S3-LCD-2 board
// The LCD and SD card share FSPI pins (MOSI=38, SCK=39, MISO=40)
// All SPI bus access must be serialized through this mutex.

#if defined(WAVESHARE_ESP32S3_LCD2) && defined(ENABLE_LCD_UI) && ENABLE_LCD_UI

extern SemaphoreHandle_t g_spiMutex;

inline void spiMutexInit() {
    g_spiMutex = xSemaphoreCreateMutex();
}

inline bool spiMutexTake(TickType_t timeout = portMAX_DELAY) {
    if (g_spiMutex) {
        return xSemaphoreTake(g_spiMutex, timeout) == pdTRUE;
    }
    return true;  // No mutex = no contention
}

inline void spiMutexGive() {
    if (g_spiMutex) {
        xSemaphoreGive(g_spiMutex);
    }
}

#else

// No-op on boards without shared SPI bus
inline void spiMutexInit() {}
inline bool spiMutexTake(TickType_t timeout = 0) { return true; }
inline void spiMutexGive() {}

#endif
