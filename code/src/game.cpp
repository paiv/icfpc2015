
static const int MAX_CANDIDATES = 25;
static const int MC_TIMEOUT = 400;

static cell
GameGetSpawnCell(int boardWidth, int boardHeight, unit& u)
{
  cell left = {};
  cell right = {};

  for (cell c : u.members)
  {
    if (c.q <= left.q)
      left = c;
    if (c.q >= right.q)
      right = c;
  }

  s16 leftPos = (boardWidth - (right.q - left.q + 1)) / 2;

  return {leftPos,0};
}

static game_state
GameGetInitialState(problem& p, int seed)
{
  lcg_random random(seed);
  int unitIndex = random.next() % p.units.size();
  cell firstUnit = GameGetSpawnCell(p.width, p.height, p.units[unitIndex]);
  placement pos = {firstUnit.q, firstUnit.r, angle::R0};

  board board(p.width, p.height);
  for (cell& c : p.filled)
    board.set(c, true);

  game_state state = {
    &p,
    random,
    board,
    unitIndex,
    pos,
    {pos},
    p.source_length - 1,
  };

  return state;
}

static angle
rotate_cw(angle a)
{
  switch (a)
  {
    case angle::R0: return angle::R1;
    case angle::R1: return angle::R2;
    case angle::R2: return angle::R3;
    case angle::R3: return angle::R4;
    case angle::R4: return angle::R5;
    case angle::R5: return angle::R0;
  }
}

static angle
rotate_ccw(angle a)
{
  switch (a)
  {
    case angle::R0: return angle::R5;
    case angle::R1: return angle::R0;
    case angle::R2: return angle::R1;
    case angle::R3: return angle::R2;
    case angle::R4: return angle::R3;
    case angle::R5: return angle::R4;
  }
}

static cell
move_pos(cell pos, moved action, int reversed)
{
  switch (action)
  {
    case moved::E: return hex_neighbor(pos, 0);
    case moved::W: return hex_neighbor(pos, 3);
    case moved::SE: return hex_neighbor(pos, reversed ? 1 : 5);
    case moved::SW: return hex_neighbor(pos, reversed ? 2 : 4);
    default:
      return pos;
  }
}

static unit&
move_unit(const unit& u, const placement& pl, unit& v)
{
  assert (u.members.size() == v.members.size());

  cell offset = {pl.q, pl.r};
  v.pivot = hex_add(u.pivot, offset);

  const auto& members = u.members;
  for (size_t i = 0; i < members.size(); i++)
  {
    v.members[i] = hex_add(hex_rotate_cw(hex_subtract(members[i], u.pivot), (int)pl.rotation), v.pivot);
  }

  return v;
}

static unit
GameMoveUnit(const unit& u, const placement& pl)
{
  unit v;
  v.members.resize(u.members.size());
  return move_unit(u, pl, v);
}

static placement
change_position(const placement& currentPosition, moved action, int reversed = 0)
{
  placement nextPosition = currentPosition;
  switch (action)
  {
    case moved::E:
    case moved::W:
    case moved::SE:
    case moved::SW:
      {
        cell pos = {currentPosition.q, currentPosition.r};
        pos = move_pos(pos, action, reversed);
        nextPosition.q = pos.q;
        nextPosition.r = pos.r;
      }
      break;
    case moved::CW:
      nextPosition.rotation = rotate_cw(currentPosition.rotation);
      break;
    case moved::CCW:
      nextPosition.rotation = rotate_ccw(currentPosition.rotation);
      break;
  }
  return nextPosition;
}

static bool
GameIsSameUnit(const unit& u, const unit& v)
{
  auto uset = unordered_set<cell>(begin(u.members), end(u.members));
  for (auto& c : v.members)
  {
    auto it = uset.find(c);
    if (it == end(uset))
      return false;
    uset.erase(it);
  }
  return uset.size() == 0;
}

