#include "logic.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "rect_util.hpp"
#include "scene.hpp"
#include "serializer.hpp"
#include "ssaa_window.hpp"
#include "transform.hpp"

// clang-format off
const Vec2i neighbor_deltas[8] = 
    { Vec2i{-1, -1}, Vec2i{ 0, -1}, Vec2i{ 1, -1},
      Vec2i{-1,  0},                Vec2i{ 1,  0},
      Vec2i{-1,  1}, Vec2i{ 0,  1}, Vec2i{ 1,  1},};
// clang-format on

// used with optional<Cell>&
static optional<Cell> empty = {};

bool Board::Inside(Vec2i pos) {
  return (pos.x >= 0 && pos.x < width) && (pos.y >= 0 && pos.y < height);
}

rl::Rect Board::GetCellRect(Vec2i pos, rl::Rect board_rect) {
  return rl::Rect{board_rect.x + (float)pos.x * board_rect.width / width,
                  board_rect.y + (float)pos.y * board_rect.height / height,
                  board_rect.width / width,
                  board_rect.height / height};
}

optional<Cell>& Board::Get(Vec2i pos) {
  if (!Inside(pos)) {
    return empty;
  }
  return cells[pos.y * width + pos.x];
}

optional<Cell>& Board::Get(i32 x, i32 y) {
  return Get(Vec2i{x, y});
}

optional<Cell>& Level::Get(CellID id) {
  return boards[id.board_index].Get(id.ToVec2i());
}

void Level::Tick() {
  // slowly zoom out main menu
  if (name == "mainmenu") {
    auto target = CoordTransform::ScreenToWorld(
        rl::Vector2{2.0f / 3.0f, 0.5f * inverse_aspect_ratio});
    float factor = GetFrameTime() * 0.1f + 1.0f;
    camera_coord = target - (target - camera_coord) * factor;
    camera_zoom /= factor;
    camera_moved = true;
    CoordTransform::UpdateCameraWorldRect();
  }

  if (started && state == State::gaming) {
    time += GetFrameTime();
  }

  if (camera_moved || resized) {
    ChangeRootBoard();
  }

  RemoveHighLight();

  if (name == "mainmenu") return;

  UpdateMouseOver();
  if (state == State::gaming) AddHighLight();

  HandleMouseInput();

  if (winning_check_needed) {
    CheckGameWon();
  }

  if (state != State::gaming && IsKeyPressed(KEY_R)) {
    Serializer::Load(name, *this);
  };
}

vector<pair<Portal&, BoardRectInfo>> Level::GetParentRectInfo(
    u32 index, bool non_clone_only) {
  return GetParentRectInfo(
      BoardRectInfo{
          index,
          rl::Rect{
              0, 0, (float)boards[index].width, (float)boards[index].height}},
      non_clone_only);
}

vector<pair<Portal&, BoardRectInfo>> Level::GetChildRectInfo(u32 index) {
  return GetChildRectInfo(BoardRectInfo{
      index,
      rl::Rect{0, 0, (float)boards[index].width, (float)boards[index].height}});
}

vector<pair<Portal&, BoardRectInfo>> Level::GetParentRectInfo(
    BoardRectInfo info, bool non_clone_only) {
  vector<pair<Portal&, BoardRectInfo>> vector = {};
  for (auto& portal : portals) {
    // second one always eval to true when non-clone-only is false
    if (portal.to == info.index && (portal.clone == false || !non_clone_only)) {
      Board& parent = boards[portal.from];

      float size = info.rect.width / portal.width;
      auto parent_rect = rl::Rect{
          info.rect.x - (float)portal.x * size,
          info.rect.y - (float)portal.y * size,
          (float)parent.width * size,
          (float)parent.height * size,
      };
      // clone info irrlevant here  --------------------------↓
      vector.push_back(
          {portal, BoardRectInfo{portal.from, parent_rect, false}});
    }
  }
  return vector;
}

