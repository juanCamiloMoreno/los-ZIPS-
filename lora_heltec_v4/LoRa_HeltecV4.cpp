/**
 * @file LoRa_HeltecV4.cpp
 * @brief Implementacion de la clase LoRaHeltecV4
 */

#include "LoRa_HeltecV4.h"

// Constructor - Inicializa el radio con los pines del Heltec V4
LoRaHeltecV4::LoRaHeltecV4() :
    spi(nullptr),
    radio(new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY)),
    initialized(false),
    lastStatus(RADIOLIB_ERR_NONE)
{
}

bool LoRaHeltecV4::begin(float frequency, float bandwidth, uint8_t spreadingFactor,
                          uint8_t codingRate, uint8_t syncWord, int8_t power) {

    // Habilitar alimentacion externa (para OLED y LoRa boost)
    setVext(true);
    delay(100);

    // Inicializar pines del amplificador de potencia
    initPA();

    // Configurar SPI con los pines del Heltec V4
    spi = new SPIClass(HSPI);
    spi->begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);

    // Configurar el modulo para usar nuestro SPI
    radio.LORA_CS = LORA_CS;

    Serial.println("[LoRa] Inicializando SX1262...");
    Serial.printf("[LoRa] Frecuencia: %.2f MHz\n", frequency);
    Serial.printf("[LoRa] Ancho de banda: %.1f kHz\n", bandwidth);
    Serial.printf("[LoRa] SF: %d, CR: 4/%d\n", spreadingFactor, codingRate);
    Serial.printf("[LoRa] Potencia: %d dBm\n", power);

    // Inicializar el radio SX1262
    // Parametros: freq, bw, sf, cr, syncWord, power, preambleLen, tcxoVoltage, useRegulatorLDO
    lastStatus = radio.begin(
        frequency,
        bandwidth,
        spreadingFactor,
        codingRate,
        syncWord,
        power,
        LORA_PREAMBLE_LEN,
        SX126X_DIO3_TCXO_VOLTAGE,  // Voltaje TCXO
        false                       // Usar DC-DC (no LDO)
    );

    if (lastStatus != RADIOLIB_ERR_NONE) {
        Serial.printf("[LoRa] ERROR: Fallo al inicializar. Codigo: %d\n", lastStatus);
        return false;
    }

    Serial.println("[LoRa] Radio inicializado correctamente");

    // Configurar limite de corriente para el PA
    lastStatus = radio.setCurrentLimit(LORA_CURRENT_LIMIT);
    if (lastStatus != RADIOLIB_ERR_NONE) {
        Serial.printf("[LoRa] ADVERTENCIA: No se pudo configurar limite de corriente. Codigo: %d\n", lastStatus);
    }

    // Configurar DIO2 como switch RF automatico
    #ifdef SX126X_DIO2_AS_RF_SWITCH
    lastStatus = radio.setDio2AsRfSwitch(true);
    if (lastStatus != RADIOLIB_ERR_NONE) {
        Serial.printf("[LoRa] ADVERTENCIA: No se pudo configurar DIO2 como RF switch. Codigo: %d\n", lastStatus);
    }
    Serial.println("[LoRa] DIO2 configurado como RF switch");
    #endif

    // Habilitar CRC
    lastStatus = radio.setCRC(RADIOLIB_SX126X_LORA_CRC_ON);
    if (lastStatus != RADIOLIB_ERR_NONE) {
        Serial.printf("[LoRa] ADVERTENCIA: No se pudo habilitar CRC. Codigo: %d\n", lastStatus);
    }

    initialized = true;
    Serial.println("[LoRa] Configuracion completa");

    return true;
}

int16_t LoRaHeltecV4::send(uint8_t* data, size_t len) {
    if (!initialized) {
        return RADIOLIB_ERR_CHIP_NOT_FOUND;
    }

    // Activar PA para transmision
    setTxEnable(true);

    // Enviar datos
    lastStatus = radio.transmit(data, len);

    // Desactivar PA despues de TX
    setTxEnable(false);

    if (lastStatus == RADIOLIB_ERR_NONE) {
        Serial.printf("[LoRa] Enviados %d bytes\n", len);
    } else {
        Serial.printf("[LoRa] Error al enviar. Codigo: %d\n", lastStatus);
    }

    return lastStatus;
}

int16_t LoRaHeltecV4::send(const String& str) {
    return send((uint8_t*)str.c_str(), str.length());
}

int16_t LoRaHeltecV4::receive(uint8_t* data, size_t maxLen, uint32_t timeout) {
    if (!initialized) {
        return RADIOLIB_ERR_CHIP_NOT_FOUND;
    }

    // Desactivar PA (modo RX)
    setTxEnable(false);

    // Recibir con timeout
    if (timeout > 0) {
        lastStatus = radio.receive(data, maxLen, timeout);
    } else {
        lastStatus = radio.receive(data, maxLen);
    }

    if (lastStatus == RADIOLIB_ERR_NONE) {
        size_t len = radio.getPacketLength();
        Serial.printf("[LoRa] Recibidos %d bytes, RSSI: %.1f dBm, SNR: %.1f dB\n",
                      len, getRSSI(), getSNR());
        return len;
    } else if (lastStatus == RADIOLIB_ERR_RX_TIMEOUT) {
        return 0;
    }

    Serial.printf("[LoRa] Error al recibir. Codigo: %d\n", lastStatus);
    return lastStatus;
}

