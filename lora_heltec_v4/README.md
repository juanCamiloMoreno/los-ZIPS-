# LoRa Heltec V4 - Codigo Limpio

Codigo limpio y simplificado para manejar comunicacion LoRa en la placa **Heltec WiFi LoRa 32 V4** (ESP32-S3 + SX1262).

Basado en el firmware de Meshtastic v2.7.15.

## Hardware Soportado

- **Placa:** Heltec WiFi LoRa 32 V4
- **MCU:** ESP32-S3 (240 MHz, 8MB PSRAM)
- **Radio:** Semtech SX1262
- **PA:** GC1109 (Amplificador de potencia externo)
- **Pantalla:** OLED SSD1315 128x64

## Estructura del Proyecto

```
lora_heltec_v4/
├── config_heltec_v4.h    # Definicion de pines y constantes
├── LoRa_HeltecV4.h       # Clase principal (header)
├── LoRa_HeltecV4.cpp     # Implementacion
├── platformio.ini        # Configuracion PlatformIO
├── README.md             # Este archivo
└── examples/
    ├── transmisor/       # Ejemplo TX simple
    ├── receptor/         # Ejemplo RX bloqueante
    └── receptor_irq/     # Ejemplo RX con interrupciones
```

## Instalacion

### PlatformIO (Recomendado)

1. Clona este repositorio
2. Abre la carpeta `lora_heltec_v4` en VS Code con PlatformIO
3. Selecciona el entorno deseado:
   - `heltec_v4_tx` - Transmisor
   - `heltec_v4_rx` - Receptor
   - `heltec_v4_rx_irq` - Receptor con IRQ

### Arduino IDE

1. Instala la libreria **RadioLib** desde el Administrador de Bibliotecas
2. Instala el soporte para ESP32-S3 (URL: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json)
3. Selecciona la placa "Heltec WiFi LoRa 32 (V3)" (compatible con V4)
4. Copia todos los archivos `.h` y `.cpp` a tu proyecto

## Uso Basico

### Transmisor

```cpp
#include "LoRa_HeltecV4.h"

LoRaHeltecV4 lora;

void setup() {
    Serial.begin(115200);

    // Inicializar con parametros por defecto (915 MHz)
    if (!lora.begin()) {
        Serial.println("Error al inicializar LoRa");
        while(1);
    }
}

void loop() {
    // Enviar mensaje
    lora.send("Hola desde Heltec V4!");
    delay(5000);
}
```

### Receptor

```cpp
#include "LoRa_HeltecV4.h"

LoRaHeltecV4 lora;
uint8_t buffer[255];

void setup() {
    Serial.begin(115200);
    lora.begin();
}

void loop() {
    // Esperar paquete (timeout 10 seg)
    int bytes = lora.receive(buffer, sizeof(buffer), 10000);

    if (bytes > 0) {
        buffer[bytes] = '\0';
        Serial.printf("Recibido: %s\n", buffer);
        Serial.printf("RSSI: %.1f dBm\n", lora.getRSSI());
    }
}
```

## Configuracion de Pines (Heltec V4)

| Funcion     | Pin GPIO |
|-------------|----------|
| LORA_SCK    | 9        |
| LORA_MISO   | 11       |
| LORA_MOSI   | 10       |
| LORA_CS     | 8        |
| LORA_RST    | 12       |
| LORA_DIO1   | 14       |
| LORA_BUSY   | 13       |
| PA_POWER    | 7        |
| PA_EN       | 2        |
| PA_TX_EN    | 46       |
| VEXT_ENABLE | 36       |

## Parametros LoRa por Defecto

| Parametro         | Valor      |
|-------------------|------------|
| Frecuencia        | 915.0 MHz  |
| Ancho de Banda    | 125 kHz    |
| Spreading Factor  | 12         |
| Coding Rate       | 4/8        |
| Sync Word         | 0x12       |
| Potencia TX       | 17 dBm     |

## Frecuencias por Region

| Region          | Frecuencia (MHz) |
|-----------------|------------------|
| Americas (US)   | 902-928          |
| Europa (EU)     | 863-870          |
| Australia       | 915-928          |
| Asia            | 920-925          |
| India           | 865-867          |

## API de la Clase

### Inicializacion

```cpp
bool begin(float freq, float bw, uint8_t sf, uint8_t cr, uint8_t sync, int8_t power);
```

### Transmision

```cpp
int16_t send(uint8_t* data, size_t len);
int16_t send(const String& str);
```

### Recepcion

```cpp
int16_t receive(uint8_t* data, size_t maxLen, uint32_t timeout);
int16_t startReceive();          // Modo no bloqueante
bool available();                 // Verificar si hay datos
int16_t readData(uint8_t* data, size_t maxLen);
```

### Metricas

```cpp
float getRSSI();                  // Intensidad de senal
float getSNR();                   // Relacion senal/ruido
float getFrequencyError();        // Error de frecuencia
```

### Configuracion

```cpp
int16_t setFrequency(float freq);
int16_t setOutputPower(int8_t power);
int16_t setSpreadingFactor(uint8_t sf);
int16_t setBandwidth(float bw);
```

### Control de Energia

```cpp
int16_t sleep();                  // Modo bajo consumo
int16_t standby();                // Modo standby
```

## Codigos de Error Comunes

| Codigo | Descripcion                  |
|--------|------------------------------|
| 0      | RADIOLIB_ERR_NONE (exito)    |
| -2     | ERR_CHIP_NOT_FOUND           |
| -3     | ERR_PACKET_TOO_LONG          |
| -5     | ERR_TX_TIMEOUT               |
| -6     | ERR_RX_TIMEOUT               |
| -7     | ERR_CRC_MISMATCH             |

## Notas Importantes

1. **TCXO:** El Heltec V4 usa un TCXO de 1.8V en DIO3 para mejor estabilidad de frecuencia.

2. **PA Externo:** El GC1109 debe activarse para TX y desactivarse para RX. La clase lo maneja automaticamente.

3. **VEXT:** El pin 36 controla la alimentacion del OLED y boost del LoRa. Es activo bajo.

4. **Regulaciones:** Verifica las regulaciones de tu pais antes de transmitir. No excedas los limites de potencia permitidos.

## Licencia

Este codigo es de dominio publico. Basado en el firmware de Meshtastic.
