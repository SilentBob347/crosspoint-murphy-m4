#include "FrontlightActivity.h"

#include <algorithm>

#include "CrossPointSettings.h"
#include "FrontlightController.h"
#include "components/UITheme.h"
#include "components/icons/frontlight_icons.h"
#include "fontIds.h"

namespace {
constexpr int BAR_TOP = 125;
constexpr int BAR_BOTTOM_MARGIN = 165;
constexpr int BAR_W = 104;
constexpr int TOUCH_PAD = 28;
constexpr int BRIGHTNESS_STEPS = 16;
constexpr int LIGHT_BUTTON_W = 88;
constexpr int LIGHT_BUTTON_H = 56;
constexpr int LIGHT_BUTTON_BOTTOM_MARGIN = 84;

uint8_t brightnessForStep(int step) {
  return static_cast<uint8_t>(step >= BRIGHTNESS_STEPS - 1 ? 255 : 1 + std::clamp(step, 0, BRIGHTNESS_STEPS - 1) * 17);
}

int brightnessStep(uint8_t value) {
  if (value <= 1) return 0;
  return std::clamp((static_cast<int>(value) - 1 + 8) / 17, 0, BRIGHTNESS_STEPS - 1);
}

void drawIconInverted(const GfxRenderer& renderer, const freeink::Icon& icon, const int x, const int y) {
  const int bytesPerRow = (icon.w + 7) / 8;
  for (int py = 0; py < icon.h; ++py) {
    for (int px = 0; px < icon.w; ++px) {
      const uint8_t packed = icon.bits[py * bytesPerRow + px / 8];
      if ((packed & (0x80u >> (px % 8))) == 0) renderer.drawPixel(x + px, y + py, false);
    }
  }
}
}  // namespace

void FrontlightActivity::onEnter() {
  Activity::onEnter();
  requestUpdate();
}

void FrontlightActivity::onExit() {
  if (changed) SETTINGS.saveToFile();
  Activity::onExit();
}

void FrontlightActivity::adjust(int delta) {
  uint8_t& value = selected == Field::Brightness ? SETTINGS.frontlightBrightness : SETTINGS.frontlightWarmth;
  const uint8_t next = selected == Field::Brightness
                           ? brightnessForStep(brightnessStep(value) + delta)
                           : static_cast<uint8_t>(std::clamp(static_cast<int>(value) + delta, 0, 100));
  if (next == value) return;
  value = next;
  if (selected == Field::Brightness && next > 0) SETTINGS.frontlightEnabled = 1;
  changed = true;
  frontlightController.apply();
  requestUpdate();
}

bool FrontlightActivity::updateFromTouch(int x, int y) {
  const int width = renderer.getScreenWidth();
  const int barBottom = renderer.getScreenHeight() - BAR_BOTTOM_MARGIN;
  const int barHeight = barBottom - BAR_TOP;
  const int brightnessX = width / 4 - BAR_W / 2;
  const int warmthX = width * 3 / 4 - BAR_W / 2;
  Field field;
  int maximum;
  if (x >= brightnessX - TOUCH_PAD && x <= brightnessX + BAR_W + TOUCH_PAD) {
    field = Field::Brightness;
    maximum = 255;
  } else if (frontlightController.hasWarmth() && x >= warmthX - TOUCH_PAD && x <= warmthX + BAR_W + TOUCH_PAD) {
    field = Field::Warmth;
    maximum = 100;
  } else {
    return false;
  }
  if (y < BAR_TOP - TOUCH_PAD || y > barBottom + TOUCH_PAD) return false;
  selected = field;
  uint8_t& value = field == Field::Brightness ? SETTINGS.frontlightBrightness : SETTINGS.frontlightWarmth;
  // Vertical controls read naturally like a thermometer: minimum at the bottom.
  const int position = barBottom - std::clamp(y, BAR_TOP, barBottom);
  value = field == Field::Brightness ? brightnessForStep(position * (BRIGHTNESS_STEPS - 1) / std::max(1, barHeight))
                                     : static_cast<uint8_t>(position * maximum / std::max(1, barHeight));
  if (field == Field::Brightness && value > 0) SETTINGS.frontlightEnabled = 1;
  changed = true;
  frontlightController.apply();
  return true;
}