int16_t LoRaHeltecV4::receive(String& str, uint32_t timeout) {
    if (!initialized) {
        return RADIOLIB_ERR_CHIP_NOT_FOUND;
    }

    setTxEnable(false);

    if (timeout > 0) {
        lastStatus = radio.receive(str, timeout);
    } else {
        lastStatus = radio.receive(str);
    }

    return lastStatus;
}

int16_t LoRaHeltecV4::startReceive() {
    if (!initialized) {
        return RADIOLIB_ERR_CHIP_NOT_FOUND;
    }

    setTxEnable(false);
    lastStatus = radio.startReceive();
    return lastStatus;
}

bool LoRaHeltecV4::available() {
    if (!initialized) {
        return false;
    }

    // Verificar si hay interrupcion pendiente
    uint16_t irqFlags = radio.getIrqFlags();

    // Verificar flag de RX completado
    if (irqFlags & RADIOLIB_SX126X_IRQ_RX_DONE) {
        return true;
    }

    return false;
}

int16_t LoRaHeltecV4::readData(uint8_t* data, size_t maxLen) {
    if (!initialized) {
        return RADIOLIB_ERR_CHIP_NOT_FOUND;
    }

    lastStatus = radio.readData(data, maxLen);

    if (lastStatus == RADIOLIB_ERR_NONE) {
        return radio.getPacketLength();
    }

    return lastStatus;
}

float LoRaHeltecV4::getRSSI() {
    return radio.getRSSI();
}

float LoRaHeltecV4::getSNR() {
    return radio.getSNR();
}

float LoRaHeltecV4::getFrequencyError() {
    return radio.getFrequencyError();
}

int16_t LoRaHeltecV4::setFrequency(float freq) {
    lastStatus = radio.setFrequency(freq);
    return lastStatus;
}

int16_t LoRaHeltecV4::setOutputPower(int8_t power) {
    // Limitar potencia maxima
    if (power > 22) power = 22;
    if (power < -9) power = -9;

    lastStatus = radio.setOutputPower(power);
    return lastStatus;
}

int16_t LoRaHeltecV4::setSpreadingFactor(uint8_t sf) {
    lastStatus = radio.setSpreadingFactor(sf);
    return lastStatus;
}

int16_t LoRaHeltecV4::setBandwidth(float bw) {
    lastStatus = radio.setBandwidth(bw);
    return lastStatus;
}

int16_t LoRaHeltecV4::sleep() {
    if (!initialized) {
        return RADIOLIB_ERR_CHIP_NOT_FOUND;
    }

    // Desactivar PA
    #ifdef USE_GC1109_PA
    digitalWrite(LORA_PA_EN, LOW);
    digitalWrite(LORA_PA_TX_EN, LOW);
    #endif

    lastStatus = radio.sleep(true);  // Mantener configuracion
    Serial.println("[LoRa] Radio en modo sleep");
    return lastStatus;
}

int16_t LoRaHeltecV4::standby() {
    lastStatus = radio.standby();
    return lastStatus;
}

String LoRaHeltecV4::getStatus() {
    String status = "[LoRa] Estado: ";

    if (!initialized) {
        status += "No inicializado";
    } else if (lastStatus == RADIOLIB_ERR_NONE) {
        status += "OK";
    } else {
        status += "Error (";
        status += String(lastStatus);
        status += ")";
    }

    return status;
}

int16_t LoRaHeltecV4::setRxBoostedGain(bool enable) {
    lastStatus = radio.setRxBoostedGainMode(enable);
    return lastStatus;
}

void LoRaHeltecV4::setDio1Action(void (*callback)()) {
    radio.setDio1Action(callback);
}

void LoRaHeltecV4::clearDio1Action() {
    radio.clearDio1Action();
}

// ============================================================================
// Metodos privados
// ============================================================================

void LoRaHeltecV4::initPA() {
    #ifdef USE_GC1109_PA
    Serial.println("[LoRa] Inicializando amplificador GC1109...");

    // Pin de alimentacion del PA
    pinMode(LORA_PA_POWER, OUTPUT);
    digitalWrite(LORA_PA_POWER, HIGH);

    // Pin de habilitacion del PA
    pinMode(LORA_PA_EN, OUTPUT);
    digitalWrite(LORA_PA_EN, LOW);  // Inicialmente deshabilitado

    // Pin de habilitacion de TX
    pinMode(LORA_PA_TX_EN, OUTPUT);
    digitalWrite(LORA_PA_TX_EN, LOW);

    Serial.println("[LoRa] PA inicializado");
    #endif
}

void LoRaHeltecV4::setTxEnable(bool txEnable) {
    #ifdef USE_GC1109_PA
    digitalWrite(LORA_PA_POWER, HIGH);
    digitalWrite(LORA_PA_EN, HIGH);
    digitalWrite(LORA_PA_TX_EN, txEnable ? HIGH : LOW);
    #endif
}

void LoRaHeltecV4::setVext(bool enable) {
    // VEXT es activo bajo en Heltec V4
    pinMode(VEXT_ENABLE, OUTPUT);
    digitalWrite(VEXT_ENABLE, enable ? LOW : HIGH);
}
