/**
 * @file receptor.ino
 * @brief Ejemplo de receptor LoRa para Heltec WiFi LoRa 32 V4
 *
 * Este ejemplo recibe paquetes LoRa continuamente y muestra
 * la informacion del paquete recibido (datos, RSSI, SNR).
 *
 * Hardware: Heltec WiFi LoRa 32 V4 (ESP32-S3 + SX1262)
 *
 * Libreria requerida:
 *   - RadioLib (https://github.com/jgromes/RadioLib)
 */

#include <Arduino.h>
#include "../LoRa_HeltecV4.h"

// Crear instancia del radio
LoRaHeltecV4 lora;

// Buffer para datos recibidos
uint8_t buffer[LORA_MAX_PACKET_SIZE];

// Contadores de estadisticas
uint32_t paquetesRecibidos = 0;
uint32_t errores = 0;

void setup() {
    // Inicializar puerto serial
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println();
    Serial.println("========================================");
    Serial.println("  Heltec V4 - Receptor LoRa");
    Serial.println("========================================");
    Serial.println();

    // Inicializar LoRa con los mismos parametros que el transmisor
    bool exito = lora.begin(
        915.0,    // Frecuencia MHz (debe coincidir con TX)
        125.0,    // Ancho de banda kHz
        12,       // Spreading Factor
        8,        // Coding Rate
        0x12,     // Sync Word (debe coincidir con TX)
        17        // Potencia dBm
    );

    if (!exito) {
        Serial.println("ERROR: No se pudo inicializar el radio LoRa");
        while (true) {
            delay(1000);
        }
    }

    // Habilitar ganancia boosted para mejor recepcion
    lora.setRxBoostedGain(true);

    Serial.println("Radio LoRa listo para recibir!");
    Serial.println("Esperando paquetes...");
    Serial.println();
}

void loop() {
    // Intentar recibir un paquete (timeout de 10 segundos)
    int16_t bytesRecibidos = lora.receive(buffer, sizeof(buffer), 10000);

    if (bytesRecibidos > 0) {
        // Paquete recibido exitosamente
        paquetesRecibidos++;

        // Terminar string con null
        buffer[bytesRecibidos] = '\0';

        // Obtener metricas
        float rssi = lora.getRSSI();
        float snr = lora.getSNR();
        float freqError = lora.getFrequencyError();

        // Mostrar informacion
        Serial.println("----------------------------------------");
        Serial.printf("[RX] Paquete #%d recibido!\n", paquetesRecibidos);
        Serial.printf("[RX] Datos (%d bytes): %s\n", bytesRecibidos, buffer);
        Serial.printf("[RX] RSSI: %.1f dBm\n", rssi);
        Serial.printf("[RX] SNR: %.1f dB\n", snr);
        Serial.printf("[RX] Error de frecuencia: %.1f Hz\n", freqError);

        // Indicador de calidad de senal
        if (rssi > -70) {
            Serial.println("[RX] Calidad: Excelente");
        } else if (rssi > -85) {
            Serial.println("[RX] Calidad: Buena");
        } else if (rssi > -100) {
            Serial.println("[RX] Calidad: Aceptable");
        } else {
            Serial.println("[RX] Calidad: Debil");
        }

        Serial.println("----------------------------------------");
        Serial.println();

    } else if (bytesRecibidos == 0) {
        // Timeout - no se recibio nada
        Serial.println("[RX] Timeout - esperando mas paquetes...");

    } else {
        // Error
        errores++;
        Serial.printf("[RX] Error al recibir. Codigo: %d\n", bytesRecibidos);

        // Intentar reiniciar el receptor
        if (errores > 5) {
            Serial.println("[RX] Demasiados errores, reiniciando radio...");
            lora.standby();
            delay(100);
            errores = 0;
        }
    }
}