void FrontlightActivity::loop() {
  int x = 0, y = 0;
  if (mappedInput.isScreenTouchHeld(x, y)) {
    if (updateFromTouch(x, y)) dragging = true;
    return;
  }
  if (dragging) {
    dragging = false;
    requestUpdate();
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) return finish();
  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    selected = selected == Field::Brightness ? Field::Warmth : Field::Brightness;
    requestUpdate();
    return;
  }
  if (mappedInput.wasScreenTapped(x, y)) {
    const int lightButtonX = (renderer.getScreenWidth() - LIGHT_BUTTON_W) / 2;
    const int lightButtonY = renderer.getScreenHeight() - LIGHT_BUTTON_BOTTOM_MARGIN - LIGHT_BUTTON_H;
    if (x >= lightButtonX && x < lightButtonX + LIGHT_BUTTON_W && y >= lightButtonY &&
        y < lightButtonY + LIGHT_BUTTON_H) {
      SETTINGS.frontlightEnabled = !SETTINGS.frontlightEnabled;
      changed = true;
      frontlightController.apply();
      requestUpdate();
      return;
    }
    if (y > renderer.getScreenHeight() - 90) return finish();
    if (updateFromTouch(x, y)) requestUpdate();
  }
  buttonNavigator.onPressAndContinuous({MappedInputManager::Button::Up}, [this] { adjust(1); });
  buttonNavigator.onPressAndContinuous({MappedInputManager::Button::Down}, [this] { adjust(-1); });
}

void FrontlightActivity::render(RenderLock&&) {
  renderer.clearScreen();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();
  const int barBottom = height - BAR_BOTTOM_MARGIN;
  const int barHeight = barBottom - BAR_TOP;
  auto slider = [&](const freeink::Icon& icon, int x, int value, int maximum, int blocks) {
    renderer.drawIcon(icon.bits, x + (BAR_W - icon.w) / 2, 52, icon.w);
    const int litBlocks = (value * blocks + maximum - 1) / maximum;
    for (int i = 0; i < blocks; ++i) {
      const int blockTop = barBottom - ((i + 1) * barHeight / blocks);
      const int blockBottom = barBottom - (i * barHeight / blocks);
      const int blockHeight = std::max(2, blockBottom - blockTop - 1);
      renderer.drawRect(x, blockTop, BAR_W, blockHeight);
      if (i < litBlocks) renderer.fillRect(x + 3, blockTop + 3, BAR_W - 6, std::max(1, blockHeight - 6));
    }
  };
  const int currentBrightnessStep = brightnessStep(SETTINGS.frontlightBrightness);
  slider(icon_brightness_64, width / 4 - BAR_W / 2, currentBrightnessStep + 1, BRIGHTNESS_STEPS, BRIGHTNESS_STEPS);
  if (frontlightController.hasWarmth())
    slider(icon_warmth_64, width * 3 / 4 - BAR_W / 2, SETTINGS.frontlightWarmth, 100, 16);
  const int lightButtonX = (width - LIGHT_BUTTON_W) / 2;
  const int lightButtonY = height - LIGHT_BUTTON_BOTTOM_MARGIN - LIGHT_BUTTON_H;
  const int powerIconX = lightButtonX + (LIGHT_BUTTON_W - icon_power_32.w) / 2;
  const int powerIconY = lightButtonY + (LIGHT_BUTTON_H - icon_power_32.h) / 2;
  if (SETTINGS.frontlightEnabled) {
    renderer.fillRect(lightButtonX, lightButtonY, LIGHT_BUTTON_W, LIGHT_BUTTON_H);
    drawIconInverted(renderer, icon_power_32, powerIconX, powerIconY);
  } else {
    renderer.drawRect(lightButtonX, lightButtonY, LIGHT_BUTTON_W, LIGHT_BUTTON_H);
    renderer.drawIcon(icon_power_32.bits, powerIconX, powerIconY, icon_power_32.w);
  }
  renderer.displayBuffer();
}