static bool
GameIsValidMove(unit& u, const board& board)
{
  s16 boardWidth = (s16)board.map[0].size();
  s16 boardHeight = (s16)board.map.size();

  for (cell& c : u.members)
  {
    OffsetCoord coord = roffset_from_cube(0, c);
    if (coord.col < 0
      || coord.col >= boardWidth
      || coord.row < 0
      || coord.row >= boardHeight)
      return false;

    if (board[c])
      return false;
  }

  return true;
}

static bool
GameIsValidPlacement(unit& u, game_state& nextState)
{
  return GameIsValidMove(u, nextState.board);
}

inline static bool
is_onboard(int width, int height, cell c)
{
  auto x = roffset_from_cube(0, c);
  return (x.col >= 0 && x.col < width
    && x.row >= 0 && x.col < height);
}

static bool
GameCanLockUnit(unit& u, game_state& nextState)
{
  problem *p = nextState.problem;
  board& board = nextState.board;
  int right = p->width;
  int bottom = p->height;

  for (cell& c : u.members)
  {
    OffsetCoord coord = roffset_from_cube(0, c);
    if (coord.col <= 0
      || coord.col >= right - 1
      || coord.row <= 0
      || coord.row >= bottom - 1)
      return true;

    for (int i = 0; i < 6; i++)
    {
      cell n = hex_neighbor(c, i);
      if (board[n])
        return true;
    }
  }
  return false;
}

static void
lock_unit(unit& u, board& board)
{
  for (cell& c : u.members)
    board.set(c, true);
}

inline static bool
all_true(const vector<bool>& v)
{
  for (bool b : v)
    if (!b) return false;
  return true;
}

static int
collapse_rows(board& board)
{
  int total_collapsed = 0;
  vector<vector<bool>>& map = board.map;
  for (int r = map.size() - 1; r >= 0; )
  {
    vector<bool>& row = map[r];
    if (all_true(row))
    {
      move_backward(begin(map), begin(map) + r, begin(map) + r + 1);
      map[0] = vector<bool>(row.size());
      total_collapsed++;
    }
    else
    {
      r--;
    }
  }
  return total_collapsed;
}

static game_state
GameLockUnit(game_state& currentState)
{
  unit& u = currentState.problem->units[currentState.unitIndex];
  unit currentUnit = GameMoveUnit(u, currentState.unitPosition);

  game_state nextState = currentState;

  lock_unit(currentUnit, nextState.board);
  int collapsed = collapse_rows(nextState.board);
  if (collapsed > 0)
  {
    int size = currentUnit.members.size();
    int ls_old = currentState.last_lines_collapsed;
    int points = size + 100 * (1 + collapsed) * collapsed / 2;
    int bonus = (ls_old > 1) ? ((ls_old - 1) * points / 10) : 0;
    nextState.score += points + bonus;
  }

  nextState.last_lines_collapsed = collapsed;

  if (nextState.unitsLeft <= 0)
  {
    nextState.is_terminal = true;
  }
  else
  {
    problem *p = nextState.problem;
    int unitIndex = nextState.random.next() % p->units.size();
    cell nextUnit = GameGetSpawnCell(p->width, p->height, p->units[unitIndex]);
    placement pos = { nextUnit.q, nextUnit.r, angle::R0 };

    unit spawnedUnit = GameMoveUnit(p->units[unitIndex], pos);

    if (!GameIsValidPlacement(spawnedUnit, nextState))
    {
      nextState.unitIndex = -1;
      nextState.unitPosition = {};
      nextState.visited = {};
      nextState.is_terminal = true;
    }
    else
    {
      nextState.unitIndex = unitIndex;
      nextState.unitPosition = pos;
      nextState.visited = {pos};
      nextState.unitsLeft--;
    }
  }

  return nextState;
}

