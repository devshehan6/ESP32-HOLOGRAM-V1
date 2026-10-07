# MECH-SEQ // S3 — ESP32 Sequence Controller

ESP32-S3 based IR-triggered mechanical sequence controller with a built-in web interface for pattern editing, delay tuning, and persistent storage.

---

## 📌 Overview

This project turns an ESP32-S3 into a standalone mechanical sequencer. When an IR sensor detects an object (HIGH → LOW transition), the ESP32 plays a pre-programmed sequence across 7 output pins with configurable step delays. The entire pattern matrix, delay, and row count can be edited live through a browser-based UI — no re-uploading required.

---

## ✨ Features

- 🎯 **7-channel output sequencer** (L1–L7)
- 👁️ **IR sensor trigger** with edge detection (HIGH → LOW)
- 🌐 **Built-in web UI** (dark theme, mobile-friendly)
- ⏱️ **Adjustable step delay** — 1 µs to 1,000,000 µs (1 s)
- 📊 **Pattern matrix editor** — toggle each pin per step
- 💾 **Persistent storage** using ESP32 `Preferences` (NVS)
- 📤 **Import / Export** patterns as JSON
- 📶 **WiFi + AP fallback** — auto-creates `MECH-SEQ-S3` hotspot if WiFi fails
- 🧠 **localStorage caching** on browser side

---

## 🧰 Required Materials

| Item | Qty | Notes |
|------|-----|-------|
| ESP32-S3 DevKit | 1 | Any S3 variant with ≥ 11 usable GPIOs |
| IR Sensor Module | 1 | e.g. FC-51 / TCRT5000 (digital out) |
| Output devices | 7 | LEDs, relays, MOSFETs, drivers, etc. |
| Resistors | as needed | For LEDs / driver inputs |
| External 5V supply | 1 | Recommended for relays/loads |
| Jumper wires | — | — |
| Breadboard / PCB | 1 | — |

> ⚠️ **ESP32 GPIOs are 3.3 V.** Do NOT drive relays or motors directly — use a transistor/MOSFET/relay module.

---

## 🔌 Pin Configuration

### Output pins (as defined in code)

| Label | GPIO | Function |
|-------|------|----------|
| L1 | 21 | Step output 1 |
| L2 | 20 | Step output 2 |
| L3 | 10 | Step output 3 |
| L4 | 9  | Step output 4 |
| L5 | 8  | Step output 5 |
| L6 | 7  | Step output 6 |
| L7 | 6  | Step output 7 |

### Input

| Label | GPIO | Function |
|-------|------|----------|
| IR_PIN | 5 | IR sensor input (INPUT_PULLUP) |

> 🔧 **Change pins here** in the source code:
> ```cpp
> const int pins[7] = { 21, 20, 10, 9, 8, 7, 6 };
> const int IR_PIN = 5;
> ```

---

## 📐 Wiring Diagram

See **`hologram V1.png`** in the repository root.

```
   ┌─────────────┐
   │  IR SENSOR  │──── OUT ──► GPIO 5
   │  (VCC/GND)  │
   └─────────────┘

   ┌──────────────────────────────┐
   │         ESP32-S3             │
   │  GPIO 21 ──► L1  (LED/Relay) │
   │  GPIO 20 ──► L2              │
   │  GPIO 10 ──► L3              │
   │  GPIO  9 ──► L4              │
   │  GPIO  8 ──► L5              │
   │  GPIO  7 ──► L6              │
   │  GPIO  6 ──► L7              │
   └──────────────────────────────┘
```
### Don't trust the image included pin numbers. make a project and change pins in your code
<img src="hologram V1.png" width="100%">

---

## ⚙️ User Configurable Settings

Open the `.ino` / `.cpp` file and edit the top section:

### 1. WiFi Credentials
```cpp
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```
If connection fails within ~10 seconds, the ESP32 starts its own AP:
- **SSID:** `MECH-SEQ-S3`
- **Password:** `12345678`

### 2. Output Pins
```cpp
const int pins[7] = { 21, 20, 10, 9, 8, 7, 6 };
```
Change any GPIO number. Avoid boot-strapping pins (0, 3, 45, 46) if possible.

### 3. IR Sensor Pin
```cpp
const int IR_PIN = 5;
```
Input uses `INPUT_PULLUP`. The sensor should pull LOW when triggered.

