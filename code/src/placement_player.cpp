
typedef tuple<board, int, placement> tid;
typedef tuple<double, int> vid;

namespace std {
  template <> struct hash<tid> {
    size_t operator()(const tid& v) const {
      int seed = hash<board>()(get<0>(v));
      seed ^= hash<int>()(get<1>(v)) + 0x9e3779b9 + (seed<<6) + (seed>>2);
      seed ^= hash<placement>()(get<2>(v)) + 0x9e3779b9 + (seed<<6) + (seed>>2);
      return seed;
    }
  };
}

class placement_player
{
  mt19937 generator;
  unordered_map<int, placement> targets;
  list<placement> track;

  // (board, unitIndex, placement) -> (score, visits)
  unordered_map<tid, vid> stats;

  moved choose_uct_track(game_state& currentState);
  double depth_charge_random(game_state state);
  tuple<double,int> depth_charge_uct(game_state state, placement nextPos);

public:
  placement_player();
  tuple<string, int> solve(problem& p, int seed);
  moved step(game_state& currentState);
  list<placement> random_hopper(game_state& state);
};

placement_player::placement_player()
{
  auto seed = system_clock::now().time_since_epoch().count();
  generator = mt19937(seed);
}

moved
placement_player::step(game_state& currentState)
{
  auto target = targets.find(currentState.unitsLeft);
  if (target != end(targets))
  {
    if (track.front() == currentState.unitPosition)
    {
      track.pop_front();
      if (track.size() > 0)
        return action_to(currentState.unitPosition, track.front());
      // else
      //   clog << "! index:" << currentState.unitIndex << ", left:" << currentState.unitsLeft << endl;
    }
  }

  // ~~ logging
  game_state logState = currentState;
  unit& u = logState.problem->units[logState.unitIndex];
  unit currentUnit = GameMoveUnit(u, logState.unitPosition);
  lock_unit(currentUnit, logState.board);
  clog << logState;
  // ~~

  return choose_uct_track(currentState);
}

list<placement>
placement_player::random_hopper(game_state& state)
{
  list<placement> places = GameGetCandidatePlacements(state);
  places = GameFilterCandidatePlacements(state, places);
  if (places.size() == 0)
    return {};

  uniform_int_distribution<int> distribution(0, places.size() - 1);
  int randomIndex = distribution(generator);
  return {vector<placement>(begin(places), end(places))[randomIndex]};
}

double
heuristics_score(game_state& state)
{
  vector<vector<bool>>& map = state.board.map;
  int width = map[0].size();
  int height = map.size();
  int hrow = 0;
  int hcol = 0;
  int boardSize = width * height;

  for (int r = 0; r < map.size(); r++)
  {
    int x = 0;
    int gaps = 0;
    bool b = false;
    for (bool v : map[r])
    {
      x += v;
      if (b != v)
      {
        b = v;
        gaps++;
      }
    }
    hrow += r * (x - gaps);
  }

  for (int col = 0; col < width; col++)
  {
    int gaps = 0;
    bool b = false;
    for (int row = 0; row < height; row++)
    {
      bool v = map[row][col];
      if (b != v)
      {
        b = v;
        gaps += (height - row);
      }
    }
    hcol -= gaps;
  }

  // problem *p = state.problem;

  return (double)state.score
    + (double)(hrow + hcol) / boardSize;
    // + 20. * (p->source_length - state.unitsLeft) / (double)p->source_length;
    // + (p->source_length - state.unitsLeft);
}

double
placement_player::depth_charge_random(game_state state)
{
  int len = 0;
  while (!state.is_terminal)
  {
    len++;
    list<placement> pos = random_hopper(state);
    if (pos.size() == 0)
      break;
    state = GameFastForward(state, pos.front());
  }
  return heuristics_score(state);
}

inline static double
uct_value(int parentVisits, int nodeVisits, double nodeScore)
{
  static const double lambda = 1.414213562373;
  return nodeScore / nodeVisits + lambda * sqrt(log(parentVisits) / nodeVisits);
}

static vid&
safe_index(unordered_map<tid, vid>& stats, const tid& k)
{
  auto it = stats.find(k);
  if (it == end(stats))
  {
    stats[k] = {0,0};
    return stats[k];
  }
  return it->second;
}

static int
uct_index(board& board, int unitIndex, vector<placement>& places, int parentVisits, unordered_map<tid, vid>& stats)
{
  int bestIndex = 0;
  double bestValue = 0;
  for (int i = 0; i < places.size(); i++)
  {
    auto it = stats.find({board, unitIndex, places[i]});
    if (it == end(stats))
      return i;

    vid childStats = it->second;

    if (parentVisits == 0 || get<1>(childStats) == 0)
      return i;

    double x = uct_value(parentVisits, get<1>(childStats), get<0>(childStats));
    if (x > bestValue)
    {
      bestValue = x;
      bestIndex = i;
    }
  }
  return bestIndex;
}

