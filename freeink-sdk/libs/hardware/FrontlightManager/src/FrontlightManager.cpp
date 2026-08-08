#include "FrontlightManager.h"

#if FREEINK_CAP_FRONTLIGHT
namespace {
constexpr uint32_t maxDuty(uint8_t bits) { return (1u << bits) - 1u; }

// Fixed LEDC channels for the Arduino-ESP32 2.x path (3.x keys by GPIO and allocates
// channels itself). Frontlight owns 0 (cool/primary) and 1 (warm); no other SDK LEDC
// user on a frontlight board takes these (the Buzzer uses the 3.x gpio-keyed API).
constexpr uint8_t LEDC_CH_COOL = 0;
constexpr uint8_t LEDC_CH_WARM = 1;

// Turn a 0-100 percentage into a duty, honoring active level. `pct` is pre-clamped.
uint32_t dutyFor(uint32_t pct, uint32_t full, bool activeHigh) {
  uint32_t duty = (pct * full) / 100u;
  return activeHigh ? duty : full - duty;
}

#if defined(ARDUINO) && ESP_ARDUINO_VERSION_MAJOR >= 3
void attachChannel(int8_t gpio, uint8_t /*ch*/, uint32_t freq, uint8_t bits) { ledcAttach(gpio, freq, bits); }
void writeChannel(int8_t gpio, uint8_t /*ch*/, uint32_t duty) { ledcWrite(gpio, duty); }
#else
void attachChannel(int8_t gpio, uint8_t ch, uint32_t freq, uint8_t bits) {
  ledcSetup(ch, freq, bits);
  ledcAttachPin(gpio, ch);
}
void writeChannel(int8_t /*gpio*/, uint8_t ch, uint32_t duty) { ledcWrite(ch, duty); }
#endif
}  // namespace
#endif

void FrontlightManager::begin() {
#if FREEINK_CAP_FRONTLIGHT
  const auto& fl = BoardConfig::ACTIVE.frontlight;
  if (fl.gpio == BoardConfig::PIN_UNASSIGNED) return;

  attachChannel(fl.gpio, LEDC_CH_COOL, fl.pwmFrequency, fl.pwmResolutionBits);
  if (fl.gpioWarm != BoardConfig::PIN_UNASSIGNED) {
    attachChannel(fl.gpioWarm, LEDC_CH_WARM, fl.pwmFrequency, fl.pwmResolutionBits);
  }
  _begun = true;
  setBrightness(0);
#endif
}

#if FREEINK_CAP_FRONTLIGHT
void FrontlightManager::apply() {
  const auto& fl = BoardConfig::ACTIVE.frontlight;
  if (!_begun || fl.gpio == BoardConfig::PIN_UNASSIGNED) return;

  const uint32_t full = maxDuty(fl.pwmResolutionBits);
  const bool dual = fl.gpioWarm != BoardConfig::PIN_UNASSIGNED;

  // Single channel: the primary carries the whole brightness. Warm/cool pair: split the
  // total brightness between cool and warm by the color-temperature mix, so overall
  // brightness stays ~constant as the color shifts (warm 0 = all cool, 100 = all warm).
  uint32_t totalDuty = 0;
  if (_useLevel && _brightnessLevel > 0) {
    const uint32_t n = static_cast<uint32_t>(_brightnessLevel - 1u);
    totalDuty = 1u + (n * n * (full - 1u)) / (254u * 254u);
  } else if (!_useLevel) {
    totalDuty = (static_cast<uint32_t>(_brightness) * full) / 100u;
  }
  // Round one channel and derive the other from the total. This guarantees that
  // even totalDuty=1 lights exactly one channel instead of truncating both to 0.
  const uint32_t warmDuty = dual ? (totalDuty * _warmPercent + 50u) / 100u : 0u;
  const uint32_t coolDuty = dual ? totalDuty - warmDuty : totalDuty;
  writeChannel(fl.gpio, LEDC_CH_COOL, fl.activeHigh ? coolDuty : full - coolDuty);

  if (dual) {
    writeChannel(fl.gpioWarm, LEDC_CH_WARM, fl.activeHigh ? warmDuty : full - warmDuty);
  }
}
#endif

void FrontlightManager::setBrightness(uint8_t percent) {
#if FREEINK_CAP_FRONTLIGHT
  if (percent > 100) percent = 100;
  _brightness = percent;
  _brightnessLevel = (static_cast<uint16_t>(percent) * 255u) / 100u;
  _useLevel = false;
  if (percent > 0) _lastBrightness = percent;
  apply();
#else
  (void)percent;
#endif
}

void FrontlightManager::setBrightnessLevel(uint8_t level) {
#if FREEINK_CAP_FRONTLIGHT
  _brightnessLevel = level;
  _brightness = (static_cast<uint16_t>(level) * 100u) / 255u;
  _useLevel = true;
  apply();
#else
  (void)level;
#endif
}

void FrontlightManager::off() { setBrightness(0); }
void FrontlightManager::on() { setBrightness(_lastBrightness); }

void FrontlightManager::setColorTemperature(uint8_t warmPercent) {
#if FREEINK_CAP_FRONTLIGHT
  _warmPercent = warmPercent > 100 ? 100 : warmPercent;
  // Only re-drives hardware when a warm channel exists; on single-channel boards this just
  // records the request (apply() ignores _warmPercent without a second channel).
  apply();
#else
  (void)warmPercent;
#endif
}
