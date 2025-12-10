# CO3091 Logic Design - IoT Environmental Monitoring System

## 📋 Project Overview

Real-time environmental monitoring system using ESP32-S3 with **TensorFlow Lite Micro** for on-device anomaly detection. The system monitors temperature and humidity, performs ML inference, and provides visual feedback through LED indicators and web interface.

---

## 🔧 Hardware Requirements

- **Microcontroller**: ESP32-S3 (YOLO UNO)
- **Sensor**: DHT20 (Temperature & Humidity) - I2C pins 11, 12
- **Display**: 
  - LCD 16x2 (I2C: 0x27)
- **Indicators**:
  - Red LED (GPIO 21) - WiFi/Status
  - NeoPixel RGB LED (GPIO 16) - Anomaly/Status
- **Connectivity**: WiFi + MQTT (ThingsBoard)

---

## 🎯 Core Features

### ✅ Multi-tasking System (FreeRTOS)
- 5 concurrent tasks running in parallel
- Optimized stack allocation for each task
- Semaphore-based synchronization for critical sections

### ✅ Machine Learning on Edge
- TensorFlow Lite Micro inference
- Binary anomaly classification (NORMAL/ANOMALY)
- StandardScaler normalization applied in real-time
- ~50-100ms inference time

### ✅ Real-time Monitoring
- DHT20 sensor readings every ~4 seconds
- ML inference every 3 seconds
- Web dashboard for live data visualization
- MQTT integration with ThingsBoard

### ✅ Smart Indicators
- LED blinking based on WiFi status
- NeoPixel color/animation based on anomaly detection
- LCD display with sensor data and status

---

## 📊 Task Description

### **Task 1: led_blinky** (`src/led_blinky.cpp`)
**Purpose**: Indicate WiFi connection status using red LED

| Aspect | Details |
|--------|---------|
| **Frequency** | Continuous blinking |
| **Logic** | Fast blink (300ms) = WiFi disconnected<br>Slow blink (1000ms) = WiFi connected |
| **Stack Size** | 2048 bytes |
| **Priority** | 2 (Normal) |
| **Dependencies** | `isWifiConnected` (global flag) |
| **Output** | GPIO 21 (Red LED) |

**Code Flow**:
```cpp
if (!isWifiConnected) {
    // Fast blink 300ms on/off
} else {
    // Slow blink 1000ms on/off
}
```

---

### **Task 2: neo_blinky** (`src/neo_blinky.cpp`)
**Purpose**: Display anomaly detection status using NeoPixel RGB LED

| Aspect | Details |
|--------|---------|
| **Frequency** | Continuous animation |
| **Logic** | ANOMALY (true):<br>&nbsp;&nbsp;- Red color<br>&nbsp;&nbsp;- Fast blink (200ms on/off)<br>NORMAL (false):<br>&nbsp;&nbsp;- Green color<br>&nbsp;&nbsp;- Slow blink (2000ms on/off) |
| **Stack Size** | 2048 bytes |
| **Priority** | 2 (Normal) |
| **Dependencies** | `anomaly_detected` (from tiny_ml_task) |
| **Output** | GPIO 16 (NeoPixel) |
| **Features** | - Color-coded status<br>- Speed indicates severity |

**Code Flow**:
```cpp
if (anomaly_detected) {
    // Red + 200ms blink (urgent)
} else {
    // Green + 2000ms blink (normal)
}
```

---

### **Task 3: temp_humi_monitor** (`src/temp_humi_monitor.cpp`)
**Purpose**: Read DHT20 sensor and update global variables; display on LCD/OLED

| Aspect | Details |
|--------|---------|
| **Frequency** | Every ~4 seconds<br>(1s DHT read + 3s webserver delay) |
| **Data Source** | DHT20 (I2C: GPIO 11, 12) |
| **Sensors** | Temperature (°C)<br>Humidity (%) |
| **Stack Size** | 8192 bytes |
| **Priority** | 2 (Normal) |
| **Dependencies** | DHT20, LiquidCrystal_I2C, U8G2 |
| **Output** | LCD 16x2 (0x27)<br>OLED 128x64 (SH1106)<br>Global variables |

**Display Output**:
```
LCD:
Temp: 28.50
Humi: 65.36
Status: NORMAL
```

**Global Variables Updated**:
- `glob_temperature` - Current temperature
- `glob_humidity` - Current humidity
- Used by `tiny_ml_task` for inference

---

### **Task 4: tiny_ml_task** (`src/tinyml.cpp`)
**Purpose**: Real-time anomaly detection using TensorFlow Lite Micro ML model

