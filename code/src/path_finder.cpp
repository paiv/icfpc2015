
inline static moved
rotation_to(placement& from, placement& to)
{
  int diff = int(to.rotation) - int(from.rotation);
  switch (diff)
  {
    case 1:
    case -5: // R5 -> R0
      return moved::CW;
    case -1:
    case 5: // R0 -> R5
      return moved::CCW;
  }
  return moved::CW;
}

inline static moved
action_to(placement& from, placement& to)
{
  int diffq = (int)to.q - from.q;
  if (from.r == to.r)
  {
    if (diffq > 0)
      return moved::E;
    else if (diffq < 0)
      return moved::W;
    else
      return rotation_to(from, to);
  }
  else
  {
    if (diffq < 0)
      return moved::SW;
    else
      return moved::SE;
  }
}


typedef struct _search_state
{
  int backward_cost;
  int forward_cost;
  u8 is_terminal;
  u8 is_reversed;
  board board;
  unit unit;
  placement unitPosition;
  placement targetPosition;
  unordered_set<placement> visited;
  list<placement> path;

  friend bool operator ==(_search_state a, _search_state b)
  {
      return a.unitPosition == b.unitPosition;
  }

  friend ostream& operator << (ostream& so, const _search_state& state)
  {
    so << "path: ";
    for (const placement& p : state.path)
      so << p << "-";
    return so;
  }

} search_state;

template <> struct hash<search_state> {
  size_t operator()(const search_state& h) const {
    return hash<placement>()(h.unitPosition);
  }
};

inline static int
angle_distance(angle from, angle to)
{
  int d = abs(int(to) - int(from));
  if (d > 3)
    d = 6 - d;
  return d;
}

static list<search_state>
expand_search(const search_state& state)
{
  static const moved actions[] = { moved::E, moved::W, moved::SE, moved::SW, moved::CW, moved::CCW };

  list<search_state> result;

  const placement& currentPos = state.unitPosition;
  const placement& target = state.targetPosition;
  const cell targetCell = {target.q, target.r};
  unit currentUnit = GameMoveUnit(state.unit, currentPos);

  for (moved action : actions)
  {
    placement nextPos = change_position(currentPos, action, state.is_reversed);
    if (nextPos == currentPos
      || state.visited.find(nextPos) != state.visited.end())
      continue;

    unit movedUnit = GameMoveUnit(state.unit, nextPos);
    if (GameIsSameUnit(currentUnit, movedUnit)
      || !GameIsValidMove(movedUnit, state.board))
      continue;

    search_state next = state;
    next.unitPosition = nextPos;
    next.visited.insert(nextPos);
    next.path.push_back(nextPos);
    next.backward_cost++;
    next.forward_cost = hex_distance({nextPos.q, nextPos.r}, targetCell)
      + angle_distance(nextPos.rotation, target.rotation);
    next.is_terminal = nextPos == target;
    result.push_back(next);
  }

  return result;
}

static search_state
make_path_search(game_state& game, placement& target)
{
  placement pos = game.unitPosition;
  board& board = game.board;

  search_state state = {};
  state.board = board;
  state.unit = game.problem->units[game.unitIndex];
  state.unitPosition = pos;
  state.targetPosition = target;
  state.visited = game.visited;
  state.forward_cost = hex_distance({pos.q, pos.r}, {target.q, target.r})
    + angle_distance(pos.rotation, target.rotation);
  state.is_terminal = pos == target;

  return state;
}

static list<placement>
path_to(game_state& game, placement& target, int reversed)
{
  search_state state = make_path_search(game, target);

  if (reversed)
  {
    state.is_reversed = 1;
    swap(state.targetPosition, state.unitPosition);
    state.visited.erase(state.targetPosition);
    state.visited.insert(state.unitPosition);
  }

  // clog << "astar " << pos << "-" << target << endl;

  state = astar(state, &expand_search);

  // clog << state << endl;

  return state.path;
}

static list<placement>
path_to_and_lock(game_state& game, placement& target, int reversed)
{
  static const moved actions[] = { moved::E, moved::W, moved::SE, moved::SW };
  static const moved rotations[] = { moved::CW, moved::CCW };

  list<placement> path = path_to(game, target, reversed);
  if (path.size() == 0)
    return path;

  if (reversed)
  {
    path.pop_back();
    reverse(begin(path), end(path));
    path.push_back(target);
  }

  unit& u = game.problem->units[game.unitIndex];

  for (moved action : actions)
  {
    placement nextPos = change_position(target, action);
    unit movedUnit = GameMoveUnit(u, nextPos);
    if (!GameIsValidPlacement(movedUnit, game))
    {
      // cout << "lock position: " << nextPos << endl;
      path.push_back(nextPos);
      return path;
    }
  }

  unit currentUnit = GameMoveUnit(u, game.unitPosition);

  for (moved action : rotations)
  {
    placement nextPos = change_position(target, action);
    unit movedUnit = GameMoveUnit(u, nextPos);
    if (!GameIsSameUnit(currentUnit, movedUnit)
      && !GameIsValidPlacement(movedUnit, game))
    {
      // cout << "lock position: " << nextPos << endl;
      path.push_back(nextPos);
      return path;
    }
  }

  return {};
}