vector<pair<Portal&, BoardRectInfo>> Level::GetChildRectInfo(
    BoardRectInfo info) {
  vector<pair<Portal&, BoardRectInfo>> vector = {};
  for (auto& portal : portals) {
    if (portal.from == info.index) {
      Board& parent = boards[portal.from];
      Board& child = boards[portal.to];

      float cell_size_parent = info.rect.width / parent.width;
      float portal_rect_width = info.rect.width * portal.width / parent.width;
      float portal_rect_height =
          info.rect.height * portal.height / parent.height;
      auto child_rect = rl::Rect{
          info.rect.x + (float)portal.x * cell_size_parent,
          info.rect.y + (float)portal.y * cell_size_parent,
          portal_rect_width,
          portal_rect_height,
      };
      vector.push_back(
          {portal, BoardRectInfo{portal.to, child_rect, portal.clone}});
    }
  }
  return vector;
}

void Level::ChangeRootBoard() {
  static constexpr u32 max_root_board_change = 4;

  for (int i = 0; i < max_root_board_change; i++) {
    if (!ChangeRootBoardOnce()) break;
  }
}

bool Level::ChangeRootBoardOnce() {
  auto root_rect = rl::Rect{
      0, 0, (float)boards[root_board].width, (float)boards[root_board].height};
  rl::Rect root_rect_clipped =
      RectUtil::boolean_and(camera_world_rect, root_rect);
  float root_rect_clipped_size =
      root_rect_clipped.width * root_rect_clipped.height;

  // go up one layer when not covering the full screen, AND clipped area
  // becomes bigger
  if (!RectUtil::is_inside(root_rect, camera_world_rect)) {
    // should only contain 0 or 1 element
    auto portal_AND_infos = GetParentRectInfo(root_board, true);

    // edge case: sometimes going up one layer the clipped area wouldn't become
    // bigger but more times would, see level 5
    static constexpr u32 max_go_up = 8;
    u32 go_up = 1;
    bool need_to_go_up = false;
    // when size < go_up, the uppmost board has been reached
    while (go_up <= max_go_up && portal_AND_infos.size() == go_up) {
      auto& portal_AND_info = portal_AND_infos.back();
      auto& portal = portal_AND_info.first;
      auto& rect_info = portal_AND_info.second;

      auto parent_rect_clipped =
          RectUtil::boolean_and(camera_world_rect, rect_info.rect);
      auto parent_rect_clipped_size =
          parent_rect_clipped.width * parent_rect_clipped.height;
      // size tolerance
      if (parent_rect_clipped_size > root_rect_clipped_size + 0.000001f) {
        need_to_go_up = true;
        break;
      }

      // test next layer
      auto parent_portal_AND_info = GetParentRectInfo(rect_info, true);
      // concatenate
      portal_AND_infos.insert(portal_AND_infos.end(),
                              parent_portal_AND_info.begin(),
                              parent_portal_AND_info.end());
      go_up++;
    }

    if (need_to_go_up) {
      float last_width = boards[root_board].width;

      for (auto& portal_AND_info : portal_AND_infos) {
        auto& portal = portal_AND_info.first;
        auto& rect_info = portal_AND_info.second;
        float scale = last_width / portal.width;
        last_width = boards[rect_info.index].width;
        camera_coord /= scale;
        camera_coord.x += portal.x;
        camera_coord.y += portal.y;
        camera_zoom *= scale;
        root_board = portal.from;
      }
      CoordTransform::UpdateCameraWorldRect();
      UpdateBoardRectCache();
      return true;
    }
  }
  // go down one layer when child covering full screen, OR clipped area doesn't
  // become smaller, AND wouldn't exceed zoom limit
  // don't you love edge cases
  auto portal_AND_infos = GetChildRectInfo(root_board);

  for (auto& portal_AND_info : portal_AND_infos) {
    auto& portal = portal_AND_info.first;
    auto& child_rect_info = portal_AND_info.second;

    // skip when it isn't in screen
    if (!child_rect_info.rect.CheckCollision(camera_world_rect)) continue;

    auto& child_rect = child_rect_info.rect;
    auto child_rect_clipped =
        RectUtil::boolean_and(camera_world_rect, child_rect);
    auto child_rect_clipped_size =
        child_rect_clipped.width * child_rect_clipped.height;

    // tolerance needs to be smaller than zooming out, otherwise would go into
    // oscillation
    if (RectUtil::is_inside(child_rect_info.rect, camera_world_rect) ||
        child_rect_clipped_size >= root_rect_clipped_size) {
      float scale = (float)(boards[child_rect_info.index].width) / portal.width;

      // test if reaching zoom limit
      if (camera_zoom / scale < zoom_min) continue;

      camera_coord.x -= portal.x;
      camera_coord.y -= portal.y;
      camera_coord *= scale;
      camera_zoom /= scale;
      CoordTransform::UpdateCameraWorldRect();
      root_board = portal.to;
      UpdateBoardRectCache();
      return true;
    }
  }
  return false;
}

