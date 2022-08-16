#include "vec2i.hpp"

Vec2i Vec2i::operator+(Vec2i& other) {
  return Vec2i{x + other.x, y + other.y};
}

Vec2i Vec2i::operator-(Vec2i& other) {
  return Vec2i{x - other.x, y - other.y};
}

bool Vec2i::operator=(Vec2i& other) {
  return (x == other.x && y == other.y);
}
