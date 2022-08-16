#pragma once

#include <optional>
#include <string>

#include "atlas.hpp"
#include "logic.hpp"
#include "rl.hpp"
#include "ssaa_window.hpp"

extern int max_levels;
extern bool mouse_on_ui;  // shouldn't have mouse button input when this is true
extern bool quit;

struct UIInfo {
  UI index;
  rl::Rect rect;
  bool enabled = true;    // for when prev/next level is disabled
  bool clickable = true;  // for timer etc
};

class Animation {
 public:
  // 0.0-2.0(use 1.0-2.0 for asymmetric animation effects)
  float t = 0.0f;
  float duration = 0.5f;
  void Tick();
  void On();
  void Off();
};

class Scene {
 public:
  Scene();
  void Tick(SSAAWindow& window);
  void Draw(AtlasManager& atlas);

 private:
  int completed_levels;
  Level level;
  Animation toolbar = {0.0f, 0.35f};
  Animation level_clear;

  std::optional<UI> hover;
  std::optional<UI> pressed;
  std::vector<UIInfo> uis;  // immediate mode, destroy every tick and rebuild
  char numbers[9];          //\0
  void AssembleUI();
  void DrawUI(AtlasManager& atlas);

  void GetUIHover();
  void GetUIPressed();
};
