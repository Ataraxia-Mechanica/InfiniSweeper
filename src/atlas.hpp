#pragma once

#include "fixed_size_int.hpp"
#include "rl.hpp"

enum class Tile {
  covered = 0,
  covered_press = 1,
  covered_highlight = 2,
  covered_highlight_press = 3,
  question = 4,
  question_press = 5,
  question_highlight = 6,
  question_highlight_press = 7,
  flagged = 8,
  //
  flagged_highlight = 10,
  //
  mine = 12,
  no_mine = 13,
  detonated = 14,
};

enum class UI {
  none = 0,
  menu = 6,
  play,
  previous,
  restart,
  next,
  select,
  low,
  mid,
  high,
  quit,
  level,
  time,
  mine,
  close,
};

class AtlasManager {
 public:
  AtlasManager();
  void Draw(u32 mine_num, rl::Rect rect);
  void Draw(Tile type, rl::Rect rect);
  void DrawUI(char character, rl::Rect screen_rect, rl::Color tint);
  void DrawUI(UI index, rl::Rect screen_rect, rl::Color tint);
  void DrawLogo(rl::Rect screen_rect);

 private:
  rl::Rect RectFromIndex(u32 index, u32 x_max, u32 res);
  rl::Rect RectFromChar(char c, u32 x_max, u32 res);
  rl::Texture2D number[2];
  rl::Texture2D tile;
  rl::Texture2D ui;
  rl::Texture2D logo;
  const u32 number_x_max = 4;
  const u32 tile_x_max = 4;
  const u32 ui_x_max = 8;
  u32 tile_resolution;
  u32 ui_resolution;
};