// score, max depth
tuple<double,int>
placement_player::depth_charge_uct(game_state state, placement nextPos)
{
  list<tid> path;

  double score = heuristics_score(state);

  while (!state.is_terminal)
  {
    tid childId = {state.board, state.unitIndex, nextPos};
    path.push_back(childId);

    if (stats.find(childId) == end(stats))
    {
      list<placement> pos = random_hopper(state);
      if (pos.size() == 0)
        break;

      nextPos = pos.front();

      state = GameFastForward(state, nextPos);
      score = depth_charge_random(state);
      break;
    }

    vid& childStats = stats[childId];

    state = GameFastForward(state, nextPos);
    score = heuristics_score(state);

    list<placement> places = GameGetCandidatePlacements(state);
    if (places.size() == 0)
      break;

    vector<placement> vplaces(begin(places), end(places));

    int placeIndex = uct_index(state.board, state.unitIndex, vplaces, get<1>(childStats), stats);
    nextPos = vplaces[placeIndex];
  }

  for (tid& childId : path)
  {
    vid& childStats = safe_index(stats, childId);
    stats[childId] = { get<0>(childStats) + score, get<1>(childStats) + 1 };
  }

  return make_tuple(score, path.size());
}

moved
placement_player::choose_uct_track(game_state& currentState)
{
  list<placement> places = GameGetCandidatePlacements(currentState);
  places = GameFilterCandidatePlacements(currentState, places);
  clog << places.size() << " candidate placements" << endl;
  if (places.size() == 0)
    return moved::SE;

  placement target = places.front();
  vector<placement> vplaces(begin(places), end(places));

  if (places.size() == 1)
  {
    track = path_to_and_lock(currentState, target);
    if (track.size() > 0)
    {
      game_state goalState = GameFastForward(currentState, target);
      clog << "BEST (0) target: " << target << " score:" << heuristics_score(goalState) << endl;
      clog << "goal:" << endl << goalState.board;

      targets.clear();
      targets[currentState.unitsLeft] = target;

      return action_to(currentState.unitPosition, track.front());
    }
    return moved::SW;
  }

  {
    auto startTime = system_clock::now();
    auto finishBy = startTime + duration<int, milli>(MC_TIMEOUT);

    int totalVisits = 0;
    int maxDepth = 0;

    while (true)
    {
      if (system_clock::now() > finishBy)
        break;

      int placeIndex = uct_index(currentState.board, currentState.unitIndex, vplaces, totalVisits, stats);
      auto sco_dep = depth_charge_uct(currentState, vplaces[placeIndex]);
      maxDepth = max(maxDepth, get<1>(sco_dep));

      if (totalVisits++ > 10000)
        break;
    }

    clog << "visits: " << totalVisits << ", stats size: " << stats.size()
      << ", depth " << maxDepth << endl;
  }

  // clog << "FROM stats (visits,score): ";
  // for (auto p : stats)
  //   clog << "(" << get<1>(p.second) << ") " << get<0>(p.second) << " | ";
  // clog << endl;

  typedef tuple<placement,double,int> plscore;
  vector<plscore> valid_places;

  for (int i = 0; i < vplaces.size(); i++)
  {
    auto it = stats.find({currentState.board, currentState.unitIndex, vplaces[i]});
    if (it == end(stats))
      continue;

    vid& stateStats = it->second;
    int n = get<1>(stateStats);

    if (n > 0)
    {
      double score = get<0>(stateStats) / n;
      valid_places.push_back({vplaces[i], score, i});
    }
  }

  sort(begin(valid_places), end(valid_places), [](const plscore& a, const plscore& b) {
    return get<1>(a) > get<1>(b);
  });

  // ~~ logging
  // clog << "from candidates: ";
  // for (auto& best : valid_places)
  // {
  //   clog << "[" << get<0>(best) << ", " << get<1>(best) << "] ";
  // }
  // clog << endl;
  // ~~

  for (auto& best : valid_places)
  {
    target = get<0>(best);
    track = path_to_and_lock(currentState, target);
    if (track.size() > 0)
    {
      clog << "BEST (" << get<2>(best) << ") target: " << target << " score:" << get<1>(best) << endl;
      game_state goalState = GameFastForward(currentState, target);
      clog << "goal:" << endl << goalState.board;

      targets.clear();
      targets[currentState.unitsLeft] = target;

      return action_to(currentState.unitPosition, track.front());
    }
  }

  return moved::SW;
}

tuple<string, int>
placement_player::solve(problem& p, int seed)
{
  list<moved> actionsTaken;

  game_state currentState = GameGetInitialState(p, seed);

  while (!currentState.is_terminal)
  {
    moved action = step(currentState);
    actionsTaken.push_back(action);
    currentState = GameGetNextState(currentState, action);
  }

  int score = currentState.score;
  string commands = solve_phrases(actionsTaken);

  return make_tuple(commands, score);
}
