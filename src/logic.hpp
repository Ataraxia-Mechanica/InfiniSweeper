#pragma once

#include <optional>
#include <utility>
#include <vector>

#include "atlas.hpp"
#include "fixed_size_int.hpp"
#include "rl.hpp"
#include "serializer.hpp"
#include "vec2i.hpp"

using std::optional;
using std::pair;
using std::vector;

enum class State {
  gaming,
  lost,
  won,
};

struct CellID {
  i32 x;
  i32 y;
  u32 board_index;
  inline constexpr CellID(i32 x, i32 y, u32 i) : x(x), y(y), board_index(i){};
  inline constexpr CellID(Vec2i p, u32 i) : x(p.x), y(p.y), board_index(i){};
  inline Vec2i ToVec2i() { return Vec2i{x, y}; };
  inline bool operator==(const CellID& other) const {
    return x == other.x && y == other.y && board_index == other.board_index;
  };
  inline bool operator!=(const CellID& other) const {
    return !(*this == other);
  };
};

struct Portal {
  i32 x;
  i32 y;
  i32 width;
  i32 height;
  u32 from;
  u32 to;
  bool clone = false;  // when zooming out of a board, zoom out from the one
                       // that's not clone.
};

struct Cell {
  bool covered = true;
  // used only at board generation phase, stops mines from spawning in
  bool safe = false;
  bool mine = false;
  u32 number = 0;

  bool flagged = false;
  bool question_mark = false;

  bool highlighted = false;
  bool pressed = false;
  bool chord = false;
  vector<CellID> neighbors;
  void Draw(rl::Rect world_coord, State state, AtlasManager& atlas);
};

struct Board {
  u32 width;
  u32 height;
  vector<optional<Cell>> cells;
  bool has_clones = false;

  bool Inside(Vec2i pos);
  rl::Rect GetCellRect(Vec2i pos, rl::Rect board_rect);
  optional<Cell>& Get(Vec2i pos);     // checked
  optional<Cell>& Get(i32 x, i32 y);  // checked
  void Draw(rl::Rect rect, State state, AtlasManager& atlas);
};

struct BoardRectInfo {
  u32 index;
  rl::Rect rect;
  bool clone = false;
};

// saves data about how to arrive at current rect. stops backtracking
struct PortalRecord {
  optional<Portal*> portal;
  bool go_up;
  u32 depth;
  BoardRectInfo board_info;

  bool RejectRoute(Portal& portal, bool go_up);
};

namespace Serializer {
void Load(std::string name, Level& level, int completed_levels);
};

class Level {
 public:
  friend void Serializer::Load(std::string name,
                               Level& level,
                               int completed_levels);
  std::string name;
  i32 mine_left;  // could be negative when falsely marked more mines
  State state;
  bool started;
  float time;

  void Tick();
  void Draw(AtlasManager& atlas);

 private:
  optional<CellID> mouse_over;
  optional<CellID> mouse_over_last_frame;

  vector<Board> boards;
  vector<Portal> portals;
  vector<BoardRectInfo> board_rect_cache;

  u32 root_board;

  void DrawCloneHint(BoardRectInfo info);

  optional<Cell>& Get(CellID id);

  // assume scale at 1, UL 0,0
  vector<pair<Portal&, BoardRectInfo>> GetParentRectInfo(
      u32 index, bool non_clone_only = false);
  vector<pair<Portal&, BoardRectInfo>> GetChildRectInfo(u32 index);

  vector<pair<Portal&, BoardRectInfo>> GetParentRectInfo(
      BoardRectInfo info, bool non_clone_only = false);
  vector<pair<Portal&, BoardRectInfo>> GetChildRectInfo(BoardRectInfo info);

  // moves camera when needed
  void ChangeRootBoard();
  bool ChangeRootBoardOnce();

  void UpdateBoardRectCache();

  void CalculateNeighbors();  // go through each portal at most once, rect clip
                              // all the cells in the clipping boardrect
  void CalculateNeighborsRecursionBoardRectUp(
      BoardRectInfo info,
      vector<BoardRectInfo>& board_rects,
      vector<bool>& portal_gone_through);
  void CalculateNeighborsRecursionBoardRectDown(
      BoardRectInfo info,
      vector<BoardRectInfo>& board_rects,
      vector<bool>& portal_gone_through);

  void CalculateMineNumbers(bool override = false);

  void RemoveHighLight();
  void AddHighLight();
  void UpdateMouseOver();
  void HandleMouseInput();

  // recursively opens empty cells' neighbors
  // does not open a flagged cell
  void Open(Cell& cell);
  void Chord(Cell& cell);  // when neighbors' marked mine amount matches
  void CycleMarking(Cell& cell);

  bool winning_check_needed = false;
  void CheckGameWon();
};  // namespace Level
