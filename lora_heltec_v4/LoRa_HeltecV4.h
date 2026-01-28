/**
 * @file LoRa_HeltecV4.h
 * @brief Clase para manejo de LoRa en Heltec WiFi LoRa 32 V4
 *
 * Utiliza RadioLib para comunicacion con el chip SX1262
 * Incluye soporte para el amplificador de potencia GC1109
 */

#ifndef LORA_HELTEC_V4_H
#define LORA_HELTEC_V4_H

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include "config_heltec_v4.h"

// Tamano maximo del buffer de datos
#define LORA_MAX_PACKET_SIZE 255

/**
 * @class LoRaHeltecV4
 * @brief Clase para comunicacion LoRa en Heltec V4 con SX1262
 */
class LoRaHeltecV4 {
public:
    /**
     * @brief Constructor de la clase
     */
    LoRaHeltecV4();

    /**
     * @brief Inicializa el radio LoRa
     * @param frequency Frecuencia en MHz (ej: 915.0)
     * @param bandwidth Ancho de banda en kHz (ej: 125.0)
     * @param spreadingFactor Factor de dispersion (7-12)
     * @param codingRate Tasa de codificacion (5-8)
     * @param syncWord Palabra de sincronizacion
     * @param power Potencia de transmision en dBm
     * @return true si la inicializacion fue exitosa
     */
    bool begin(float frequency = LORA_FREQUENCY,
               float bandwidth = LORA_BANDWIDTH,
               uint8_t spreadingFactor = LORA_SPREADING,
               uint8_t codingRate = LORA_CODING_RATE,
               uint8_t syncWord = LORA_SYNC_WORD,
               int8_t power = LORA_TX_POWER);

    /**
     * @brief Envia datos por LoRa (bloqueante)
     * @param data Puntero a los datos a enviar
     * @param len Longitud de los datos
     * @return Codigo de estado de RadioLib
     */
    int16_t send(uint8_t* data, size_t len);

    /**
     * @brief Envia una cadena por LoRa
     * @param str Cadena a enviar
     * @return Codigo de estado de RadioLib
     */
    int16_t send(const String& str);

    /**
     * @brief Recibe datos por LoRa (bloqueante con timeout)
     * @param data Buffer para almacenar datos recibidos
     * @param maxLen Tamano maximo del buffer
     * @param timeout Tiempo de espera en ms (0 = infinito)
     * @return Numero de bytes recibidos, o codigo de error negativo
     */
    int16_t receive(uint8_t* data, size_t maxLen, uint32_t timeout = 0);

    /**
     * @brief Recibe una cadena por LoRa
     * @param str String donde almacenar los datos
     * @param timeout Tiempo de espera en ms
     * @return Codigo de estado de RadioLib
     */
    int16_t receive(String& str, uint32_t timeout = 0);

    /**
     * @brief Inicia el modo de recepcion continua (no bloqueante)
     * @return Codigo de estado de RadioLib
     */
    int16_t startReceive();

    /**
     * @brief Verifica si hay datos disponibles (modo no bloqueante)
     * @return true si hay un paquete listo para leer
     */
    bool available();

    /**
     * @brief Lee el paquete recibido (usar despues de available())
     * @param data Buffer para los datos
     * @param maxLen Tamano del buffer
     * @return Numero de bytes leidos
     */
    int16_t readData(uint8_t* data, size_t maxLen);

    /**
     * @brief Obtiene el RSSI del ultimo paquete recibido
     * @return RSSI en dBm
     */
    float getRSSI();

    /**
     * @brief Obtiene el SNR del ultimo paquete recibido
     * @return SNR en dB
     */
    float getSNR();

    /**
     * @brief Obtiene el error de frecuencia del ultimo paquete
     * @return Error de frecuencia en Hz
     */
    float getFrequencyError();

    /**
     * @brief Cambia la frecuencia de operacion
     * @param freq Nueva frecuencia en MHz
     * @return Codigo de estado
     */
    int16_t setFrequency(float freq);

    /**
     * @brief Cambia la potencia de transmision
     * @param power Potencia en dBm (max 22 para SX1262)
     * @return Codigo de estado
     */
    int16_t setOutputPower(int8_t power);

    /**
     * @brief Cambia el factor de dispersion
     * @param sf Factor de dispersion (7-12)
     * @return Codigo de estado
     */
    int16_t setSpreadingFactor(uint8_t sf);

    /**
     * @brief Cambia el ancho de banda
     * @param bw Ancho de banda en kHz
     * @return Codigo de estado
     */
    int16_t setBandwidth(float bw);

    /**
     * @brief Pone el radio en modo sleep para ahorro de energia
     * @return Codigo de estado
     */
    int16_t sleep();

    /**
     * @brief Despierta el radio del modo sleep
     * @return Codigo de estado
     */
    int16_t standby();

    /**
     * @brief Obtiene el estado actual del radio
     * @return String con el estado
     */
    String getStatus();

    /**
     * @brief Habilita/deshabilita el modo de ganancia boosted en RX
     * @param enable true para habilitar
     * @return Codigo de estado
     */
    int16_t setRxBoostedGain(bool enable);

    /**
     * @brief Configura callback para interrupcion de recepcion
     * @param callback Funcion a llamar cuando se recibe un paquete
     */
    void setDio1Action(void (*callback)());

    /**
     * @brief Limpia la accion configurada en DIO1
     */
    void clearDio1Action();

    /**
     * @brief Acceso directo al objeto SX1262 de RadioLib
     * @return Referencia al objeto SX1262
     */
    SX1262& getRadio() { return radio; }

private:
    // Instancia de SPI
    SPIClass* spi;

    // Objeto del radio SX1262
    SX1262 radio;

    // Bandera de inicializacion
    bool initialized;

    // Ultimo estado
    int16_t lastStatus;

    /**
     * @brief Inicializa los pines del amplificador de potencia
     */
    void initPA();

    /**
     * @brief Configura el PA para transmision
     * @param txEnable true para TX, false para RX
     */
    void setTxEnable(bool txEnable);

    /**
     * @brief Habilita la alimentacion externa (VEXT)
     * @param enable true para habilitar
     */
    void setVext(bool enable);
};

#endif // LORA_HELTEC_V4_H
