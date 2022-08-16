#include "rl.hpp"

namespace RectUtil {
bool is_inside(rl::Rect outer, rl::Rect inner);
rl::Rect boolean_and(rl::Rect a, rl::Rect b);
rl::Rect Square(float x, float y, float size);
rl::Vector2 Lerp(rl::Rect rect, float percx, float percy);
rl::Rect Fit(float aspect, rl::Rect bound);
}  // namespace RectUtil
