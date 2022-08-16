#include "scene.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>

#include "rect_util.hpp"
#include "serializer.hpp"
#include "transform.hpp"

using std::optional;

// serializer changes this appropriately when loading level selection screen
int max_levels;

bool quit = false;
bool mouse_on_ui = false;

float Quadratic(float num, float edge0, float edge1) {
  // Scale, bias and saturate num to 0..1 range
  num = std::clamp((num - edge0) / (edge1 - edge0), 0.0f, 1.0f);
  // Square
  return num * num;
}

float BounceBack(float num, float up, float down, float duration) {
  float up2 = up + duration;
  float down2 = down + duration;
  if (num > up2) {
    return 1.0f - Quadratic(num, down, down2);
  } else {
    return Quadratic(num, up, up2);
  }
}

void clamped_to_char(char* in, char digits, int number) {
  int up = std::pow(10, digits) - 1;
  int down = 1 - std::pow(10, digits - 1);
  number = std::clamp(number, down, up);

  char format[6];
  snprintf(format, 6, "%%%dd", digits);

  memset(in, 0, digits + 1);
  snprintf(in, digits + 1, format, number);
}

Scene::Scene() {
  uis.reserve(32);

  auto save = std::ifstream{"save"};
  if (!save.is_open()) {
    completed_levels = 0;
  } else {
    save >> completed_levels;
  }

  Serializer::Load("mainmenu", level, completed_levels);
}

void Scene::Tick(SSAAWindow& window) {
  toolbar.Tick();
  level_clear.Tick();

  auto state_prev = level.state;

  level.Tick();

  if (state_prev != State::won && level.state == State::won) toolbar.On();

  if (level.state == State::won) {
    int level_num = atoi(level.name.c_str());  // 0 on fail
    if (level_num > completed_levels) {
      completed_levels = level_num;

      std::ofstream save;
      save.open("save", std::ofstream::out | std::ofstream::trunc);
      save << level_num;
    }
  }

  AssembleUI();

  GetUIHover();
  GetUIPressed();

  if (hover.has_value() && hover == pressed) {
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      pressed = {};

      auto ui = hover.value();
      switch (ui) {
        case UI::menu:
          toolbar.On();
          toolbar.Off();
          break;
        case UI::previous: {
          int name_prev = std::atoi(level.name.c_str()) - 1;
          if (name_prev > 0) Serializer::Load(std::to_string(name_prev), level);
          break;
        }
        case UI::restart: Serializer::Load(level.name, level); break;
        case UI::next: {
          int name_next = std::atoi(level.name.c_str()) + 1;
          if (name_next <= max_levels)
            Serializer::Load(std::to_string(name_next), level);
          break;
        }
        case UI::play: [[fallthrough]];
        case UI::select:
          Serializer::Load("levelselection", level, completed_levels);
          break;
        case UI::high: window.SetScale(1.0f); break;
        case UI::mid: window.SetScale(2.0f); break;
        case UI::low: window.SetScale(1.5f); break;
        case UI::quit: {
          if (level.name == "mainmenu")
            quit = true;
          else
            Serializer::Load("mainmenu", level);
          break;
        }
        default: break;
      }

      // trigger logic here
    }
  } else if (hover != pressed) {
    pressed = {};
  }
}

void Scene::Draw(AtlasManager& atlas) {
  level.Draw(atlas);
  DrawUI(atlas);
}

