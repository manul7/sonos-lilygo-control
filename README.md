# TTGO Sonos Volume Controller

ESP32-based hardware volume knob for Sonos speakers using a TTGO T-Display board.

## Features

- Large volume display on TFT screen
- Two-button control: up/down volume
- Auto-repeat when button held
- Single-button fallback: short press +, long press -
- WiFi connectivity to Sonos via UPnP/SOAP

### Planned

- **Rotary encoder**: rotation adjusts volume, push toggles mute/unmute
- **Button**: play/stop toggle

## Hardware

- **Board**: TTGO T-Display (ESP32 with 1.14" TFT, 240x135)
- **Buttons**: GPIO35 (A), GPIO0 (B)

### Planned

- **Rotary encoder**: with push button (CLK, DT, SW pins)

## Project Structure

```
ttgo_counter/
├── ttgo_counter.ino   # Main sketch (UI, buttons, WiFi)
├── config.h           # Configuration (excluded from git)
├── config.h.example   # Configuration template
├── sonos.h            # Sonos API declarations
├── sonos.cpp          # Sonos UPnP/SOAP implementation
└── README.md
```

## Dependencies

Install via Arduino Library Manager:

| Library | Purpose |
|---------|---------|
| TFT_eSPI | Display driver |
| WiFi | ESP32 WiFi (built-in) |
| HTTPClient | HTTP requests (built-in) |

### TFT_eSPI Configuration

In `User_Setup_Select.h`, enable:
```cpp
#include <User_Setups/Setup25_TTGO_T_Display.h>
```

## Configuration

Copy `config.h.example` to `config.h` and edit:

```cpp
// WiFi
const char* WIFI_SSID = "YourNetwork";
const char* WIFI_PASSWORD = "YourPassword";

// Sonos
const char* SONOS_IP = "192.168.x.x";  // Your Sonos speaker IP
constexpr int SONOS_PORT = 1400;
constexpr int VOLUME_STEP = 2;

// Hardware & Display
constexpr int BUTTON_A_PIN = 35;
constexpr int BUTTON_B_PIN = 0;
constexpr uint8_t DISPLAY_ROTATION = 1;
constexpr bool DISPLAY_INVERT = true;

// Timing
constexpr uint32_t DEBOUNCE_MS = 35;
constexpr uint32_t LONG_PRESS_MS = 700;
constexpr uint32_t REPEAT_INTERVAL_MS = 300;
```

Note: `config.h` contains credentials and is excluded from version control.

## Usage

- **Button A (GPIO35)**: Volume up
- **Button B (GPIO0)**: Volume down
- Hold either button for auto-repeat

Single-button mode activates automatically if only one button is detected.