| Aspect | Details |
|--------|---------|
| **Frequency** | Every 3 seconds |
| **Model** | Binary classification (NORMAL/ANOMALY) |
| **Input Features** | - Temperature (normalized)<br>- Humidity (normalized) |
| **Output** | Score 0.0-1.0 (threshold=0.5) |
| **Stack Size** | 16384 bytes |
| **Tensor Arena** | 32 KB |
| **Priority** | 2 (Normal) |
| **Dependencies** | `glob_temperature`, `glob_humidity` |
| **Output** | `anomaly_detected` (boolean) |

**Processing Pipeline**:

| Step | Operation | Example |
|------|-----------|---------|
| 1 | Read sensor data | temp=28.5°C, humidity=65.36% |
| 2 | Convert humidity range | 65.36% → 0.6536 |
| 3 | Normalize temperature | (28.5 - 29.95) / 10.02 = -0.1446 |
| 4 | Normalize humidity | (0.6536 - 0.6234) / 0.1447 = 0.2087 |
| 5 | Run inference | Model([−0.1446, 0.2087]) → 0.234 |
| 6 | Classify result | 0.234 < 0.5 → NORMAL |
| 7 | Update global flag | `anomaly_detected = false` |

**StandardScaler Parameters** (from Python training):
```cpp
TEMP_MEAN = 29.95°C,    TEMP_STD = 10.02
HUMID_MEAN = 0.6234,    HUMID_STD = 0.1447
```

**Inference Details**:
- Input: 2 features (temperature, humidity)
- Model architecture: Dense layers with ReLU + Sigmoid
- Output: Float 0.0-1.0 (anomaly probability)
- Threshold: 0.5 (> 0.5 = ANOMALY, ≤ 0.5 = NORMAL)

---

### **Task 5: coreiot_task** (`src/coreiot.cpp`)
**Purpose**: Send sensor data and anomaly status to ThingsBoard via MQTT

| Aspect | Details |
|--------|---------|
| **Frequency** | Every 10 seconds |
| **Protocol** | MQTT |
| **Server** | ThingsBoard Cloud (Core IoT) |
| **Data Sent** | - Temperature<br>- Humidity<br>- Anomaly status<br>- WiFi signal strength (RSSI) |
| **Stack Size** | 4096 bytes |
| **Priority** | 2 (Normal) |
| **Dependencies** | `glob_temperature`, `glob_humidity`, `anomaly_detected` |
| **Synchronization** | `xBinarySemaphoreInternet` (waits for WiFi) |

**JSON Payload Example**:
```json
{
  "temperature": 28.50,
  "humidity": 65.36,
  "anomaly_detected": false,
  "rssi": -67
}
```

**Workflow**:
```cpp
1. Wait for WiFi connection (semaphore)
2. Connect to MQTT broker
3. Send JSON with sensor data
4. Wait 10 seconds
5. Repeat
```

---

### **Task 6: task_power_optimize** (`src/task_power_optimize.cpp`)
**Purpose**: Manage power consumption using BOOT button (2 modes: Normal & Optimize)

| Aspect | Details |
|--------|--------|
| **Frequency** | Continuous monitoring (100ms polling) |
| **Modes** | 1. POWER_NORMAL - All tasks active, full brightness (80-100mA)<br>2. POWER_OPTIMIZE - Reduced load, dimmed displays (40-50mA) |
| **Input Control** | BOOT button (GPIO 0) - Hold > 1s to toggle between modes |
| **Stack Size** | 3072 bytes |
| **Priority** | 2 (Normal) |
| **Dependencies** | `anomaly_detected` (from tiny_ml_task) |
| **Output** | `current_power_state` (enum), Semaphores |
| **Synchronization** | `xBinarySemaphorePowerOptimize` (enter optimize mode)<br>`xBinarySemaphoreNormalMode` (return to normal) |

**Power Mode Transitions**:

```
POWER_NORMAL (100% load)
    ↓ (Hold BOOT > 1s)
POWER_OPTIMIZE (50% reduced)
    ↓ (Hold BOOT > 1s again)
POWER_NORMAL
```

**Features**:
- ✅ Manual mode switching via BOOT button (Hold > 1s)
- ✅ Semaphore signals to other tasks (neo_blinky reduces brightness to 50%)
- ✅ Auto-timeout protection (bootHandled flag prevents accidental switching)
- ✅ Real-time power state logging every 10 seconds
- ✅ WiFi stays active in both modes (always connected to server)

**GPIO Configuration**:
- **GPIO 0 (BOOT)**: Manual mode switch - Hold > 1s to toggle between NORMAL ↔ OPTIMIZE

---

## 📈 Task Execution Timeline (with Power Management)

