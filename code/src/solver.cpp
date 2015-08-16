
static list<moved>
solve_paths(problem& p, int seed, list<placement>& placements)
{
  list<moved> result;
  return result;
}

static solution
solve_seed(problem& p, int seed)
{
  mc_player player;
  tuple<string, int> result = player.solve(p, seed);

  solution r;
  r.problem_id = p.problem_id;
  r.seed = seed;
  r.commands = get<0>(result);
  r.score = get<1>(result);

  return r;
}

static list<solution>
solve_problem(problem& p)
{
  list<solution> results;

  for (int seed : p.source_seeds)
  {
    solution r = solve_seed(p, seed);
    results.push_back(r);
  }

  return results;
}
