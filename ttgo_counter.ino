#include <TFT_eSPI.h>
#include <WiFi.h>
#include "config.h"
#include "sonos.h"

namespace {

struct Button {
  int pin;
  bool active_low;
  bool enabled;
  bool raw_pressed;
  bool stable_pressed;
  uint32_t last_raw_change_ms;
  uint32_t pressed_since_ms;
};

enum class ButtonEventType { None, Press, Release };

struct ButtonEvent {
  ButtonEventType type;
  uint32_t held_ms;
};

TFT_eSPI tft = TFT_eSPI();
Button button_a{};
Button button_b{};

int current_volume = 0;
bool single_button_mode = false;
uint32_t last_repeat_a_ms = 0;
uint32_t last_repeat_b_ms = 0;

int majorityRead(int pin) {
  int highs = 0;
  for (int i = 0; i < 7; ++i) {
    highs += (digitalRead(pin) == HIGH) ? 1 : 0;
    delayMicroseconds(250);
  }
  return (highs >= 4) ? HIGH : LOW;
}

bool readPressed(const Button &button) {
  const int level = majorityRead(button.pin);
  return button.active_low ? (level == LOW) : (level == HIGH);
}

bool pinLooksStable(int pin) {
  int transitions = 0;
  int prev = majorityRead(pin);
  for (int i = 0; i < 120; ++i) {
    delay(2);
    const int now = majorityRead(pin);
    if (now != prev) {
      ++transitions;
      prev = now;
    }
  }
  return transitions <= 4;
}

Button makeButton(int pin, bool active_low) {
  Button b{};
  b.pin = pin;
  b.active_low = active_low;
  b.enabled = pinLooksStable(pin);
  b.raw_pressed = b.enabled ? readPressed(b) : false;
  b.stable_pressed = b.raw_pressed;
  b.last_raw_change_ms = millis();
  b.pressed_since_ms = 0;
  return b;
}

ButtonEvent updateButton(Button &b, uint32_t now_ms) {
  if (!b.enabled) {
    return {ButtonEventType::None, 0};
  }

  const bool raw_pressed = readPressed(b);
  if (raw_pressed != b.raw_pressed) {
    b.raw_pressed = raw_pressed;
    b.last_raw_change_ms = now_ms;
  }

  if ((now_ms - b.last_raw_change_ms) < DEBOUNCE_MS) {
    return {ButtonEventType::None, 0};
  }

  if (b.stable_pressed == b.raw_pressed) {
    return {ButtonEventType::None, 0};
  }

  b.stable_pressed = b.raw_pressed;
  if (b.stable_pressed) {
    b.pressed_since_ms = now_ms;
    return {ButtonEventType::Press, 0};
  }

  const uint32_t held = now_ms - b.pressed_since_ms;
  return {ButtonEventType::Release, held};
}

int pickFontToFit(const char *text, int max_width, int start_font, int min_font) {
  for (int font = start_font; font >= min_font; --font) {
    if (tft.textWidth(text, font) <= max_width) {
      return font;
    }
  }
  return min_font;
}

void drawVolume() {
  char vol_text[8];
  snprintf(vol_text, sizeof(vol_text), "%d", current_volume);

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  const int cx = tft.width() / 2;
  const int cy = tft.height() / 2;
  const int max_w = tft.width() - 12;

  const int font = pickFontToFit(vol_text, max_w, 8, 4);
  tft.drawString(vol_text, cx, cy, font);
}

void changeVolume(int delta) {
  int new_volume = current_volume + delta;
  if (new_volume < 0) new_volume = 0;
  if (new_volume > 100) new_volume = 100;
  if (new_volume == current_volume) return;

  if (sonosSetVolume(new_volume)) {
    current_volume = new_volume;
    drawVolume();
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(120);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // GPIO35 is input-only; GPIO0 usually has external pull-up.
  pinMode(BUTTON_A_PIN, INPUT);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  delay(20);

  button_a = makeButton(BUTTON_A_PIN, true);
  button_b = makeButton(BUTTON_B_PIN, true);

  // Some board variants expose only one usable GPIO button.
  single_button_mode = (button_a.enabled && !button_b.enabled);

  Serial.printf("button_a_enabled=%d button_b_enabled=%d single_mode=%d\n",
                button_a.enabled ? 1 : 0,
                button_b.enabled ? 1 : 0,
                single_button_mode ? 1 : 0);

  tft.init();
  tft.setRotation(DISPLAY_ROTATION);
  tft.invertDisplay(DISPLAY_INVERT);

  // Show connecting message
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("WiFi...", tft.width() / 2, tft.height() / 2, 4);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());

  // Get initial volume from Sonos
  tft.drawString("Sonos...", tft.width() / 2, tft.height() / 2, 4);
  current_volume = sonosGetVolume();
  if (current_volume < 0) current_volume = 0;

  drawVolume();
}

void loop() {
  const uint32_t now = millis();

  const ButtonEvent a_event = updateButton(button_a, now);
  const ButtonEvent b_event = updateButton(button_b, now);

  if (single_button_mode) {
    // Fallback mode: short press +, long press -.
    if (a_event.type == ButtonEventType::Release) {
      changeVolume((a_event.held_ms >= LONG_PRESS_MS) ? -VOLUME_STEP : +VOLUME_STEP);
    }
  } else {
    // Initial press
    if (a_event.type == ButtonEventType::Press) {
      changeVolume(+VOLUME_STEP);
      last_repeat_a_ms = now;
    }
    if (b_event.type == ButtonEventType::Press) {
      changeVolume(-VOLUME_STEP);
      last_repeat_b_ms = now;
    }
    // Auto-repeat while held
    if (button_a.stable_pressed && (now - last_repeat_a_ms) >= REPEAT_INTERVAL_MS) {
      changeVolume(+VOLUME_STEP);
      last_repeat_a_ms = now;
    }
    if (button_b.stable_pressed && (now - last_repeat_b_ms) >= REPEAT_INTERVAL_MS) {
      changeVolume(-VOLUME_STEP);
      last_repeat_b_ms = now;
    }
  }

  delay(4);
}