### 4. Default Delay (µs)
```cpp
unsigned long delayUs = 100000;   // 100 ms
```
Range: `1` → `1,000,000`

### 5. Max Rows
```cpp
#define MAX_ROWS 100
```
Increase for longer sequences (watch RAM usage).

### 6. Default Pattern (in `setup()`)
```cpp
int defaultPattern[7][4] = {
    {1,1,1,1},
    {1,0,0,0},
    {1,1,1,1},
    {1,0,0,0},
    {1,1,1,1},
    {1,0,0,0},
    {1,1,1,1}
};
```
This loads only the first time (when no saved data exists in NVS).

---

## 🚀 Getting Started

1. **Install Arduino IDE** + **ESP32 board package** (Espressif).
2. Select board: **ESP32S3 Dev Module**.
3. Set **USB CDC On Boot → Enabled** (if using native USB).
4. Wire hardware as shown in `diagram.jpg`.
5. Update WiFi credentials and pins in code.
6. Upload the sketch.
7. Open **Serial Monitor @ 115200 baud** to see the assigned IP.
8. Open the IP in a browser → the MECH-SEQ UI loads.

---

## 🖥️ Using the Web Interface

| Section | Purpose |
|---------|---------|
| **Step Delay** | Set delay per step (µs). Buttons: ±1 µs, ±1 ms |
| **IR Sensor** | Live status display |
| **Current Sequence** | Shows active step + pin states |
| **Turn Count** | Number of rows in the sequence |
| **Pattern Matrix** | Toggle L1–L7 for each row |
| **EXPORT** | Download `seq.json` |
| **IMPORT** | Load `seq.json` from disk |
| **SAVE TO ESP** | Persist pattern + settings to NVS |

---

## 🔄 Sequence Behavior

1. IR sensor sees **HIGH → LOW** transition.
2. Sequence starts from **step 0**.
3. Each step outputs the pattern row across L1–L7.
4. After `delayUs` microseconds → next step.
5. After the last step (`t` rows) → all outputs go **LOW**, sequence stops.
6. Next IR trigger restarts from step 0.

> Retriggering during a running sequence is ignored (edge-triggered only).

---

## 💾 Data Persistence

Data is stored in **two places**:

| Location | What | When |
|----------|------|------|
| Browser `localStorage` (`mech-v5`) | UI cache | Auto on every UI change |
| ESP32 NVS (`Preferences` namespace `"seq"`) | Delay, t, rows | Only on **SAVE TO ESP** |

⚠️ Patterns are **NOT saved to NVS** in this version — only `delayUs`, `t`, and `rowCount`. Re-uploading the sketch reloads `defaultPattern`; use **IMPORT** to restore full pattern from JSON.

---

## 📡 API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/` | Web UI |
| GET | `/api/get` | Returns current config + patterns as JSON |
| POST | `/api/save` | Saves delay/t/rows (and parses patterns) |

**Example GET response:**
```json
{
  "delayUs": 100000,
  "t": 4,
  "rowCount": 4,
  "patterns": [[1,1,1,1],[1,0,0,0], ...]
}
```

---

## 🛠️ Troubleshooting

| Problem | Fix |
|---------|-----|
| No WiFi connection | Check SSID/password; verify 2.4 GHz network (ESP32 doesn't support 5 GHz) |
| Can't access web UI | Find IP via Serial Monitor; ensure device is on same network |
| IR not triggering | Confirm `INPUT_PULLUP` behaviour; sensor output should go LOW on detection |
| Outputs inverted | Swap `HIGH`/`LOW` in `applyCurrentStep()` |
| Pattern resets after reboot | Use **SAVE TO ESP**, and remember patterns need re-import |
| Sequence too fast/slow | Adjust delay in UI or change `delayUs` in code |

---

## 📁 File Structure

```
/
├── MECH-SEQ-S3.ino     # Main sketch (this file)
├── diagram.jpg         # Wiring diagram
├── README.md           # This file
└── seq.json            # (Optional) Exported pattern
```

---

## 📝 License

Free to use, modify, and distribute for personal and commercial projects.

---

## 👤 Author

**MECH-SEQ // S3** — Custom ESP32 sequencer firmware.

For questions, improvements, or bug reports, open an issue in the repository.
