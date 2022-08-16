#pragma once
#include "fixed_size_int.hpp"

struct Vec2i {
  i32 x;
  i32 y;
  Vec2i operator+(Vec2i& other);
  Vec2i operator-(Vec2i& other);
  bool operator=(Vec2i& other);
};
