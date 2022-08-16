#pragma once

#include "fixed_size_int.hpp"
#include "rl.hpp"

extern rl::Vector2 canvas_size;
extern float ssaa_scale;
extern rl::Vector2 window_size;
extern float inverse_aspect_ratio;
extern bool resized;

class SSAAWindow {
 public:
  SSAAWindow(u32 x, u32 y, float scale, std::string title);

  // Superresolution Scale, only actually changes at BeginDrawing
  void SetScale(float scale);

  // call this at the start of a loop, only does lazy updating
  void CheckResize();

  // this one second
  void BeginDrawing();

  // third
  void BeginImGui();

  // lastly this
  void EndDrawing();

  inline bool ShouldClose() { return window.ShouldClose(); };

  float scale;

 private:
  void InitRenderTexture();  // Requires canvas_size to be set
  void Resize();
  void SetGlobals(u32 x, u32 y, float scale);

  rl::Window window;
  rl::RenderTexture2D ssaa;
  float target_scale;
};
