
class random_player
{
  mt19937 generator;
public:
  random_player();
  tuple<string, int> solve(problem& p, int seed);
  moved step(game_state& currentState);
};

random_player::random_player()
{
  auto seed = system_clock::now().time_since_epoch().count();
  generator = mt19937(seed);
}

moved
random_player::step(game_state& currentState)
{
  list<moved> actions = GameGetLegalMoves(currentState);
  if (actions.size() == 0)
    return moved::SE;

  uniform_int_distribution<int> distribution(0, actions.size() - 1);
  int randomMove = distribution(generator);

  return vector<moved>(begin(actions), end(actions))[randomMove];
}

tuple<string, int>
random_player::solve(problem& p, int seed)
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
