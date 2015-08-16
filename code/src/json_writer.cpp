
static Json::Value
toJson(solution& r)
{
  Json::Value v(Json::objectValue);

  v["problemId"] = r.problem_id;
  v["seed"] = r.seed;
  if (r.tag.size() > 0)
    v["tag"] = r.tag;
  v["solution"] = r.commands;

  return v;
}
