#include "FrontlightController.h"

#include "CrossPointSettings.h"

FrontlightController frontlightController;

void FrontlightController::begin() {
  manager.begin();
  apply();
}

void FrontlightController::apply() {
  if (!manager.present()) return;
  manager.setColorTemperature(SETTINGS.frontlightWarmth);
  manager.setBrightnessLevel(SETTINGS.frontlightEnabled ? SETTINGS.frontlightBrightness : 0);
}

void FrontlightController::turnOffTemporarily() { manager.setBrightnessLevel(0); }
