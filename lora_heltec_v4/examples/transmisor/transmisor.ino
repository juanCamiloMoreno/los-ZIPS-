/**
 * @file transmisor.ino
 * @brief Ejemplo de transmisor LoRa para Heltec WiFi LoRa 32 V4
 *
 * Este ejemplo envia paquetes LoRa periodicamente.
 * Cada paquete contiene un contador y un mensaje.
 *
 * Hardware: Heltec WiFi LoRa 32 V4 (ESP32-S3 + SX1262)
 *
 * Conexiones:
 *   - No se requieren conexiones externas
 *   - El modulo LoRa esta integrado en la placa
 *
 * Libreria requerida:
 *   - RadioLib (https://github.com/jgromes/RadioLib)
 *
 * Instalacion en PlatformIO (platformio.ini):
 *   lib_deps = jgromes/RadioLib@^6.4.0
 *
 * Instalacion en Arduino IDE:
 *   Herramientas -> Administrar Bibliotecas -> Buscar "RadioLib"
 */

#include <Arduino.h>
#include "../LoRa_HeltecV4.h"

// Crear instancia del radio
LoRaHeltecV4 lora;

// Variables
uint32_t contador = 0;
uint32_t ultimoEnvio = 0;
const uint32_t INTERVALO_ENVIO = 5000;  // Enviar cada 5 segundos

void setup() {
    // Inicializar puerto serial
    Serial.begin(115200);
    while (!Serial && millis() < 3000);  // Esperar serial (max 3 seg)

    Serial.println();
    Serial.println("========================================");
    Serial.println("  Heltec V4 - Transmisor LoRa");
    Serial.println("========================================");
    Serial.println();

    // Inicializar LoRa con parametros por defecto
    // Puedes cambiar los parametros segun tu region
    bool exito = lora.begin(
        915.0,    // Frecuencia MHz (915 para Americas, 868 para Europa)
        125.0,    // Ancho de banda kHz
        12,       // Spreading Factor (7-12, mayor = mas alcance, menor velocidad)
        8,        // Coding Rate (5-8)
        0x12,     // Sync Word
        17        // Potencia dBm (max 22)
    );

    if (!exito) {
        Serial.println("ERROR: No se pudo inicializar el radio LoRa");
        Serial.println("Verifica las conexiones y reinicia el dispositivo");
        while (true) {
            delay(1000);
        }
    }

    Serial.println("Radio LoRa listo para transmitir!");
    Serial.println();
}

void loop() {
    // Verificar si es momento de enviar
    if (millis() - ultimoEnvio >= INTERVALO_ENVIO) {
        ultimoEnvio = millis();

        // Crear mensaje
        String mensaje = "Heltec V4 #" + String(contador);

        Serial.printf("[TX] Enviando: \"%s\" (%d bytes)...\n",
                      mensaje.c_str(), mensaje.length());

        // Medir tiempo de transmision
        uint32_t tiempoInicio = millis();

        // Enviar mensaje
        int16_t estado = lora.send(mensaje);

        uint32_t tiempoTx = millis() - tiempoInicio;

        if (estado == RADIOLIB_ERR_NONE) {
            Serial.printf("[TX] Exito! Tiempo de TX: %d ms\n", tiempoTx);
            contador++;
        } else {
            Serial.printf("[TX] Error al enviar. Codigo: %d\n", estado);
        }

        Serial.println();
    }
}