```
Time(s) | led_blinky | neo_blinky | temp_humi | tiny_ml | coreiot | power_opt
--------|-----------|-----------|----------|---------|---------|----------
  0     | Blink     | 100% LED  | Read DHT | Setup   | -       | Monitor
  1     | Blink     | 100% LED  | Display  | -       | -       | Check BOOT
  3     | Blink     | 100% LED  | -        | Infer   | -       | Check BOOT
  4     | Blink     | 100% LED  | Read DHT | -       | -       | Check BOOT
  5     | Blink     | 50% DIM↓  | Display  | -       | -       | [User holds BOOT]
  6     | Blink     | 50% DIM↓  | -        | Infer   | -       | → POWER_OPTIMIZE
  7     | Blink     | 50% DIM↓  | -        | -       | -       | Check BOOT
  10    | Blink     | 50% DIM↓  | Read DHT | Infer   | Send    | Log state
  ...   | Repeat    | Repeat    | Repeat   | Repeat  | Repeat  | Repeat
```

---

## 🏗️ Architecture Diagram (with Power Management)

```
DHT20 Sensor (I2C)
      ↓
temp_humi_monitor (4s cycle)
      ↓
glob_temperature, glob_humidity
      ↓
tiny_ml_task (3s cycle)
      ├→ Normalize data (StandardScaler)
      ├→ Run TFLite inference
      └→ anomaly_detected (boolean)
      ↓
POWER MANAGEMENT LAYER:
┌──────────────────────────────────┐
│ task_power_optimize (100ms)      │
│ - Monitor BOOT button (GPIO0)    │
│ - Manage 2 power modes           │
│ - Signal via semaphores          │
└──────────────────────────────────┘
      ↓ (Semaphore signals)
Output tasks:
├→ neo_blinky (LED: 100% or 50% brightness)
├→ led_blinky (WiFi status indicator)
├→ temp_humi_monitor (LCD/OLED display)
└→ coreiot_task (MQTT → ThingsBoard)

Power States:
NORMAL (100%) ↔ OPTIMIZE (50% reduced)
```

---

## 🔌 GPIO Configuration

| Pin | Device | Function |
|-----|--------|----------|
| **GPIO 11, 12** | DHT20 (I2C SDA/SCL) | Temperature/Humidity sensor |
| **GPIO 21** | Red LED | WiFi status indicator |
| **GPIO 16** | NeoPixel | Anomaly status indicator |
| **GPIO 5** | Boot button | System control |
| **I2C 0x27** | LCD 16x2 | Display |
| **I2C (other)** | OLED SH1106 | Display |

---

## 📚 Libraries Used

- **TensorFlow Lite Micro** - ML inference
- **DHT20** - Temperature/humidity sensor
- **AsyncWebServer** - Web interface
- **ArduinoJson** - JSON serialization
- **PubSubClient** - MQTT client
- **LiquidCrystal_I2C** - LCD control
- **U8G2** - OLED control
- **Adafruit_NeoPixel** - RGB LED control

---

## 🐛 Troubleshooting

| Issue | Cause | Solution |
|-------|-------|----------|
| NaN inference | Missing normalization | Check StandardScaler parameters |
| Wrong predictions | Model not trained properly | Retrain model without BatchNorm |
| Stack overflow | Arena too small | Increase tensor_arena size |
| WiFi disconnects | Signal weak | Move closer to router |
| MQTT failed | Server down | Check ThingsBoard status |

---

## 📝 Notes

- **Multi-threading**: Tasks run with FreeRTOS, 6 tasks total (4KB-16KB stack each)
- **Semaphore Sync**: 3 semaphores for WiFi, Power Optimize, and Normal mode coordination
- **ML Inference**: TFLite Micro on-device (~50-100ms/run), no server dependency
- **Data Normalization**: StandardScaler applied in real-time before inference
- **Power Management**: 3 modes (100% → 50% → 10% power) controlled via BOOT button
- **Web Updates**: Dashboard refreshes every 3 seconds
- **GPIO Wakeup**: Light sleep can be triggered by GPIO7 (anomaly detection)
- **Brightness Control**: NeoPixel dims to 50% in POWER_OPTIMIZE mode via semaphore

---

## 🎯 Power Management Quick Reference

| Mode | Load | Display | WiFi | Indicator | Use Case |
|------|------|---------|------|-----------|----------|
| **NORMAL** | 100% | Full bright | Active | 100% LED | Full operation |
| **POWER_OPTIMIZE** | 50% | Dimmed (50%) | Active | 50% LED | Night/Low activity |

**Activation**: Hold BOOT button (GPIO0) for > 1 second to toggle between NORMAL ↔ POWER_OPTIMIZE

**Power Consumption**:
- NORMAL: 80-100mA (with WiFi, MQTT, full display)
- POWER_OPTIMIZE: 40-50mA (WiFi/MQTT stay on, display dimmed 50%)
