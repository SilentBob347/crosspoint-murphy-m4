#pragma once

#include "activities/Activity.h"
#include "util/ButtonNavigator.h"

class FrontlightActivity final : public Activity {
 public:
  FrontlightActivity(GfxRenderer& renderer, MappedInputManager& input) : Activity("Frontlight", renderer, input) {}
  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
  bool isFrontlightActivity() const override { return true; }

 private:
  enum class Field { Brightness, Warmth } selected = Field::Brightness;
  bool dragging = false;
  bool changed = false;
  ButtonNavigator buttonNavigator;
  void adjust(int delta);
  bool updateFromTouch(int x, int y);
};
