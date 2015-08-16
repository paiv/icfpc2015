

static void
run_controller(list<string>& files, list<string>& phrases, int time_limit, int memory_limit)
{
  Json::Value result(Json::arrayValue);

  for (string fn : files)
  {
    problem p = read_problem(fn);
    list<solution> solved = solve_problem(p);
    for (solution& r : solved)
      result.append(toJson(r));
  }

  cout << result << endl;
}
