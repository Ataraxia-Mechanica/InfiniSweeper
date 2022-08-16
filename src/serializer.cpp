#include "serializer.hpp"

#include <toml.h>

#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "logic.hpp"
#include "scene.hpp"  //load max levels into scene manager
#include "transform.hpp"

using std::optional;
using std::vector;

void Serializer::Load(std::string name, Level& level, int completed_levels) {
  level.boards.clear();
  level.boards.reserve(256);
  level.portals.clear();
  level.board_rect_cache.clear();

  level.name = name;
  level.mine_left = 0;
  level.state = State::gaming;
  level.started = false;
  level.time = 0;
  level.mouse_over = {};
  level.mouse_over_last_frame = {};
  level.root_board = 0;

  // load the file every time. Hot reloading easier to design maps
  auto levels = toml::parse_file("levels.toml");
  auto level_node = levels[name];

  // boards
  vector<optional<int>> target_mine;
  optional<int> total_mine = level_node["totalmine"].value<int>();

  for (int i = 0; level_node["board" + std::to_string(i)].is_string(); i++) {
    auto board_str = std::stringstream(
        level_node["board" + std::to_string(i)].value<std::string>().value());
    level.boards.emplace_back();
    auto& board = level.boards[i];
    target_mine.push_back(
        level_node["board" + std::to_string(i) + "mine"].value<int>());

    int y = 0;
    int board_mine = 0;
    for (std::string line; std::getline(board_str, line, '\n'); y++) {
      board.width = line.length() / 2;
      for (int x = 0; x < board.width; x++) {
        char token[3] = {line[2 * x], line[2 * x + 1], '\0'};

        if (strcmp(token, "[]") == 0) {
          board.cells.push_back(Cell{});
        } else if (strcmp(token, "<>") == 0) {
          board.cells.push_back(Cell{.covered = false});
        } else if (strcmp(token, "mm") == 0) {
          board.cells.push_back(Cell{.mine = true});
          board_mine++;
        } else if (strcmp(token, "ff") == 0) {
          board.cells.push_back(Cell{.mine = true, .flagged = true});
          board_mine++;
        } else if (strcmp(token, "sf") == 0) {
          board.cells.push_back(Cell{.safe = true});
        } else if ((token[0] >= '0' && token[0] <= '9') &&
                   (token[1] >= '0' && token[1] <= '9')) {
          int number = (token[0] - '0') * 10 + (token[1] - '0');
          // if(number - level_completed <= 1)
          board.cells.push_back(Cell{.covered = false, .number = (u32)number});
          // else board.cells.push_back(Cell{});
        } else {
          board.cells.push_back({});
        }
      }
    }
    board.height = y;

    while (board_mine < target_mine[i].value_or(0)) {
      int x = GetRandomValue(0, board.width - 1);   // both sides inclusive
      int y = GetRandomValue(0, board.height - 1);  // both sides inclusive
      auto& optional_cell = board.Get(x, y);
      if (!optional_cell) continue;
      auto& cell = optional_cell.value();

      if (!cell.mine && !cell.safe && cell.covered) {
        cell.mine = true;
        board_mine++;
      }
    }
  }

  // assign additional mines from total_mine
  if (total_mine) {
    level.mine_left = total_mine.value();
    vector<Board*> board_to_add;
    int board_amount = level.boards.size();
    int current_total_mine = 0;

    for (int i = 0; i < board_amount; i++) {
      auto& board = level.boards[i];
      for (i32 x = 0; x < board.width; x++) {
        for (i32 y = 0; y < board.height; y++) {
          if (!board.Get(x, y)) continue;
          if (!board.Get(x, y).value().mine) continue;
          current_total_mine++;
        }
      }
    }

    for (int i = 0; i < board_amount; i++) {
      if (!target_mine[i]) {
        board_to_add.push_back(&level.boards[i]);
      }
    }
    // every board had mine amount, assign mine to every board instead
    if (board_to_add.empty()) {
      for (int i = 0; i < board_amount; i++) {
        board_to_add.push_back(&level.boards[i]);
      }
    }

    // to ensure distribution is uniform, get cell amount of all boards
    // also counts empty cells!
    int total_cell_amount = 0;
    vector<int> cell_amount;

    for (int i = 0; i < board_to_add.size(); i++) {
      auto& board = *board_to_add[i];
      total_cell_amount += board.width * board.height;
      cell_amount.push_back(board.width * board.height);
    }

    while (current_total_mine < total_mine.value()) {
      int index = GetRandomValue(0, total_cell_amount - 1);
      int i = 0;
      while (index >= 0 && i < cell_amount.size()) {
        index -= cell_amount[i];
        i++;
      }
      // go back one step
      i--;
      index += cell_amount[i];
      auto& optional_cell = board_to_add[i]->cells[index];
      if (!optional_cell) continue;
      auto& cell = optional_cell.value();

      if (!cell.mine && !cell.safe && cell.covered) {
        cell.mine = true;
        current_total_mine++;
      }
    }
  } else {
    for (auto& board : level.boards) {
      for (i32 x = 0; x < board.width; x++) {
        for (i32 y = 0; y < board.height; y++) {
          if (board.Get(x, y) && board.Get(x, y).value().mine) {
            level.mine_left++;
          }
        }
      }
    }
  }

  // portals
  if (level_node["portals"].is_array()) {
    // visitor pattern... dunno how this works so it's just copy paste
    level_node["portals"].as_array()->for_each([&level](auto&& portal_node) {
      if constexpr (toml::is_table<decltype(portal_node)>) {
        level.portals.emplace_back();
        auto& portal = level.portals.back();

        portal.from = portal_node["from"].template value<int>().value();
        portal.to = portal_node["to"].template value<int>().value();
        portal.x = portal_node["x"].template value<int>().value();
        portal.y = portal_node["y"].template value<int>().value();
        portal.width = portal_node["w"].template value<int>().value();
        portal.height = portal_node["h"].template value<int>().value();
        portal.clone = portal_node["clone"].value_or(false);
        if (portal.clone) level.boards[portal.to].has_clones = true;
      }
    });
  }

  level.CalculateNeighbors();
  level.CalculateMineNumbers();

  if (name == "levelselection") {
    for (auto& board : level.boards) {
      for (i32 x = 0; x < board.width; x++) {
        for (i32 y = 0; y < board.height; y++) {
          if (board.Get(x, y)) {
            if (max_levels < board.Get(x, y).value().number)
              max_levels = board.Get(x, y).value().number;
            if (board.Get(x, y).value().number - 1 > completed_levels) {
              board.Get(x, y).value().covered = true;
            }
          }
        }
      }
    }
  }

  auto& root_board = level.boards[level.root_board];

  if (name == "mainmenu") {
    camera_coord.x = 4.0f;
    camera_coord.y = 4.5f;
    camera_zoom = 1.0f / 3.0f;
  } else {
    camera_coord.x = (float)root_board.width / 2;
    camera_coord.y = (float)root_board.height / 2;
    camera_zoom = 0.5f / root_board.width;
  }
  // needed to keep everything inside view on first frame
  CoordTransform::UpdateCamera();
  level.ChangeRootBoard();
  level.UpdateBoardRectCache();
}
