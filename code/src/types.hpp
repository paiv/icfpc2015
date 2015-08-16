
#ifndef __paiv_types_hpp
#define __paiv_types_hpp

namespace paiv {

typedef Hex cell;

typedef struct unit_t
{
  cell pivot;
  vector<cell> members;
} unit;


typedef struct problem_t
{
  int problem_id;
  vector<unit> units;
  int width;
  int height;
  vector<cell> filled;
  int source_length;
  list<int> source_seeds;
} problem;


typedef struct solution_t
{
  int problem_id;
  int seed;
  string tag;
  string commands;
  int score;
} solution;


class board
{
public:
  vector<vector<bool>> map;

  board() {}
  board(int width, int height)
  {
    for (int r = 0; r < height; r++) {
        map.emplace_back(width);
    }
  }

  bool operator ==(const board& other) const
  {
    for (int row = 0; row < map.size(); row++)
      if (map[row] != other.map[row])
        return false;
    return true;
  }

  friend ostream& operator << (ostream& so, const board& board)
  {
    size_t width = board.map[0].size();

    so << "  ";
    for (int i = 0; i < width; i++)
      so << "--";
    so << "- " << endl;

    bool odd = false;
    for (auto& row : board.map)
    {
      so << " |";
      for (bool v : row)
        so << (v ? ( odd ? " o" : "o ") : "  ");
      so << " |" << endl;
      odd = !odd;
    }

    so << "  ";
    for (int i = 0; i < width; i++)
      so << "--";
    so << "- " << endl;

    return so;
  }

  inline bool at(cell c) const
  {
    return map[c.r][c.q + (c.r >> 1)];
  }

  inline bool at(int col, int row) const
  {
    return map[row][col];
  }

  inline bool operator[](cell c) const
  {
    return at(c);
  }

  inline void set(cell c, bool value)
  {
    map[c.r][c.q + (c.r >> 1)] = value;
  }

  inline void set(int col, int row, bool value)
  {
    map[row][col] = value;
  }
};


enum struct moved : u8
{
  E = 'e',
  W = 'w',
  SE = 'u',
  SW = 'v',
  CW = 'r',
  CCW = 'q',
};

enum struct angle : u8
{
  R0,
  R1,
  R2,
  R3,
  R4,
  R5,
};

typedef struct placement_t
{
  s16 q;
  s16 r;
  angle rotation;

  bool operator ==(const placement_t& other) const
  {
    return q == other.q
      && r == other.r
      && rotation == other.rotation;
  }
  bool operator !=(const placement_t& other) const
  {
    return !(*this == other);
  }

  friend ostream& operator << (ostream& so, const placement_t& p)
  {
    auto x = roffset_from_cube(0, {p.q,p.r});
    so << "(" << x.col << "," << x.row << "," << (int)p.rotation << ")";
    return so;
  }
} placement;

}

namespace std {
  template <> struct hash<paiv::placement> {
    size_t operator()(const paiv::placement& h) const {
      hash<int> hasher;
      int seed = 12345;
      seed ^= hasher(h.q) + 0x9e3779b9 + (seed<<6) + (seed>>2);
      seed ^= hasher(h.r) + 0x9e3779b9 + (seed<<6) + (seed>>2);
      seed ^= hasher((int)h.rotation) + 0x9e3779b9 + (seed<<6) + (seed>>2);
      return seed;
    }
  };

  template <> struct hash<paiv::board> {
    size_t operator()(const paiv::board& v) const {
      hash<vector<bool>> hasher;
      int seed = 12345;
      for (auto& x : v.map)
        seed ^= hasher(x) + 0x9e3779b9 + (seed<<6) + (seed>>2);
      return seed;
    }
  };
}

namespace paiv {

  typedef struct game_state_t
  {
    problem* problem;
    lcg_random random;
    board board;
    int unitIndex;
    placement unitPosition;
    unordered_set<placement> visited;
    int unitsLeft;
    u16 last_lines_collapsed;
    u8 is_terminal;
    u32 score;

    friend ostream& operator << (ostream& so, const game_state_t& state)
    {
      int step = state.problem->source_length - state.unitsLeft;
      so << "unit " << step << ":" << endl;
      so << state.board;
      return so;
    }
  } game_state;

}

#endif
