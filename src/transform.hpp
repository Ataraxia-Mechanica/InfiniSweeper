#include "rl.hpp"

extern rl::Vector2 camera_coord;
extern float camera_zoom;
extern rl::Rect camera_world_rect;
extern bool camera_moved;
extern const float zoom_max;
extern const float zoom_min;

// pixel coordinates: 0 to canvas length
// screen coordinates: x: 0 to 1, y: 0 to inverse_aspect_ratio
// world coordinates: every tile in upmost grid has length 1
// just manually do all those transformations instead of using the one from
// raylib, what could go wrong?
namespace CoordTransform {
void UpdateCamera();
void UpdateCameraKeyboard();
void UpdateCameraMouse();
void UpdateCameraWorldRect();

rl::Vector2 WorldToPixel(rl::Vector2 world_coord);
rl::Vector2 PixelToWorld(rl::Vector2 pixel_coord);

rl::Vector2 PixelToScreen(rl::Vector2 pixel_coord);
rl::Vector2 ScreenToPixel(rl::Vector2 screen_coord);

rl::Vector2 WorldToScreen(rl::Vector2 world_coord);
rl::Vector2 ScreenToWorld(rl::Vector2 screen_coord);

rl::Rect WorldToPixel(rl::Rect world);
rl::Rect PixelToWorld(rl::Rect pixel);

rl::Rect PixelToScreen(rl::Rect pixel);
rl::Rect ScreenToPixel(rl::Rect screen);

rl::Rect WorldToScreen(rl::Rect world);
rl::Rect ScreenToWorld(rl::Rect screen);
};  // namespace CoordTransform