static game_state
GameGetNextState(game_state& currentState, moved requestedAction)
{
  if (currentState.is_terminal)
    return currentState;

  game_state nextState = currentState;

  unit& u = currentState.problem->units[currentState.unitIndex];
  unit currentUnit = GameMoveUnit(u, currentState.unitPosition);

  placement nextPos = change_position(currentState.unitPosition, requestedAction);
  unit movedUnit = GameMoveUnit(u, nextPos);

  if (nextPos == currentState.unitPosition
    || currentState.visited.find(nextPos) != currentState.visited.end()
    || GameIsSameUnit(currentUnit, movedUnit))
  {
    nextState.score = 0;
    nextState.is_terminal = true;
    return nextState;
  }

  if (GameIsValidPlacement(movedUnit, nextState))
  {
    nextState.unitPosition = nextPos;
    nextState.visited.insert(nextPos);
    return nextState;
  }

  return GameLockUnit(nextState);
}

static game_state
GameFastForward(game_state& currentState, placement& lockPosition)
{
  if (currentState.is_terminal)
    return currentState;

  game_state nextState = currentState;
  nextState.unitPosition = lockPosition;

  return GameLockUnit(nextState);
}

static list<moved>
GameGetLegalMoves(game_state& currentState)
{
  static const moved actions[] = { moved::E, moved::W, moved::SE, moved::SW, moved::CW, moved::CCW };

  list<moved> result;

  unit u = currentState.problem->units[currentState.unitIndex];
  placement& currentPos = currentState.unitPosition;
  unit currentUnit = GameMoveUnit(u, currentPos);

  for (moved action : actions)
  {
    placement nextPos = change_position(currentPos, action);
    unit movedUnit = GameMoveUnit(u, nextPos);

    if (nextPos != currentPos
      && currentState.visited.find(nextPos) == currentState.visited.end()
      && !GameIsSameUnit(currentUnit, movedUnit))
    {
      result.push_back(action);
    }
  }

  return result;
}

static cell
unit_size(const unit& u)
{
  s16 hx = 0x7FFF;
  s16 hy = 0;
  s16 wx = 0x7FFF;
  s16 wy = 0;
  for (cell c : u.members)
  {
    hx = min(hx, c.r);
    hy = max(hy, c.r);
    wx = min(wx, c.q);
    wy = max(wy, c.q);
  }
  s16 h = hy - hx + 1;
  s16 w = wy - wx + 1;
  return {w, h};
}

static int
top_filled(board& board)
{
  auto& map = board.map;
  for (int i = 0; i < map.size(); i++)
  {
    for (bool x : map[i])
      if (x) return i;
  }
  return map.size();
}

static bool
has_holes(const board& board, const unit& u, const placement& pos)
{
  s16 width = board.map[0].size();
  s16 height = board.map.size();
  unit movedUnit = GameMoveUnit(u, pos);
  unordered_set<cell> members(begin(movedUnit.members), end(movedUnit.members));

  for (cell c : movedUnit.members)
  {
    if (c.r == board.map.size() - 1)
      continue;

    for (int i = 4; i < 6; i++)
    {
      cell n = hex_neighbor(c, i);
      if (!is_onboard(width, height, n))
        continue;

      if (!board.at(n) && members.find(n) == end(members))
      {
        int space = 0;
        for (int k = 1; k < 3; k++)
        {
          cell t = hex_neighbor(n, k);
          if (!is_onboard(width, height, t))
            space++;
          else if (!board.at(t) && members.find(t) == end(members))
            space++;
        }
        if (space < 2)
          return true;
      }
    }
  }
  return false;
}

static list<placement>
path_to_and_lock(game_state& game, placement& target, int reversed = 1);

static int
random_next(lcg_random& rand, int size)
{
  double v = rand.next() / (double)0x7FFF;
  return round(v * size);
}