void Level::UpdateBoardRectCache() {
  static constexpr u32 max_cache = 255;

  auto root_rect_info =
      BoardRectInfo{root_board,
                    rl::Rect{0,
                             0,
                             (float)boards[root_board].width,
                             (float)boards[root_board].height}};
  board_rect_cache.clear();

  vector<BoardRectInfo> buffer = {root_rect_info};
  vector<BoardRectInfo> backbuffer;

  while (!buffer.empty()) {
    for (auto& info : buffer) {
      board_rect_cache.push_back(info);

      auto portal_AND_infos = GetChildRectInfo(info);
      for (auto& portal_AND_info : portal_AND_infos) {
        auto& child_info = portal_AND_info.second;

        float size_on_screen =
            std::max(child_info.rect.width * camera_zoom * canvas_size.x,
                     child_info.rect.height * camera_zoom * canvas_size.x);
        if (size_on_screen > 0.004f && board_rect_cache.size() <= max_cache) {
          backbuffer.push_back(child_info);
        }
      }
    }
    buffer.clear();
    buffer.swap(backbuffer);
  }
}

bool PortalRecord::RejectRoute(Portal& portal, bool go_up) {
  if (!this->portal) return false;

  // only always go up or down, rip adjacent boards
  if (this->go_up != go_up) {
    //if (this->portal.value()->from == portal.from &&
    //    this->portal.value()->to == portal.to)
      return true;
  }

  return false;
}

