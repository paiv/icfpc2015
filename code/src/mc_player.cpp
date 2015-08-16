
class mc_player
{
public:
  tuple<string, int> solve(problem& p, int seed);
  moved step(game_state& currentState);
};

double
mc_heuristics_score(game_state& state)
{
  vector<vector<bool>>& map = state.board.map;
  int board_heuristic = 0;
  int boardSize = map.size() * map[0].size();

  for (int r = 0; r < map.size(); r++)
  {
    int x = 0;
    for (bool v : map[r])
      x += int(v);
    board_heuristic += 10 * r * x;
  }

  problem *p = state.problem;
  return state.score
    + (double)board_heuristic / boardSize
    + 1000. * (p->source_length - state.unitsLeft) / (double)p->source_length;
}

static double
depth_charge_random(game_state state)
{
  random_player rplay;

  while (!state.is_terminal)
  {
    moved action = rplay.step(state);
    state = GameGetNextState(state, action);
  }

  return mc_heuristics_score(state);
}

moved
mc_player::step(game_state& currentState)
{
  list<moved> actions = GameGetLegalMoves(currentState);
  if (actions.size() == 0)
    return moved::SE;

  moved action = actions.front();
  if (actions.size() == 1)
    return action;

  vector<moved> vactions(begin(actions), end(actions));

  auto startTime = system_clock::now();
  auto finishBy = startTime + duration<int, milli>(400);

  vector<double> totalPoints(vactions.size());
  vector<int> totalAttempts(vactions.size());

  for (int i = 0; true; i = (i + 1) % vactions.size())
  {
    if (system_clock::now() > finishBy)
      break;

    double score = depth_charge_random(GameGetNextState(currentState, vactions[i]));
    totalPoints[i] += score;
    if ((totalAttempts[i] += 1) > 1000)
      break;
  }

  vector<double> expectedPoints(vactions.size());
  for (int i = 0; i < vactions.size(); i++)
    expectedPoints[i] = totalPoints[i] / totalAttempts[i];

  int best = 0;
  double bestScore = expectedPoints[0];
  for (int i = 1; i < vactions.size(); i++)
  {
    double score = expectedPoints[i];
    if (score > bestScore)
    {
      bestScore = score;
      best = i;
    }
  }

  return vactions[best];
}

tuple<string, int>
mc_player::solve(problem& p, int seed)
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
