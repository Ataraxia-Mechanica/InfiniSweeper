#include "ssaa_window.hpp"

#include <iostream>

#include "imgui.h"
#include "rlImGui.h"

rl::Vector2 canvas_size;
float ssaa_scale;
rl::Vector2 window_size;
float inverse_aspect_ratio;
bool resized;

SSAAWindow::SSAAWindow(u32 x, u32 y, float scale, std::string title) {
  window.Init(x, y, title);
  window.SetState(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
  // just in case vsync was forcefully disabled, don't want to cook CPU
  SetTargetFPS(240);

  SetExitKey(KEY_NULL);
  SetGlobals(x, y, scale);
  InitRenderTexture();
  this->scale = scale;
  target_scale = scale;

  rlImGuiSetup(true);  // incorporate rlimgui state change into SSAAWindow, so
                       // no need to include rlimgui elsewhere
  ImGui::GetIO().IniFilename = NULL;
}

void SSAAWindow::SetScale(float scale) {
  target_scale = scale;
}

void SSAAWindow::CheckResize() {
  if (IsWindowResized() || target_scale != scale) {
    resized = true;
    Resize();
  } else {
    resized = false;
  }
}

void SSAAWindow::BeginDrawing() {
  ssaa.BeginMode();
}

void SSAAWindow::BeginImGui() {
  ssaa.EndMode();
  window.BeginDrawing();
  ((rl::Texture&)ssaa.texture)
      .Draw(rl::Rectangle(0, 0, canvas_size.x, -canvas_size.y),
            rl::Rectangle(0, 0, window.GetWidth(), window.GetHeight()));
  // DrawTexturePro(ssaa.texture,
  //                rl::Rectangle(0, 0, canvas_size.x, -canvas_size.y),
  //                rl::Rectangle(0, 0, window.GetWidth(), window.GetHeight()),
  //                rl::Vector2(0, 0),
  //                0.0f,
  //                Color{255, 255, 255, 255});
  rlImGuiBegin();
}

void SSAAWindow::EndDrawing() {
  rlImGuiEnd();
  window.EndDrawing();
}

void SSAAWindow::Resize() {
  SetGlobals(window.GetWidth(), window.GetHeight(), target_scale);
  InitRenderTexture();
  scale = target_scale;
}

void SSAAWindow::SetGlobals(u32 x, u32 y, float scale) {
  window_size = rl::Vector2(x, y);
  ssaa_scale = scale;
  canvas_size = rl::Vector2((u32)(x * scale), (u32)(y * scale));
  inverse_aspect_ratio = canvas_size.y / canvas_size.x;
}

void SSAAWindow::InitRenderTexture() {
  ssaa = rl::RenderTexture2D(canvas_size.x, canvas_size.y);
  SetTextureFilter(ssaa.texture, TEXTURE_FILTER_BILINEAR);
  SetTextureWrap(ssaa.texture, TEXTURE_WRAP_CLAMP);
}
