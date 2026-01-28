/**
 * @file receptor_irq.ino
 * @brief Receptor LoRa con interrupciones (no bloqueante)
 *
 * Este ejemplo usa interrupciones para detectar paquetes entrantes,
 * permitiendo que el ESP32 haga otras tareas mientras espera.
 *
 * Hardware: Heltec WiFi LoRa 32 V4 (ESP32-S3 + SX1262)
 */

#include <Arduino.h>
#include "../LoRa_HeltecV4.h"

// Crear instancia del radio
LoRaHeltecV4 lora;

// Flag de interrupcion (volatile porque se modifica en ISR)
volatile bool paqueteRecibido = false;

// Buffer para datos
uint8_t buffer[LORA_MAX_PACKET_SIZE];

// Contadores
uint32_t paquetes = 0;

// Rutina de servicio de interrupcion (ISR)
// IMPORTANTE: Debe ser muy corta - solo setear un flag
#if defined(ESP32)
void IRAM_ATTR onReceive() {
#else
void onReceive() {
#endif
    paqueteRecibido = true;
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println();
    Serial.println("========================================");
    Serial.println("  Heltec V4 - Receptor LoRa (IRQ)");
    Serial.println("========================================");
    Serial.println();

    // Inicializar LoRa
    if (!lora.begin(915.0, 125.0, 12, 8, 0x12, 17)) {
        Serial.println("ERROR: No se pudo inicializar LoRa");
        while (true) delay(1000);
    }

    // Configurar callback de interrupcion
    lora.setDio1Action(onReceive);

    // Iniciar recepcion continua
    int16_t estado = lora.startReceive();
    if (estado != RADIOLIB_ERR_NONE) {
        Serial.printf("ERROR: No se pudo iniciar recepcion. Codigo: %d\n", estado);
        while (true) delay(1000);
    }

    Serial.println("Radio en modo recepcion continua");
    Serial.println("Esperando paquetes...");
    Serial.println();
}

void loop() {
    // Verificar flag de interrupcion
    if (paqueteRecibido) {
        // Deshabilitar interrupciones mientras procesamos
        lora.clearDio1Action();

        // Limpiar flag
        paqueteRecibido = false;

        // Leer datos del paquete
        int16_t bytesRecibidos = lora.readData(buffer, sizeof(buffer));

        if (bytesRecibidos > 0) {
            paquetes++;
            buffer[bytesRecibidos] = '\0';

            Serial.println("----------------------------------------");
            Serial.printf("[RX] Paquete #%d: %s\n", paquetes, buffer);
            Serial.printf("[RX] %d bytes | RSSI: %.1f dBm | SNR: %.1f dB\n",
                          bytesRecibidos, lora.getRSSI(), lora.getSNR());
            Serial.println("----------------------------------------");
        } else {
            Serial.printf("[RX] Error al leer paquete. Codigo: %d\n", bytesRecibidos);
        }

        // Reconfigurar interrupcion y reiniciar recepcion
        lora.setDio1Action(onReceive);
        lora.startReceive();
    }

    // Aqui puedes hacer otras tareas mientras esperas paquetes
    // Por ejemplo, leer sensores, actualizar display, etc.

    // Pequena pausa para no saturar el CPU
    delay(10);
}