void Scene::AssembleUI() {
  using RectUtil::Square;

  uis.clear();
  if (level.name != "mainmenu") {
    float width = 0.07 + BounceBack(toolbar.t, 0, 1.5, 0.5) * 0.3;
    uis.push_back({UI::none, rl::Rect{0, 0, width, 0.07}, false, false});

    // menu
    uis.push_back({UI::menu, Square(0.01, 0.01, 0.05)});

    int num = std::atoi(level.name.c_str());
    // prev
    float y = -0.05 + BounceBack(toolbar.t, 10. / 30, 45. / 30, 5. / 30) * 0.06;
    bool enabled = num > 1;
    uis.push_back({UI::previous, Square(0.06, y, 0.05), enabled});

    // restart
    enabled = num > 0;
    y = -0.05 + BounceBack(toolbar.t, 13. / 30, 42. / 30, 5. / 30) * 0.06;
    uis.push_back({UI::restart, Square(0.11, y, 0.05), enabled});

    // next
    enabled = num < max_levels && num > 0 && num < completed_levels + 1;
    y = -0.05 + BounceBack(toolbar.t, 16. / 30, 39. / 30, 5. / 30) * 0.06;
    uis.push_back({UI::next, Square(0.16, y, 0.05), enabled});

    // select
    enabled = num > 0;
    y = -0.05 + BounceBack(toolbar.t, 19. / 30, 36. / 30, 5. / 30) * 0.06;
    uis.push_back({UI::select, Square(0.21, y, 0.05), enabled});

    UI quality;
    if (ssaa_scale == 1.0f) quality = UI::low;
    if (ssaa_scale == 1.5f) quality = UI::mid;
    if (ssaa_scale == 2.0f) quality = UI::high;
    y = -0.05 + BounceBack(toolbar.t, 22. / 30, 33. / 30, 5. / 30) * 0.06;
    uis.push_back({quality, Square(0.26, y, 0.05)});

    // quit
    y = -0.05 + BounceBack(toolbar.t, 25. / 30, 30. / 30, 5. / 30) * 0.06;
    uis.push_back({UI::quit, Square(0.31, y, 0.05)});

    if (num > 0) {
      // game stats
      uis.push_back({UI::none, rl::Rect{0.58, 0, 0.42, 0.07}, false, false});
      uis.push_back({UI::level, Square(0.59, 0.01, 0.05), true, false});
      clamped_to_char(numbers, 2, num);

      uis.push_back({UI::time, Square(0.715, 0.01, 0.05), true, false});
      clamped_to_char(&numbers[2], 3, level.time);

      uis.push_back({UI::mine, Square(0.865, 0.01, 0.05), true, false});
      clamped_to_char(&numbers[5], 3, level.mine_left);
    }
  } else {  // main menu
    uis.push_back({UI::none,
                   rl::Rect{0, 0, 1.0 / 3, inverse_aspect_ratio},
                   false,
                   false});

    auto rect = rl::Rect{0.01,
                         inverse_aspect_ratio * 0.75f,
                         1.0f / 9.0f - 0.02,
                         inverse_aspect_ratio * 0.25f};
    uis.push_back({UI::play, RectUtil::Fit(1, rect)});

    rect.x += 1.0f / 9.0f;
    UI quality;
    if (ssaa_scale == 1.0f) quality = UI::low;
    if (ssaa_scale == 1.5f) quality = UI::mid;
    if (ssaa_scale == 2.0f) quality = UI::high;
    uis.push_back({quality, RectUtil::Fit(1, rect)});

    rect.x += 1.0f / 9.0f;
    uis.push_back({UI::quit, RectUtil::Fit(1, rect)});
  }
}

void Scene::DrawUI(AtlasManager& atlas) {
  const rl::Color grey = {0, 0, 0, 100};
  const rl::Color transp = {127, 127, 127, 192};
  for (auto& ui : uis) {
    if (ui.index == UI::none)
      CoordTransform::ScreenToPixel(ui.rect).Draw(grey);
    else if (ui.enabled == false)
      atlas.DrawUI(ui.index, ui.rect, transp);
    else if (ui.clickable == false)
      atlas.DrawUI(ui.index, ui.rect, WHITE);
    else if (ui.index == hover && ui.index == pressed) {
      auto smaller_rect = rl::Rect{ui.rect.x + ui.rect.width * .05f,
                                   ui.rect.y + ui.rect.height * .05f,
                                   ui.rect.width * 0.9f,
                                   ui.rect.height * 0.9f};
      atlas.DrawUI(ui.index, smaller_rect, WHITE);
    } else if (ui.index == hover) {
      auto bigger_rect = rl::Rect{ui.rect.x - ui.rect.width * .05f,
                                  ui.rect.y - ui.rect.height * .05f,
                                  ui.rect.width * 1.1f,
                                  ui.rect.height * 1.1f};
      atlas.DrawUI(ui.index, bigger_rect, WHITE);
    } else
      atlas.DrawUI(ui.index, ui.rect, WHITE);
  }

  if (level.name == "mainmenu") {
    atlas.DrawLogo(RectUtil::Fit(
        1, {0.f, 0.f, 1.0f / 3.0f, inverse_aspect_ratio * 0.75f}));
  }

  if (std::atoi(level.name.c_str()) > 0) {
    const float loc[8] = {0.64, 0.665, 0.765, 0.79, 0.815, 0.915, 0.94, 0.965};
    for (int i = 0; i < 8; i++) {
      char& n = numbers[i];
      const float& x = loc[i];
      rl::Color c = (n == ' ') ? transp : (rl::Color)WHITE;
      if (n == ' ') n = '0';
      atlas.DrawUI(n, rl::Rect{x, 0.01, 0.025, 0.05}, c);
    }
  }
}

void Scene::GetUIHover() {
  mouse_on_ui = false;
  hover = {};

  for (auto& ui : uis) {
    rl::Vector2 mouse = (rl::Vector2)GetMousePosition() / window_size.x;
    if (ui.rect.CheckCollision(mouse)) {
      mouse_on_ui = true;
      if (ui.clickable && ui.enabled) {
        hover = ui.index;
      }
    }
  }
}

void Scene::GetUIPressed() {
  if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;

  for (auto& ui : uis) {
    rl::Vector2 mouse = (rl::Vector2)GetMousePosition() / window_size.x;
    if (ui.rect.CheckCollision(mouse)) {
      if (ui.clickable && ui.enabled) {
        pressed = ui.index;
        return;
      }
    }
  }
}

void Animation::Tick() {
  if (t == 1.0f || t == 0.0f) return;

  float future = t + GetFrameTime() / duration;
  if (t < 1.0f && future > 1.0f)
    t = 1.0f;
  else if (future > 2.0f)
    t = 0.0f;
  else
    t = future;
}

void Animation::On() {
  if (t == 0.0f) t += 0.0000001;
}

void Animation::Off() {
  if (t == 1.0f) t += 0.0000001;
}
