#include <array>
#include <chrono>
#include <iostream>
#include <fstream>
#include <list>
#include <queue>
#include <random>
#include <tuple>
#include <unordered_map>
#include <unistd.h>
#include "json/json.h"

using namespace std;
using namespace std::chrono;

#include "types_primitive.hpp"
#include "lcg_random.hpp"
#include "point.hpp"
#include "hexagons.hpp"
#include "types.hpp"
#include "astar.hpp"

using namespace paiv;

#include "settings_parser.cpp"
#include "json_reader.cpp"
#include "json_writer.cpp"
#include "game.cpp"
#include "path_finder.cpp"
#include "solve_phrases.cpp"
#include "random_player.cpp"
#include "placement_player.cpp"
#include "mc_player.cpp"
#include "solver.cpp"
#include "controller.cpp"