static list<placement>
GameGetCandidatePlacements(game_state& currentState)
{
  list<placement> result;

  if (currentState.is_terminal)
    return {};

  problem *p = currentState.problem;
  board& board = currentState.board;
  unit u = currentState.problem->units[currentState.unitIndex];
  cell sz = unit_size(u);
  s16 h = max(sz.q, sz.r);
  s16 rowBottom = p->height - min(sz.q, sz.r);
  s16 rowTop = max(0, min(top_filled(board), p->height - h) - h + 1);
  lcg_random random(rowBottom * rowTop);

  unit movedUnit = u;

  for (s16 row = rowBottom; row >= rowTop; row--)
  {
    // for (int attempt = 0; attempt < 2*p->width; attempt++)
    {
    for (s16 icol = 0; icol < p->width; icol++)
    {
      s16 col = icol; //random_next(random, p->width);
      cell x = roffset_to_cube(0, {col, row});
      for (int rot = 0; rot < 6; rot++)
      {
        placement pos = {x.q, x.r, angle(rot)};
        move_unit(u, pos, movedUnit);
        if (GameIsValidPlacement(movedUnit, currentState)
          && GameCanLockUnit(movedUnit, currentState))
        {
          if (u.members.size() == 1)
          {
            result.push_back(pos);
            break;
          }

          if (angle(rot) != currentState.unitPosition.rotation)
          {
            placement pos = {0, 0, angle(rot)};
            if (GameIsSameUnit(u, move_unit(u, pos, movedUnit)))
              continue;
          }

          result.push_back(pos);
          // if (result.size() > 10 * MAX_CANDIDATES)
            // return result;
        }
      }
    }}
  }
  return result;
}

static list<placement>
GameFilterCandidatePlacements(game_state& currentState, list<placement>& candidates)
{
  vector<placement> result;

  unit u = currentState.problem->units[currentState.unitIndex];
  problem *p = currentState.problem;
  board& board = currentState.board;

  for (placement& pos : candidates)
  {
    auto track = path_to_and_lock(currentState, pos, true);
    if (track.size() > 0)
      result.push_back(pos);

    // if (result.size() > MAX_CANDIDATES)
      // break;
  }

  auto bound = partition(begin(result), end(result),
    [board, u, p](const placement& p) {
      return !has_holes(board, u, p);
  });

  // if (bound == end(result))
  {
    bound = begin(result);
    // return list<placement>(bound, end(result));
  }

  sort(bound, end(result), [](const placement& a, const placement& b) {
    return a.r > b.r;
  });

  return list<placement>(bound, end(result));
}

static board
load_board(string bs)
{
  vector<vector<bool>> map;

  bool linein = false;
  vector<bool> line;
  int pos = 0;
  for (char c : bs)
  {
    switch (c)
    {
      case '|':
        if (!linein)
        {
          if (line.size() > 0)
            map.push_back(line);
          line = vector<bool>();
          pos = 0;
        }
        linein = !linein;
        break;
      case 'o':
        if (linein)
        {
          pos++;
          line.push_back(true);
        }
        break;
      case ' ':
        if (linein)
        {
          if (pos++ & 1)
            line.push_back(false);
        }
        break;
    }
  }
  map.push_back(line);

  board b;
  b.map = map;
  return b;
}

static game_state
GameLoadState(int step, problem& problem, int seedIndex, string boardString)
{
  auto it = begin(problem.source_seeds);
  advance(it, seedIndex);
  lcg_random random(*it);

  int unitIndex = 0;
  for (int i = 0; i < step; i++)
    unitIndex = random.next() % problem.units.size();

  board board = load_board(boardString);

  cell spawnCell = GameGetSpawnCell(problem.width, problem.height, problem.units[unitIndex]);
  placement unitPos = {spawnCell.q, spawnCell.r, angle::R0};

  return {
    &problem,
    random,
    board,
    unitIndex,
    unitPos,
    {unitPos},
    problem.source_length - step,

    /*
    u16 last_lines_collapsed;
    u8 is_terminal;
    u32 score;
    */
  };
}

static list<unit>
GameGetNextUnits(int n, const game_state& state)
{
  list<unit> result;
  problem *p = state.problem;
  lcg_random random = state.random;

  while (n-- > 0)
  {
    int unitIndex = random.next() % p->units.size();
    result.push_back(p->units[unitIndex]);
  }

  return result;
}
