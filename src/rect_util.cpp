#include "rect_util.hpp"

#include <algorithm>

bool RectUtil::is_inside(rl::Rect outer, rl::Rect inner) {
  if (outer.x <= inner.x && outer.y <= inner.y &&
      (outer.x + outer.width >= inner.x + inner.width) &&
      (outer.y + outer.height >= inner.y + inner.height))
    return true;
  return false;
}

rl::Rect RectUtil::boolean_and(rl::Rect a, rl::Rect b) {
  float x = std::max(a.x, b.x);
  float y = std::max(a.y, b.y);
  float w = std::min(a.width + a.x, b.width + b.x) - x;
  float h = std::min(a.height + a.y, b.height + b.y) - y;
  return rl::Rect(x, y, w, h);
}

rl::Rect RectUtil::Square(float x, float y, float size) {
  return rl::Rect{x, y, size, size};
}

rl::Vector2 RectUtil::Lerp(rl::Rect rect, float percx, float percy) {
  return rl::Vector2{rect.x + percx * rect.width, rect.y + percy * rect.height};
}

rl::Rect RectUtil::Fit(float aspect, rl::Rect bound) {
  float aspect_bound = bound.width / bound.height;
  if (aspect > aspect_bound) {  // wider, squeeze in y direction
    float height = bound.width / aspect;
    return {
        bound.x, bound.y + bound.height / 2 - height / 2, bound.width, height};
  } else {
    float width = bound.height * aspect;
    return {
        bound.x + bound.width / 2 - width / 2, bound.y, width, bound.height};
  }
}