void Level::CalculateNeighbors() {
  static constexpr u32 max_depth = 4;
  static constexpr u32 max_cache = 63;

  for (u32 board_index = 0; board_index < boards.size(); board_index++) {
    auto& board = boards[board_index];
    auto root_board_info =
        BoardRectInfo{board_index,
                      rl::Rect{0,
                               0,
                               (float)boards[board_index].width,
                               (float)boards[board_index].height}};

    vector<BoardRectInfo> board_rect_infos;
    vector<PortalRecord> buffer = {{{}, false, 0, root_board_info}};
    vector<PortalRecord> backbuffer;

    while (!buffer.empty()) {
      for (auto& record : buffer) {
        board_rect_infos.push_back(record.board_info);
        if (record.depth >= max_depth) continue;  // no more child

        auto portal_AND_infos = GetChildRectInfo(record.board_info);
        for (auto& portal_AND_info : portal_AND_infos) {
          auto& portal = portal_AND_info.first;
          auto& child_info = portal_AND_info.second;

          if (!record.RejectRoute(portal, /*go_up = */ false) &&
              board_rect_infos.size() <= max_cache) {
            backbuffer.push_back(
                {&portal, /*.go_up = */ false, record.depth + 1, child_info});
          }
        }

        // don't go up with cloned boards, period. causes endless edge cases and
        // headaches
        // if (boards[record.board_info.index].has_clones) continue;
        portal_AND_infos = GetParentRectInfo(record.board_info, true);
        for (auto& portal_AND_info : portal_AND_infos) {
          auto& portal = portal_AND_info.first;
          auto& parent_info = portal_AND_info.second;
          if (!record.RejectRoute(portal, /*go_up*/ true) &&
              board_rect_infos.size() <= max_cache) {
            backbuffer.push_back(
                {&portal, /*.go_up*/ true, record.depth + 1, parent_info});
          }
        }
      }
      buffer.clear();
      buffer.swap(backbuffer);
    }
    // delete root rect, its collision is handled by offsetting the
    // coord like other minesweeper games
    board_rect_infos.erase(board_rect_infos.begin());

    for (i32 x = 0; x < board.width; x++) {
      for (i32 y = 0; y < board.height; y++) {
        auto& optional_cell = board.Get(x, y);
        if (!optional_cell) continue;
        auto& cell = optional_cell.value();

        // calculate neighbors on this board
        for (auto& offset : neighbor_deltas) {
          auto& optional_neighbor = board.Get(x + offset.x, y + offset.y);
          if (!optional_neighbor) continue;
          cell.neighbors.push_back(
              CellID{x + offset.x, y + offset.y, board_index});
        }

        // the expensive part. Calculate by brute forcing rect collisions
        auto cell_rect = rl::Rect{x - 0.001f, y - 0.001f, 1.002f, 1.002f};
        for (auto& board_rect_info : board_rect_infos) {
          // overloading board, board_index, x, and y because not doing so
          // is even more confusing
          auto board_index = board_rect_info.index;
          auto& board = boards[board_index];
          for (i32 x = 0; x < board.width; x++) {
            for (i32 y = 0; y < board.height; y++) {
              auto collision_rect =
                  board.GetCellRect(Vec2i{x, y}, board_rect_info.rect);
              if (board.Get(x, y) && cell_rect.CheckCollision(collision_rect)) {
                auto id = CellID{x, y, board_index};

                // unique neighbors
                if (std::find(cell.neighbors.begin(),
                              cell.neighbors.end(),
                              id) == cell.neighbors.end()) {
                  cell.neighbors.push_back(id);
                }
              }
            }
          }
        }
      }
    }
  }

  // The final duct tape. Ensures bidirectionality so if the above code missed
  // some cells this has a chance to save it
  for (u32 board_index = 0; board_index < boards.size(); board_index++) {
    auto& board = boards[board_index];
    for (i32 x = 0; x < board.width; x++) {
      for (i32 y = 0; y < board.height; y++) {
        if (!board.Get(x, y)) continue;
        auto& cell = board.Get(x, y).value();
        auto id = CellID{x, y, board_index};

        for (auto& neighbor : cell.neighbors) {
          auto& neighbor_cell_neighbors = Get(neighbor).value().neighbors;
          if (std::find(neighbor_cell_neighbors.begin(),
                        neighbor_cell_neighbors.end(),
                        id) == neighbor_cell_neighbors.end()) {
            neighbor_cell_neighbors.push_back(id);
          }
        }
      }
    }
  }
}

void Level::CalculateMineNumbers(bool override) {
  for (auto& board : boards) {
    for (i32 x = 0; x < board.width; x++) {
      for (i32 y = 0; y < board.height; y++) {
        if (!board.Get(x, y)) continue;
        auto& cell = board.Get(x, y).value();

        // Don't override at level selection and level generation, but do so
        // when repositioning the first-click mine
        if (override == false) {
          if (cell.number != 0) continue;
        }
        cell.number = 0;

        for (auto& neighbor : cell.neighbors) {
          if (Get(neighbor).value().mine) {
            cell.number++;
          }
        }
      }
    }
  }
}

void Level::RemoveHighLight() {
  if (!mouse_over) return;
  auto& cell = Get(mouse_over.value()).value();
  cell.highlighted = false;
  for (auto& neighbor : cell.neighbors) {
    Get(neighbor).value().highlighted = false;
  }
}

