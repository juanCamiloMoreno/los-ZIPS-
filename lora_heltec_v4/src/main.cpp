/**
 * @file main.cpp
 * @brief Ejemplo LoRa para Heltec WiFi LoRa 32 V4 (ESP32-S3 + SX1262)
 *
 * Cambia el modo TX/RX descomentando la linea correspondiente
 */

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

// ============================================================================
// CONFIGURACION - CAMBIA AQUI SEGUN TU NECESIDAD
// ============================================================================

// Descomentar UNA de las siguientes lineas:
#define MODO_TRANSMISOR
// #define MODO_RECEPTOR

// Frecuencia (MHz) - Ajustar segun tu region
#define LORA_FREQUENCY      915.0   // Americas: 915, Europa: 868

// Parametros LoRa
#define LORA_BANDWIDTH      125.0   // kHz
#define LORA_SPREADING      12      // Factor de dispersion (7-12)
#define LORA_CODING_RATE    8       // Coding rate (5-8)
#define LORA_SYNC_WORD      0x12    // Palabra de sincronizacion
#define LORA_TX_POWER       17      // Potencia dBm (max 22)
#define LORA_PREAMBLE       8       // Longitud preambulo

// ============================================================================
// PINES HELTEC V4 (ESP32-S3 + SX1262)
// ============================================================================

// SPI para el SX1262
#define LORA_SCK    9
#define LORA_MISO   11
#define LORA_MOSI   10
#define LORA_CS     8

// Control del SX1262
#define LORA_RST    12
#define LORA_DIO1   14      // IRQ
#define LORA_BUSY   13

// TCXO
#define TCXO_VOLTAGE 1.8f

// Amplificador de potencia GC1109
#define PA_POWER    7
#define PA_EN       2
#define PA_TX_EN    46

// Control de alimentacion
#define VEXT_ENABLE 36      // Activo bajo

// ============================================================================
// VARIABLES GLOBALES
// ============================================================================

// Crear instancia del radio SX1262
SX1262 radio = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);

// Buffer de datos
uint8_t buffer[255];
uint32_t contador = 0;

// ============================================================================
// FUNCIONES DE INICIALIZACION
// ============================================================================

void initVext() {
    // VEXT controla alimentacion del OLED y LoRa (activo bajo)
    pinMode(VEXT_ENABLE, OUTPUT);
    digitalWrite(VEXT_ENABLE, LOW);  // Encender
    delay(100);
}

void initPA() {
    // Inicializar pines del amplificador GC1109
    pinMode(PA_POWER, OUTPUT);
    digitalWrite(PA_POWER, HIGH);

    pinMode(PA_EN, OUTPUT);
    digitalWrite(PA_EN, LOW);

    pinMode(PA_TX_EN, OUTPUT);
    digitalWrite(PA_TX_EN, LOW);
}

void setTxMode(bool tx) {
    // Configurar PA para TX o RX
    digitalWrite(PA_POWER, HIGH);
    digitalWrite(PA_EN, HIGH);
    digitalWrite(PA_TX_EN, tx ? HIGH : LOW);
}

bool initLoRa() {
    Serial.println("[LoRa] Inicializando SX1262...");

    // Configurar SPI
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);

    // Inicializar radio
    int state = radio.begin(
        LORA_FREQUENCY,
        LORA_BANDWIDTH,
        LORA_SPREADING,
        LORA_CODING_RATE,
        LORA_SYNC_WORD,
        LORA_TX_POWER,
        LORA_PREAMBLE,
        TCXO_VOLTAGE,
        false  // Usar DC-DC, no LDO
    );

    if (state != RADIOLIB_ERR_NONE) {
        Serial.printf("[LoRa] ERROR: Codigo %d\n", state);
        return false;
    }

    // Configurar limite de corriente (140mA para SX1262)
    radio.setCurrentLimit(140.0);

    // DIO2 como switch RF
    radio.setDio2AsRfSwitch(true);

    // Habilitar CRC
    radio.setCRC(RADIOLIB_SX126X_LORA_CRC_ON);

    Serial.println("[LoRa] Inicializado correctamente!");
    Serial.printf("[LoRa] Frecuencia: %.2f MHz\n", LORA_FREQUENCY);
    Serial.printf("[LoRa] BW: %.1f kHz, SF: %d, CR: 4/%d\n",
                  LORA_BANDWIDTH, LORA_SPREADING, LORA_CODING_RATE);

    return true;
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("========================================");
    #ifdef MODO_TRANSMISOR
    Serial.println("  Heltec V4 - TRANSMISOR LoRa");
    #else
    Serial.println("  Heltec V4 - RECEPTOR LoRa");
    #endif
    Serial.println("========================================");
    Serial.println();

    // Inicializar hardware
    initVext();
    initPA();

    // Inicializar LoRa
    if (!initLoRa()) {
        Serial.println("[ERROR] No se pudo inicializar LoRa!");
        while (true) {
            delay(1000);
        }
    }

    #ifdef MODO_RECEPTOR
    // Habilitar ganancia mejorada en RX
    radio.setRxBoostedGainMode(true);
    Serial.println("[LoRa] Esperando paquetes...");
    #endif

    Serial.println();
}

// ============================================================================
// LOOP
// ============================================================================

#ifdef MODO_TRANSMISOR

void loop() {
    // Crear mensaje
    String mensaje = "Heltec V4 #" + String(contador);

    Serial.printf("[TX] Enviando: \"%s\"... ", mensaje.c_str());

    // Activar PA para transmision
    setTxMode(true);

    // Enviar
    int state = radio.transmit(mensaje);

    // Desactivar TX
    setTxMode(false);

    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("OK!");
        contador++;
    } else {
        Serial.printf("Error %d\n", state);
    }

    // Esperar 5 segundos
    delay(5000);
}

#else  // MODO_RECEPTOR

void loop() {
    // Configurar para recepcion
    setTxMode(false);

    // Intentar recibir (timeout 10 segundos)
    String mensaje;
    int state = radio.receive(mensaje, 10000);

    if (state == RADIOLIB_ERR_NONE) {
        // Paquete recibido!
        contador++;

        Serial.println("----------------------------------------");
        Serial.printf("[RX] Paquete #%d recibido!\n", contador);
        Serial.printf("[RX] Mensaje: %s\n", mensaje.c_str());
        Serial.printf("[RX] RSSI: %.1f dBm\n", radio.getRSSI());
        Serial.printf("[RX] SNR: %.1f dB\n", radio.getSNR());
        Serial.println("----------------------------------------");
        Serial.println();

    } else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
        Serial.println("[RX] Timeout - esperando...");

    } else {
        Serial.printf("[RX] Error: %d\n", state);
    }
}

#endif
