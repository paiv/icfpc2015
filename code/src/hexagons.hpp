
#ifndef __paiv__hexagons_hpp
#define __paiv__hexagons_hpp

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <unordered_set>

namespace paiv {

// See http://www.redblobgames.com/grids/hexagons/implementation.html

template <typename Number, int w>
struct _Hex {
  typedef Number value_type;
  Number q, r;
  Number s() const { return -q - r; }

  bool operator == (const _Hex& b) const {
    return r == b.r && q == b.q;
  }

  bool operator != (const _Hex& b) const {
      return r != b.r || q != b.q;
  }
};

typedef _Hex<s16, 1> Hex;
typedef _Hex<s16, 0> HexDifference;
typedef _Hex<double, 1> FractionalHex;
typedef _Hex<double, 0> FractionalHexDifference;

Hex hex_add(Hex a, Hex b) {
  Hex::value_type q = a.q + b.q;
  Hex::value_type r = a.r + b.r;
  return {q,r};
}

Hex hex_subtract(Hex a, Hex b) {
  Hex::value_type q = a.q - b.q;
  Hex::value_type r = a.r - b.r;
  return {q,r};
}

Hex hex_multiply(Hex a, int k) {
  Hex::value_type q = a.q * k;
  Hex::value_type r = a.r * k;
  return {q,r};
}

int hex_length(Hex hex) {
    return int((abs(hex.q) + abs(hex.r) + abs(hex.s())) / 2);
}

int hex_distance(Hex a, Hex b) {
    return hex_length(hex_subtract(a, b));
}

const vector<Hex> hex_directions = {
  {1, 0}, {1, -1}, {0, -1},
  {-1, 0}, {-1, 1}, {0, 1}
};

Hex hex_direction(int direction /* 0 to 5 */) {
  assert (0 <= direction && direction < 6);
  return hex_directions[direction];
}

Hex hex_neighbor(Hex hex, int direction) {
  return hex_add(hex, hex_direction(direction));
}

Hex hex_rotate_cw(Hex hex, int angle /* 0 to 5 */)
{
  assert (0 <= angle && angle < 6);
  for (int i = 0; i < angle; i++)
  {
    Hex::value_type q = -hex.s();
    Hex::value_type r = -hex.q;
    hex = {q,r};
  }
  return hex;
}

Hex hex_rotate_ccw(Hex hex, int angle /* 0 to 5 */)
{
  assert (0 <= angle && angle < 6);
  for (int i = 0; i < angle; i++)
  {
    Hex::value_type q = -hex.r;
    Hex::value_type r = -hex.s();
    hex = {q,r};
  }
  return hex;
}

struct Orientation {
    const double f0, f1, f2, f3;
    const double b0, b1, b2, b3;
    const double start_angle; // in multiples of 60°
    Orientation(double f0_, double f1_, double f2_, double f3_,
                double b0_, double b1_, double b2_, double b3_,
                double start_angle_)
    : f0(f0_), f1(f1_), f2(f2_), f3(f3_),
      b0(b0_), b1(b1_), b2(b2_), b3(b3_),
      start_angle(start_angle_) {}
};

const Orientation layout_pointy
  = Orientation(sqrt(3.0), sqrt(3.0) / 2.0, 0.0, 3.0 / 2.0,
                sqrt(3.0) / 3.0, -1.0 / 3.0, 0.0, 2.0 / 3.0,
                0.5);
const Orientation layout_flat
  = Orientation(3.0 / 2.0, 0.0, sqrt(3.0) / 2.0, sqrt(3.0),
                2.0 / 3.0, 0.0, -1.0 / 3.0, sqrt(3.0) / 3.0,
                0.0);


Hex hex_round(FractionalHex h) {
    Hex::value_type q = round(h.q);
    Hex::value_type r = round(h.r);
    Hex::value_type s = round(h.s());
    FractionalHex::value_type q_diff = abs(q - h.q);
    FractionalHex::value_type r_diff = abs(r - h.r);
    FractionalHex::value_type s_diff = abs(s - h.s());
    if (q_diff > r_diff and q_diff > s_diff) {
        q = -r - s;
    } else if (r_diff > s_diff) {
        r = -q - s;
    // } else {
    //     s = -q - r;
    }
    return {q, r};
}

Hex hex_floor(FractionalHex h) {
    Hex::value_type q = floor(h.q);
    Hex::value_type r = floor(h.r);
    Hex::value_type s = floor(h.s());
    FractionalHex::value_type q_diff = abs(q - h.q);
    FractionalHex::value_type r_diff = abs(r - h.r);
    FractionalHex::value_type s_diff = abs(s - h.s());
    if (q_diff > r_diff and q_diff > s_diff) {
        q = -r - s;
    } else if (r_diff > s_diff) {
        r = -q - s;
    // } else {
    //     s = -q - r;
    }
    return {q, r};
}

FractionalHex hex_lerp(Hex a, Hex b, double t) {
  FractionalHex::value_type q = a.q + (b.q - a.q) * t;
  FractionalHex::value_type r = a.r + (b.r - a.r) * t;
  return {q,r};
}

vector<Hex> hex_linedraw(Hex a, Hex b) {
    int N = hex_distance(a, b);
    vector<Hex> results = {};
    double step = 1.0 / max(N, 1);
    for (int i = 0; i <= N; i++) {
        results.push_back(hex_round(hex_lerp(a, b, step * i)));
    }
    return results;
}

template<class T> class RectangularPointyTopMap {
    vector<vector<T>> map;

  public:
    RectangularPointyTopMap(int width, int height): map(height) {
        for (int r = 0; r < height; r++) {
            map.emplace_back(width);
        }
    }

    inline T& at(int q, int r) {
        return map[r][q + (r >> 1)];
    }
};

struct OffsetCoord {
  typedef Hex::value_type value_type;
  value_type col, row;
};

const s8 EVEN = 1;
const s8 ODD = -1;

OffsetCoord qoffset_from_cube(int offset, Hex h) {
    OffsetCoord::value_type col = h.q;
    OffsetCoord::value_type row = h.r + Hex::value_type((h.q + offset * (h.q & 1)) / 2);
    return {col, row};
}

Hex qoffset_to_cube(int offset, OffsetCoord h) {
    Hex::value_type q = h.col;
    Hex::value_type r = h.row - Hex::value_type((h.col + offset * (h.col & 1)) / 2);
    return {q, r};
}

OffsetCoord roffset_from_cube(int offset, Hex h) {
    OffsetCoord::value_type col = h.q + Hex::value_type((h.r + offset * (h.r & 1)) / 2);
    OffsetCoord::value_type row = h.r;
    return {col, row};
}

Hex roffset_to_cube(int offset, OffsetCoord h) {
    Hex::value_type q = h.col - Hex::value_type((h.row + offset * (h.row & 1)) / 2);
    Hex::value_type r = h.row;
    return {q, r};
}

}

namespace std {
    template <> struct hash<paiv::Hex> {
        size_t operator()(const paiv::Hex& h) const {
            hash<int> int_hash;
            size_t hq = int_hash(h.q);
            size_t hr = int_hash(h.r);
            return hq ^ (hr + 0x9e3779b9 + (hq << 6) + (hq >> 2));
        }
    };
}

#endif
