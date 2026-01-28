/**
 * @file config_heltec_v4.h
 * @brief Configuracion de pines para Heltec WiFi LoRa 32 V4 (ESP32-S3 + SX1262)
 *
 * Basado en el firmware Meshtastic 2.7.15
 *
 * Hardware:
 *   - MCU: ESP32-S3
 *   - Radio LoRa: SX1262
 *   - Pantalla: SSD1306/SSD1315 OLED 128x64
 *   - GPS: L76K (opcional)
 */

#ifndef CONFIG_HELTEC_V4_H
#define CONFIG_HELTEC_V4_H

// ============================================================================
// CONFIGURACION DEL RADIO LoRa SX1262
// ============================================================================

// Pines SPI para el SX1262
#define LORA_SCK    9       // SPI Clock
#define LORA_MISO   11      // SPI Master In Slave Out
#define LORA_MOSI   10      // SPI Master Out Slave In
#define LORA_CS     8       // Chip Select (NSS)

// Pines de control del SX1262
#define LORA_RST    12      // Reset del radio
#define LORA_DIO1   14      // Interrupcion IRQ
#define LORA_BUSY   13      // Pin BUSY del SX1262

// Configuracion del TCXO (oscilador de cristal controlado por temperatura)
#define SX126X_DIO3_TCXO_VOLTAGE 1.8f  // Voltaje del TCXO en DIO3

// El SX1262 usa DIO2 como switch RF (controla TX/RX automaticamente)
#define SX126X_DIO2_AS_RF_SWITCH

// ============================================================================
// AMPLIFICADOR DE POTENCIA GC1109 (Heltec V4 tiene PA externo)
// ============================================================================

#define USE_GC1109_PA           // Habilitar control del PA externo
#define LORA_PA_POWER   7       // Pin de alimentacion del PA
#define LORA_PA_EN      2       // Habilitacion del PA
#define LORA_PA_TX_EN   46      // Habilitacion de transmision

// ============================================================================
// PANTALLA OLED SSD1306/SSD1315
// ============================================================================

#define OLED_SDA    17          // I2C Data
#define OLED_SCL    18          // I2C Clock
#define OLED_RST    21          // Reset de la pantalla

#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_ADDR   0x3C        // Direccion I2C

// ============================================================================
// BOTONES Y LED
// ============================================================================

#define BUTTON_PIN  0           // Boton PRG (Boot)
#define BUTTON2_PIN 35          // Boton secundario (algunos modelos)
#define LED_PIN     35          // LED integrado

// ============================================================================
// CONTROL DE ENERGIA
// ============================================================================

#define VEXT_ENABLE 36          // Control de alimentacion externa (activo bajo)
                                // Alimenta la pantalla OLED y boost del LoRa

#define BATTERY_PIN 1           // Pin ADC para lectura de bateria
#define ADC_CTRL    37          // Control del ADC

// ============================================================================
// GPS L76K (si esta disponible)
// ============================================================================

#define GPS_TX_PIN      38      // TX del GPS -> RX del ESP32
#define GPS_RX_PIN      39      // RX del GPS <- TX del ESP32
#define GPS_RESET_PIN   42      // Reset del GPS
#define GPS_ENABLE_PIN  34      // Habilitacion del GPS
#define GPS_STANDBY_PIN 40      // Control de standby
#define GPS_PPS_PIN     41      // Pulso por segundo

// ============================================================================
// PARAMETROS LoRa POR DEFECTO (Banda ISM 915 MHz para Americas)
// ============================================================================

#define LORA_FREQUENCY      915.0f      // Frecuencia en MHz (cambiar segun region)
#define LORA_BANDWIDTH      125.0f      // Ancho de banda en kHz
#define LORA_SPREADING      12          // Factor de dispersion (7-12)
#define LORA_CODING_RATE    8           // Tasa de codificacion (5-8 = 4/5 a 4/8)
#define LORA_SYNC_WORD      0x12        // Palabra de sincronizacion (0x12 LoRa privado)
#define LORA_TX_POWER       17          // Potencia de transmision en dBm (max 22)
#define LORA_PREAMBLE_LEN   8           // Longitud del preambulo

// Corriente maxima del PA
#define LORA_CURRENT_LIMIT  140.0f      // mA

// ============================================================================
// FRECUENCIAS POR REGION
// ============================================================================

// Descomenta la region que corresponda:
// #define LORA_REGION_EU868    868.0f    // Europa
// #define LORA_REGION_US915    915.0f    // Americas
// #define LORA_REGION_AU915    915.0f    // Australia
// #define LORA_REGION_AS923    923.0f    // Asia
// #define LORA_REGION_IN865    865.0f    // India
// #define LORA_REGION_KR920    920.0f    // Corea

#endif // CONFIG_HELTEC_V4_H
