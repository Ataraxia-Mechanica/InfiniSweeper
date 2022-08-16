#include "atlas.hpp"

#include <algorithm>
#include <string>

#include "transform.hpp"

AtlasManager::AtlasManager() {
  tile = rl::Texture2D("res/tile.png");
  tile.GenMipmaps();
  tile.SetFilter(TEXTURE_FILTER_TRILINEAR);

  for (u32 i = 0; i < 2; i++) {
    number[i] = rl::Texture2D("res/number_" + std::to_string(i) + ".png");
    number[i].GenMipmaps();
    number[i].SetFilter(TEXTURE_FILTER_TRILINEAR);
  }

  tile_resolution = tile.GetWidth() / tile_x_max;

  ui = rl::Texture2D("res/ui.png");
  ui.GenMipmaps();
  ui.SetFilter(TEXTURE_FILTER_TRILINEAR);

  ui_resolution = ui.GetWidth() / ui_x_max;

  logo = rl::Texture2D("res/logo.png");
  logo.GenMipmaps();
  logo.SetFilter(TEXTURE_FILTER_TRILINEAR);
}

void AtlasManager::Draw(u32 mine_num, rl::Rect rect) {
  mine_num = std::min(mine_num, (u32)31);
  u32 rect_index = mine_num % 16;
  u32 texture_index = mine_num / 16;
  rl::Rect texture_rect =
      RectFromIndex(rect_index, number_x_max, tile_resolution);
  number[texture_index].Draw(texture_rect, rect);
}

void AtlasManager::Draw(Tile type, rl::Rect rect) {
  rl::Rect texture_rect = RectFromIndex((u32)type, tile_x_max, tile_resolution);
  tile.Draw(texture_rect, rect);
}

void AtlasManager::DrawUI(char character,
                          rl::Rect screen_rect,
                          rl::Color tint) {
  rl::Rect texture_rect = RectFromChar(character, ui_x_max, ui_resolution);
  auto rect = CoordTransform::ScreenToPixel(screen_rect);
  ui.Draw(texture_rect, rect, rl::Vector2{0, 0}, 0, tint);
}

void AtlasManager::DrawUI(UI index, rl::Rect screen_rect, rl::Color tint) {
  rl::Rect texture_rect = RectFromIndex((u32)index, ui_x_max, ui_resolution);
  auto rect = CoordTransform::ScreenToPixel(screen_rect);
  ui.Draw(texture_rect, rect, rl::Vector2{0, 0}, 0, tint);
}

void AtlasManager::DrawLogo(rl::Rect screen_rect) {
  auto rect = CoordTransform::ScreenToPixel(screen_rect);
  logo.Draw(rl::Rect(0, 0, logo.width, logo.height), rect);
}

rl::Rect AtlasManager::RectFromIndex(u32 index, u32 x_max, u32 res) {
  u32 x = index % x_max;
  u32 y = index / x_max;
  return rl::Rect(x * res, y * res, res, res);
}

rl::Rect AtlasManager::RectFromChar(char c, u32 x_max, u32 res) {
  u32 x_max_doubled = x_max * 2;
  u32 res_halved = res / 2;
  u32 index = (c == '-') ? 10 : c - '0';

  u32 x = index % x_max_doubled;
  u32 y = index / x_max_doubled;
  return rl::Rect(x * res_halved, y * res, res_halved, res);
}
