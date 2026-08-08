#pragma once

#include <FrontlightManager.h>

class FrontlightController {
 public:
  void begin();
  void apply();
  void turnOffTemporarily();
  bool present() const { return manager.present(); }
  bool hasWarmth() const { return manager.hasColorTemperature(); }

 private:
  FrontlightManager manager;
};

extern FrontlightController frontlightController;
