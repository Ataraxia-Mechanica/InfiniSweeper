#include "transform.hpp"

#include <algorithm>

#include "ssaa_window.hpp"
#include "scene.hpp"

rl::Vector2 camera_coord = rl::Vector2(0, 0);
float camera_zoom = 0.1f;
rl::Rect camera_world_rect;
bool camera_moved = false;

const float zoom_max = 256.0f;
const float zoom_min = 0.005f;

namespace CoordTransform {

void UpdateCamera() {
  rl::Vector2 camera_coord_prev = camera_coord;
  float camera_zoom_prev = camera_zoom;
  UpdateCameraKeyboard();
  UpdateCameraMouse();
  camera_zoom = std::clamp(camera_zoom, zoom_min, zoom_max);
  UpdateCameraWorldRect();
  camera_moved =
      camera_coord_prev != camera_coord || camera_zoom_prev != camera_zoom;
}

void UpdateCameraKeyboard() {
  static rl::Vector2 cam_move = {0, 0};
  const float camera_move_speed = 0.33333333f;
  const float camera_zoom_speed = 0.4f;
  const float elasticity = 0.75f;

  float mul_this_frame = 1 - (1 - elasticity) * GetFrameTime() * 60.0f;
  float delta = GetFrameTime() * GetFrameTime() * 60.0f;

  if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) cam_move.y -= delta;
  if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) cam_move.x -= delta;
  if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) cam_move.y += delta;
  if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) cam_move.x += delta;
  camera_coord += cam_move / camera_zoom * camera_move_speed;
  cam_move *= mul_this_frame;

  static float cam_zoom_delta = 0;

  // yes I know they are mouse buttons, but putting this here would make things
  // simpler
  if (IsKeyDown(KEY_LEFT_SHIFT) || IsMouseButtonDown(MOUSE_BUTTON_EXTRA))
    cam_zoom_delta -= delta;
  if (IsKeyDown(KEY_LEFT_CONTROL) || IsMouseButtonDown(MOUSE_BUTTON_SIDE))
    cam_zoom_delta += delta;
  camera_zoom /= 1.0f + cam_zoom_delta * camera_zoom_speed;
  cam_zoom_delta *= mul_this_frame;
}

void UpdateCameraMouse() {
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) ||
      IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) ||
      IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
    rl::Vector2 mouse_global_delta =
        (rl::Vector2)GetMouseDelta() / window_size.x / camera_zoom;
    camera_coord -= mouse_global_delta;
  }

  // simply multiplying a number every frame wouldn't work for mouse wheels,
  // time for a buffer
  const float wheel_zoom_speed = 125.0f;
  const float wheel_elasticity = 0.85f;
  float mul_this_frame = 1 - (1 - wheel_elasticity) * GetFrameTime() * 60.0f;

  //0.85^96 ~= 0.00000017, good enough to hold all data
  const int buf_size = 96;
  static float zoom[buf_size] = {};
  for (int i = 0; i < buf_size - 1; i++) {
    zoom[i] = zoom[i + 1] * mul_this_frame;
  }

  // positive is zoom in
  zoom[buf_size - 1] = GetMouseWheelMove();
  float zoom_avg = 0;
  for (int i = 0; i < buf_size; i++) {
    zoom_avg += zoom[i];
  }
  zoom_avg /= buf_size;

  // no going over the limit
  if (zoom_avg < 0 && camera_zoom <= zoom_min) return;
  if (zoom_avg > 0 && camera_zoom >= zoom_max) return;

  float factor = 1.0f + zoom_avg * GetFrameTime() * wheel_zoom_speed;
  rl::Vector2 mouse_pos = CoordTransform::PixelToWorld(
      (rl::Vector2)GetMousePosition() * ssaa_scale);

  camera_coord = mouse_pos - (mouse_pos - camera_coord) / factor;
  camera_zoom *= factor;
}

void UpdateCameraWorldRect() {
  camera_world_rect = ScreenToWorld(rl::Rect(0, 0, 1, inverse_aspect_ratio));
}

// Vec2
rl::Vector2 WorldToPixel(rl::Vector2 world_coord) {
  return ScreenToPixel(WorldToScreen(world_coord));
}
rl::Vector2 PixelToWorld(rl::Vector2 pixel_coord) {
  return ScreenToWorld(PixelToScreen(pixel_coord));
}

rl::Vector2 PixelToScreen(rl::Vector2 pixel_coord) {
  return pixel_coord / canvas_size.x;
}
rl::Vector2 ScreenToPixel(rl::Vector2 screen_coord) {
  return screen_coord * canvas_size.x;
}

rl::Vector2 WorldToScreen(rl::Vector2 world_coord) {
  world_coord -= camera_coord;
  world_coord *= camera_zoom;
  world_coord += rl::Vector2(0.5f, 0.5f * inverse_aspect_ratio);  // center it
  return world_coord;
}
rl::Vector2 ScreenToWorld(rl::Vector2 screen_coord) {
  screen_coord -= rl::Vector2(0.5f, 0.5f * inverse_aspect_ratio);
  screen_coord /= camera_zoom;
  screen_coord += camera_coord;
  return screen_coord;
}

// Rect
rl::Rect WorldToPixel(rl::Rect world) {
  return ScreenToPixel(WorldToScreen(world));
}
rl::Rect PixelToWorld(rl::Rect pixel) {
  return ScreenToWorld(PixelToScreen(pixel));
}

rl::Rect PixelToScreen(rl::Rect pixel) {
  return rl::Rect(pixel.x / canvas_size.x,
                  pixel.y / canvas_size.x,
                  pixel.width / canvas_size.x,
                  pixel.height / canvas_size.x);
}
rl::Rect ScreenToPixel(rl::Rect screen) {
  return rl::Rect(screen.x * canvas_size.x,
                  screen.y * canvas_size.x,
                  screen.width * canvas_size.x,
                  screen.height * canvas_size.x);
}

rl::Rect WorldToScreen(rl::Rect world) {
  world.x -= camera_coord.x;
  world.y -= camera_coord.y;
  world.x *= camera_zoom;
  world.y *= camera_zoom;
  world.width *= camera_zoom;
  world.height *= camera_zoom;
  world.x += 0.5f;
  world.y += 0.5f * inverse_aspect_ratio;
  return world;
}
rl::Rect ScreenToWorld(rl::Rect screen) {
  screen.x -= 0.5f;
  screen.y -= 0.5f * inverse_aspect_ratio;
  screen.x /= camera_zoom;
  screen.y /= camera_zoom;
  screen.width /= camera_zoom;
  screen.height /= camera_zoom;
  screen.x += camera_coord.x;
  screen.y += camera_coord.y;

  return screen;
}
};  // namespace CoordTransform