void Level::AddHighLight() {
  if (!mouse_over || name == "levelselection") return;
  auto& cell = Get(mouse_over.value()).value();
  cell.highlighted = true;
  if (cell.covered) return;
  // highlight empty cell's neighbors
  for (auto& neighbor : cell.neighbors) {
    Get(neighbor).value().highlighted = true;
  }
}

void Level::UpdateMouseOver() {
  // tiles' visual edge distance from the rect
  static constexpr float margin = 0.025f;
  mouse_over_last_frame = mouse_over;

  if (mouse_on_ui) {
    mouse_over = {};
    return;
  }

  // lazy update
  if (!camera_moved && GetMouseDelta() == rl::Vector2{0.0f, 0.0f}) return;

  mouse_over = {};

  rl::Vector2 mouse_pos = CoordTransform::PixelToWorld(
      (rl::Vector2)GetMousePosition() * ssaa_scale);
  for (auto& board_info : board_rect_cache) {
    auto& board = boards[board_info.index];
    auto& rect = board_info.rect;
    rl::Vector2 pos = (mouse_pos - rect.GetPosition()) / rect.GetSize() *
                      rl::Vector2{(float)board.width, (float)board.height};
    Vec2i pos_int = Vec2i{(i32)pos.x, (i32)pos.y};
    rl::Vector2 pos_fract = rl::Vector2{pos.x - pos_int.x, pos.y - pos_int.y};
    bool in_margin = (pos_fract.x > margin && pos_fract.x < (1 - margin) &&
                      pos_fract.y > margin && pos_fract.y < (1 - margin));
    if (board.Get(pos_int) && in_margin) {
      mouse_over = CellID{pos_int, board_info.index};
      return;
    }
  }
}

void Level::HandleMouseInput() {
  if (state != State::gaming) return;
  if (name == "menu") return;

  // screen coordinates
  static constexpr float mouse_move_invalidate_dist = 0.0125f;
  static rl::Vector2 mouse_dist_since_click = {0, 0};

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) ||
      IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) ||
      IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
    mouse_dist_since_click += (rl::Vector2)GetMouseDelta() / window_size.x;
  } else {
    mouse_dist_since_click = 0;
  }
  bool moved_too_far =
      mouse_dist_since_click.Length() > mouse_move_invalidate_dist;

  // pop the click up if tile changed or mouse moved too far
  if (mouse_over_last_frame != mouse_over || moved_too_far) {
    if (mouse_over_last_frame) {
      auto& cell = Get(mouse_over_last_frame.value()).value();

      cell.pressed = false;
      cell.chord = false;
      for (auto& neighbor : cell.neighbors) {
        Get(neighbor).value().pressed = false;
      }
    }
  }

  if (!mouse_over) return;
  auto& cell = Get(mouse_over.value()).value();

  if (name == "levelselection") {
    if (cell.covered || cell.number == 0) return;
    // uncover down
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) cell.pressed = true;

    // uncover up
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && cell.pressed) {
      Serializer::Load(std::to_string(cell.number), *this);
    }
    return;
  }

  if (!cell.flagged) {
    // uncover down
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) cell.pressed = true;

    // uncover up
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && cell.pressed) Open(cell);

    // chording down
    if (!cell.covered && ((IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
                           IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) ||
                          (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) &&
                           IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ||
                          IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE))) {
      cell.chord = true;
      for (auto& neighbor : cell.neighbors) {
        if (!Get(neighbor).value().flagged)
          Get(neighbor).value().pressed = true;
      }
    }

    // chording up
    if (cell.chord && IsMouseButtonUp(MOUSE_BUTTON_LEFT) &&
        IsMouseButtonUp(MOUSE_BUTTON_MIDDLE) &&
        IsMouseButtonUp(MOUSE_BUTTON_RIGHT)) {
      cell.chord = false;
      for (auto& neighbor : cell.neighbors) {
        Get(neighbor).value().pressed = false;
      }
      Chord(cell);
    }
  }

  // right-click changing marks
  if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
    auto& cell = Get(mouse_over.value()).value();
    if (cell.pressed)
      cell.pressed = false;
    else if (cell.covered)
      CycleMarking(cell);
  }
}

