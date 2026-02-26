# TTGO Sonos Volume Controller

<img src="docs/render_v1.jpg" alt="Hull" width="600">
<img src="docs/printed_hull_petg.jpg" alt="Hull" width="600">

ESP32-based hardware volume knob for Sonos speakers using a TTGO T-Display board.

## Features

- Volume displayed as a fill-proportional triangle with numeric readout
- Status icons (WiFi, Sonos connectivity, play/pause state) in top-right corner
- Rotary encoder for smooth volume adjustment with batching
- Encoder button toggles play/pause
- Built-in buttons as fallback (up/down with auto-repeat)
- WiFi connectivity to Sonos via UPnP/SOAP
- Auto-reconnect on WiFi drop

## Hardware

- **Board**: TTGO T-Display (ESP32 with 1.14" TFT, 240x135)
- **Rotary encoder**: CLK → GPIO25, DT → GPIO26, SW → GPIO27
- **Built-in buttons**: GPIO35 (A), GPIO0 (B)

## Project Structure

```
ttgo_counter/
├── ttgo_counter.ino   # Main sketch (UI, buttons, WiFi)
├── config.h           # Configuration (excluded from git)
├── config.h.example   # Configuration template
├── sonos.h            # Sonos API declarations
├── sonos.cpp          # Sonos UPnP/SOAP implementation
├── bom.md             # Bill of materials
├── hull/              # 3D-printable enclosure (STL files)
│   ├── TopHull_v1.stl
│   ├── BottomHull_v0.stl
│   └── knob_v0.stl
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
static const char* WIFI_SSID = "YourNetwork";
static const char* WIFI_PASSWORD = "YourPassword";

// Sonos
static const char* SONOS_IP = "192.168.x.x";  // Your Sonos speaker IP
static const int SONOS_PORT = 1400;
static const int VOLUME_STEP = 1;

// Hardware
static const int BUTTON_A_PIN = 35;
static const int BUTTON_B_PIN = 0;
static const int ENCODER_CLK_PIN = 25;
static const int ENCODER_DT_PIN = 26;
static const int ENCODER_SW_PIN = 27;

// Display
static const uint8_t DISPLAY_ROTATION = 1;
static const bool DISPLAY_INVERT = true;

// UI Colors (RGB565)
static const uint16_t COLOR_TRI_FILL  = 0x07E0;  // bright green
static const uint16_t COLOR_ICON_ON   = 0xFFFF;  // white
static const uint16_t COLOR_ICON_OFF  = 0x4208;  // dark gray

// Triangle geometry
static const int TRI_HEIGHT   = 100;
static const int TRI_BASE     = 116;
static const int TRI_OFFSET_Y = 5;

// Status icon layout (top-right)
static const int ICON_SIZE   = 10;
static const int ICON_GAP    = 4;
static const int ICON_MARGIN = 6;

// Timing
static const uint32_t DEBOUNCE_MS = 35;
static const uint32_t LONG_PRESS_MS = 700;
static const uint32_t REPEAT_INTERVAL_MS = 300;
```

Note: `config.h` contains credentials and is excluded from version control.

## Usage

- **Encoder rotation**: Volume up/down
- **Encoder button**: Play/pause toggle
- **Button A (GPIO35)**: Volume up (fallback)
- **Button B (GPIO0)**: Volume down (fallback)
- Hold built-in buttons for auto-repeat

Single-button mode activates automatically if only one built-in button is detected.

## Display

The screen shows a triangle that fills proportionally to the current volume (0-100), with the numeric value overlaid. Status icons in the top-right corner indicate:

- **WiFi**: filled circle (green = connected, gray = disconnected)
- **Sonos**: "S" letter (green = reachable, gray = unreachable)
- **Playback**: play/pause icon reflecting current transport state

## Enclosure

3D-printable STL files are in [hull/](hull/). Print the top and bottom hull halves plus the encoder knob. See [bom.md](bom.md) for parts sourcing.
