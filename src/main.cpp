#include "main.hpp"

#include <imgui.h>

#include <algorithm>
#include <iostream>

#include "atlas.hpp"
#include "icon_tiny.png.h"
#include "logic.hpp"
#include "rl.hpp"
#include "scene.hpp"
#include "serializer.hpp"
#include "ssaa_window.hpp"
#include "transform.hpp"

static const rl::Color bg = Color{40, 48, 65, 255};

int main() {
  // c++'s rand library is way overengineered for this
  SetRandomSeed(time(0));

  auto window = SSAAWindow{1280, 720, 2.0f, "InfiniSweeper"};

  // make the disgusting white flash disappear as soon as possible
  BeginDrawing();
  ClearBackground(BLACK);
  EndDrawing();

  // icon and screen size
  {
#ifndef __APPLE__
    auto icon_image = rl::Image{".png", icon_tiny_png, sizeof(icon_tiny_png)};
    icon_image.Format(PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    SetWindowIcon(icon_image);
#endif
    int display = GetCurrentMonitor();
    int width = GetMonitorWidth(display);
    int height = GetMonitorHeight(display);
    SetWindowSize(width / 3 * 2, height / 3 * 2);
    SetWindowPosition(width / 6, height / 6);
  }

  // loading screen
  {
    BeginDrawing();
    ClearBackground(BLACK);
    auto loading = rl::Texture{"res/loading.png"};
    int size = std::min(GetScreenWidth(), GetScreenHeight()) / loading.width;
    if (size < 0) size = 1;
    size *= loading.width;
    auto src = rl::Rect(0, 0, loading.width, loading.height);
    auto desc = rl::Rect((GetScreenWidth() - size) / 2,
                         (GetScreenHeight() - size) / 2,
                         size,
                         size);
    loading.Draw(src, desc);
    EndDrawing();
  }

  AtlasManager atlas;
  Scene scene;

  while (!window.ShouldClose() && !quit) {
    CheckFullscreen();
    window.CheckResize();
    CoordTransform::UpdateCamera();
    scene.Tick(window);

    // draw
    window.BeginDrawing();
    ClearBackground(bg);
    scene.Draw(atlas);

    // imgui
    window.BeginImGui();
#ifndef NDEBUG
    ImGuiDebugUI();
#endif
    window.EndDrawing();
  }
  return 0;
}

void ImGuiDebugUI() {
  static bool debug_window = false;
  if (IsKeyPressed(KEY_GRAVE)) debug_window = !debug_window;
  if (debug_window) {
    DrawFPS(10, 10);

    ImGui::SetNextWindowSize(ImVec2(0, 0));
    ImGui::Begin("Debug", &debug_window);

    ImGui::Text("Globals:");
    ImGui::Separator();  //------------------------
    ImGui::Text("Canvas Size: %4.0f × %4.0f", canvas_size.x, canvas_size.y);
    ImGui::Text("SSAA Scale: %.2f", ssaa_scale);
    ImGui::Text("Window Size: %4.0f × %4.0f", window_size.x, window_size.y);
    ImGui::Text("Inverse Aspect Ratio: %.4f", inverse_aspect_ratio);
    ImGui::Separator();  //------------------------
    ImGui::Text("Camera:");
    ImGui::Text("Pos: %.4f × %.4f\nZoom: %.4f",
                camera_coord.x,
                camera_coord.y,
                camera_zoom);
    ImGui::Text("Camera World Rect:\nX: %.4f Y: %.4f\nW: %.4f H: %.4f",
                camera_world_rect.x,
                camera_world_rect.y,
                camera_world_rect.width,
                camera_world_rect.height);
    ImGui::Separator();  //------------------------
    auto mouse_world =
        CoordTransform::PixelToWorld(rl::Mouse::GetPosition() * ssaa_scale);
    ImGui::Text("Mouse World Pos:\n %.4f × %.4f", mouse_world.x, mouse_world.y);
    ImGui::Text("Scroll Wheel: %.2f", GetMouseWheelMove());
    ImGui::End();
  }
}

void CheckFullscreen() {
  static bool fullscreen = false;
  static rl::Vector2 position;
  static int width;
  static int height;
  if ((IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) &&
      IsKeyPressed(KEY_ENTER)) {
    if (IsWindowFullscreen()) {
      ToggleFullscreen();
      ClearWindowState(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST);
      SetWindowPosition(position.x, position.y);
      SetWindowSize(width, height);
    } else {
      position = GetWindowPosition();
      width = GetScreenWidth();
      height = GetScreenHeight();
      SetWindowState(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST);
      int display = GetCurrentMonitor();
      SetWindowSize(GetMonitorWidth(display), GetMonitorHeight(display));
      ToggleFullscreen();
    }
  }
}