void Level::Open(Cell& cell) {
  if (cell.flagged) return;
  if (!cell.covered) return;  // stop infinite recursing

  if (cell.mine && started == false) {
    // if the first click is a mine, put it at a random position on the same
    // board
    // spaghetti code here, checks current cell's ID by using mouse_over because
    // chording doesn't matter for the first click
    auto& id = mouse_over.value();
    auto& board = boards[id.board_index];
    while (true) {
      i32 x = GetRandomValue(0, board.width - 1);   // both sides inclusive
      i32 y = GetRandomValue(0, board.height - 1);  // both sides inclusive
      auto& optional_cell = board.Get(x, y);
      if (!optional_cell) continue;
      auto& new_cell = optional_cell.value();
      if (new_cell.mine || !new_cell.covered || new_cell.safe) continue;
      new_cell.mine = true;
      cell.mine = false;
      CalculateMineNumbers(true);
      break;
    }
  }

  cell.covered = false;
  started = true;
  cell.question_mark = false;

  if (cell.mine == true) {
    state = State::lost;
    return;
  }

  winning_check_needed = true;

  if (cell.number == 0) {
    for (auto& neighbor : cell.neighbors) {
      Open(Get(neighbor).value());
    }
  }
}

void Level::Chord(Cell& cell) {
  // no more to flag -> open all
  u32 flags = 0;
  for (auto& neighbor : cell.neighbors) {
    if (Get(neighbor).value().flagged) flags++;
  }
  if (cell.number == flags) {
    for (auto& neighbor : cell.neighbors) {
      Open(Get(neighbor).value());
    }
    return;
  }

  // covered amount equal number, mark all as mines
  // commented out because this is too op
  // u32 cover = 0;
  // for (auto& neighbor : cell.neighbors) {
  //   if (Get(neighbor).value().covered) cover++;
  // }
  // if (cover == cell.number) {
  //   for (auto& neighbor : cell.neighbors) {
  //     auto& cell = Get(neighbor).value();
  //     if (cell.covered) {
  //       cell.question_mark = false;
  //       cell.flagged = true;
  //     }
  //   }
  // }
}

void Level::CycleMarking(Cell& cell) {
  if (cell.flagged == false && cell.question_mark == false) {
    cell.flagged = true;
    mine_left--;
  } else if (cell.flagged == true) {
    cell.flagged = false;
    cell.question_mark = true;
    mine_left++;
  } else if (cell.question_mark == true) {
    cell.flagged = false;
    cell.question_mark = false;
  }
}

void Level::CheckGameWon() {
  winning_check_needed = false;
  for (auto& board : boards) {
    for (i32 x = 0; x < board.width; x++) {
      for (i32 y = 0; y < board.height; y++) {
        if (!board.Get(x, y)) continue;
        auto& cell = board.Get(x, y).value();
        if (cell.covered && !cell.mine) return;
      }
    }
  }
  mine_left = 0;
  state = State::won;
}

// ---------------- Drawing functions ---------------
void Cell::Draw(rl::Rect world_coord, State state, AtlasManager& atlas) {
  rl::Rect coord = CoordTransform::WorldToPixel(world_coord);
  if (!covered) {
    if (mine)
      atlas.Draw(Tile::detonated, coord);
    else
      atlas.Draw(number, coord);
  } else {  // covered
    if (flagged) {
      if (state == State::lost) {
        if (mine)
          atlas.Draw(Tile::mine, coord);
        else
          atlas.Draw(Tile::no_mine, coord);
        return;
      }
    } else if (state == State::lost && mine) {
      atlas.Draw(Tile::mine, coord);
      return;
    } else if (state == State::won && mine) {
      atlas.Draw(Tile::flagged, coord);
      return;
    }

    u8 offset = 0;
    if (question_mark) offset = 4;
    if (flagged) offset = 8;

    if (highlighted) {
      if (pressed)
        atlas.Draw((Tile)((int)Tile::covered_highlight_press + offset), coord);
      else
        atlas.Draw((Tile)((int)Tile::covered_highlight + offset), coord);
    } else {
      if (pressed)
        atlas.Draw((Tile)((int)Tile::covered_press + offset), coord);
      else
        atlas.Draw((Tile)((int)Tile::covered + offset), coord);
    }
  }
}

