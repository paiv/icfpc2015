
static void
copy_ints(const Json::Value& v, list<int>& t)
{
  for (int i = 0; i < v.size(); i++)
    t.push_back(v[i].asInt());
}

static cell
read_cell(const Json::Value& v)
{
  s16 x = v.get("x", 0).asInt();
  s16 y = v.get("y", 0).asInt();
  return roffset_to_cube(0, {x,y});
}

static void
copy_cells(const Json::Value& v, vector<cell>& t)
{
  for (int i = 0; i < v.size(); i++)
    t.push_back(read_cell(v[i]));
}

static unit
read_unit(const Json::Value& v)
{
  unit u;
  u.pivot = read_cell(v.get("pivot", 0));
  copy_cells(v.get("members", 0), u.members);
  return u;
}

static void
copy_units(const Json::Value& v, vector<unit>& t)
{
  for (int i = 0; i < v.size(); i++)
    t.push_back(read_unit(v[i]));
}

static problem
read_problem(string& fn)
{
  problem p;
  Json::Value json;

  ifstream fin(fn, ifstream::binary);
  fin >> json;

  p.problem_id = json.get("id", 0).asInt();
  p.width = json.get("width", 0).asInt();
  p.height = json.get("height", 0).asInt();
  p.source_length = json.get("sourceLength", 0).asInt();
  copy_units(json.get("units", Json::Value(Json::arrayValue)), p.units);
  copy_cells(json.get("filled", Json::Value(Json::arrayValue)), p.filled);
  copy_ints(json.get("sourceSeeds", Json::Value(Json::arrayValue)), p.source_seeds);

  return p;
}
