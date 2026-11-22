# ESP8266 Dual OLED Eyes POC

Prueba de concepto (POC) de ojos robóticos animados y sincronizados en **dos pantallas OLED 0.96" (`gm009606v4`, `128×64`)** usando un **ESP8266 NodeMCU v3**, pensada como base de interfaz expresiva para un futuro **Agente de IA**.

---

## Características de la POC
- **1 ojo grande por pantalla (`128×64`):** Escalado respecto al diseño clásico de 2 ojos en una sola pantalla, aprovechando el ancho completo para desplazamientos de mirada fluidos.
- **Sin soldadura en las pantallas:** Ambas pantallas conservan su dirección de fábrica (`0x3C`) conectadas en dos buses independientes mediante `Wire.pins(sda, scl)` nativo del ESP8266 (~35–40 FPS a 700 kHz).
- **Motor de animación procedural:** Primitivas vectoriales con interpolación suave (*lerp*), parpadeo automático aleatorio y micro-movimientos de mirada en reposo (*idle*).
- **Secuencia automatizada de emociones:** Recorre automáticamente `NEUTRAL`, `HAPPY`, `SURPRISED`, `ANGRY`, `SAD`, `THINKING`, `WINK` (asimétrico) y `SLEEPY`.
- **Control manual por Monitor Serie:** Permite probar cualquier emoción o disparar un parpadeo al instante.
- **Pruebas unitarias nativas (`env:native`):** Lógica de estados e interpolación verificable en PC con Unity sin flashear la placa.

---

## Diagrama Esquemático de Conexiones

```text
                       ESP8266 NodeMCU v3
                     +--------------------+
                     |               3V3  |---+-----------------------+
                     |               GND  |---|---+-------------------+
                     |                    |   |   |                   |
   OLED Izquierdo    |   D1 (GPIO5)  SCL  |---|---|-+                 |
  +--------------+   |   D2 (GPIO4)  SDA  |---|---|---+-+             |
  | GND          |<---------------------------|---+ | | |             |
  | VDD          |<--+--------------------+   |   | | | |             |
  | SCK  (0x3C)  |<---------------------------|-----+ | |             |
  | SDA          |<---------------------------|-------+ |             |
  +--------------+   |                    |   |   |     |             |
                     |   D5 (GPIO14) SCL  |---|---|-----|---+         |
   OLED Derecho      |   D6 (GPIO12) SDA  |---|---|-----|---|---+     |
  +--------------+   |                    |   |   |     |   |   |     |
  | GND          |<-------------------------------+     |   |   |     |
  | VDD          |<--+--------------------+---+         |   |   |     |
  | SCK  (0x3C)  |<-------------------------------------+---+   |     |
  | SDA          |<---------------------------------------------+     |
  +--------------+   +--------------------+
```

### Tabla de Pinout

| Señal | OLED Izquierdo (`0x3C`) | OLED Derecho (`0x3C`) | Pin NodeMCU v3 | GPIO ESP8266 |
| :--- | :--- | :--- | :--- | :--- |
| **Alimentación (+)** | `VDD` | `VDD` | `3V3` | — |
| **Tierra (-)** | `GND` | `GND` | `GND` | — |
| **Reloj I2C** | `SCK` | — | `D1` | `GPIO5` |
| **Datos I2C** | `SDA` | — | `D2` | `GPIO4` |
| **Reloj I2C** | — | `SCK` | `D5` | `GPIO14` |
| **Datos I2C** | — | `SDA` | `D6` | `GPIO12` |

> **Nota de estabilidad:** Si tus cables Dupont son largos (>15 cm) y notas píxeles corruptos o congelamientos, reduce `I2C_CLOCK_HZ` en `include/config.h` de `700000` a `400000`.

---

## Estructura del Proyecto

```text
.
├── include/
│   ├── config.h            # Pines, dirección I2C, clock y FPS objetivo
│   ├── eye_types.h         # Tipos puros (Emotion, EyeParams, DualEyeState)
│   ├── eye_controller.h    # Máquina de estados, interpolación y parpadeo
│   └── display_manager.h   # Driver dual SSD1306Wire y renderizado procedural
├── src/
│   ├── eye_controller.cpp  # Lógica de animaciones (portable a native)
│   ├── display_manager.cpp # Renderizado en pantallas OLED
│   └── main.cpp            # Setup, loop a 40 FPS y comandos Serial
├── test/
│   └── test_eye_controller.cpp # Pruebas unitarias Unity (env:native)
├── platformio.ini
└── README.md
```

---

## Compilación, Carga y Pruebas (PlatformIO)

### 1. Ejecutar pruebas unitarias en PC (`native`)
```bash
pio test -e native
```

### 2. Compilar firmware (`nodemcuv2`)
```bash
pio run -e nodemcuv2
```

### 3. Cargar a la NodeMCU v3
```bash
pio run -e nodemcuv2 -t upload
```

### 4. Abrir Monitor Serie (`115200` baudios)
```bash
pio device monitor
```

### Comandos disponibles en el Monitor Serie
- `0` .. `7`: Fijar emoción manual (`0: NEUTRAL`, `1: HAPPY`, `2: SURPRISED`, `3: ANGRY`, `4: SAD`, `5: THINKING`, `6: WINK`, `7: SLEEPY`)
- `a`: Reanudar secuencia automática (modo demo)
- `b`: Disparar parpadeo manual
- `h` o `?`: Mostrar ayuda

---

## Proyección hacia Agente de IA
Esta arquitectura separa completamente la **lógica de expresiones** (`EyeController`) del **renderizado hardware** (`DisplayManager`), lo que permite escalar fácilmente a:
1. **Control remoto por Wi-Fi (WebSocket / HTTP / MQTT):** Recibir comandos JSON desde un agente de IA, por ejemplo:
   `{"emotion": "thinking", "look": [12, -5], "blink": true}`
2. **Estados de conversación:** Mapear `listening`, `thinking`, `speaking`, `idle` y `error` a expresiones en tiempo real.
3. **Configuración persistente en `LittleFS`:** Guardar credenciales Wi-Fi y offsets de calibración física de cada pantalla en `/config.json`.