void Board::Draw(rl::Rect rect, State state, AtlasManager& atlas) {
  auto pixel_rect = CoordTransform::WorldToPixel(rect);
  if (pixel_rect.width * pixel_rect.height < 2.0f) return;
  for (i32 x = 0; x < width; x++) {
    for (i32 y = 0; y < height; y++) {
      auto element = Get(x, y);
      if (!element) continue;
      auto cell_rect = GetCellRect(Vec2i{x, y}, rect);
      element.value().Draw(cell_rect, state, atlas);
    }
  }
}

void Level::Draw(AtlasManager& atlas) {
  for (auto& info : board_rect_cache) {
    boards[info.index].Draw(info.rect, state, atlas);
  }
  for (auto& info : board_rect_cache) {
    DrawCloneHint(info);
  }
}

float smoothstep(float num, float edge0, float edge1) {
  // Scale, bias and saturate num to 0..1 range
  num = std::clamp((num - edge0) / (edge1 - edge0), 0.0f, 1.0f);
  // Evaluate polynomial
  return num * num * (3 - 2 * num);
}

// drawing a square shouldn't be this complex, yet here we are...
void Level::DrawCloneHint(BoardRectInfo info) {
  if (!boards[info.index].has_clones) return;

  static const rl::Color blue = {0x59e2ff00};
  static const rl::Color yellow = {0xff9a0000};

  auto& board = boards[info.index];
  auto color = info.clone ? blue : yellow;

  auto screen_rect = CoordTransform::WorldToScreen(info.rect);
  auto screen_rect_clipped = RectUtil::boolean_and(
      screen_rect, rl::Rect{0, 0, 1, inverse_aspect_ratio});
  float screen_perc = screen_rect_clipped.width * screen_rect_clipped.height /
                      inverse_aspect_ratio;

  float visibility;
  // A
  // |       ________________
  // |     _/                \__
  // |    /                     \__
  // |  _/                         \_
  // | /                             \
  // -------------------------------------->
  // 0.00004 0.0004          0.8    0.95
  if (screen_perc < 0.5f)
    visibility = smoothstep(screen_perc, 0.00004f, 0.0004f);
  else
    visibility = smoothstep(screen_perc, 0.95f, 0.8f);

  color.a = visibility * 255;
  //// 0.5-0.8 over 2 secs
  // float time_variance =
  //     ((info.clone ? 1.0 : -1.0) * sin(GetTime() * 3.1416 / 2.0) + 4.333333)
  //     * 0.15;
  // color.a = visibility * (time_variance + 0.5) / 1.5 * 255;
  // color.r *= time_variance;
  // color.g *= time_variance;
  // color.b *= time_variance;

  auto pixel_rect = CoordTransform::ScreenToPixel(screen_rect);
  static constexpr float margin = 0.025f;
  float roundness =
      margin / ((float)std::min(board.width, board.height) / 2.0f) * 2.0f;
  float line_width = pixel_rect.width / board.width * margin * 2.0f;

  // E N L A R G E (covers up the darkened edge of uncovered cells at low
  // size, but visible at big size, making them look less jarring)
  line_width += 0.0025f * std::max(canvas_size.x, canvas_size.y) *
                (screen_perc + 1.0f) / 2.0f;

  // by default it draws lines outside of the bounding box. This makes them
  // drawn in the middle
  pixel_rect.x += line_width / 2.0f;
  pixel_rect.y += line_width / 2.0f;
  pixel_rect.width -= line_width;
  pixel_rect.height -= line_width;

  line_width *= 1.75f;

  pixel_rect.DrawRoundedLines(roundness, 12, line_width, color);
}
