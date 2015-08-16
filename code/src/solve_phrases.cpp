
static array<char,6>
encodings(moved mv)
{
  static array<char,6> encodings[] = {
    {'p', '\'', '!', '.', '0', '3'}, // move W
    {'b', 'c', 'e', 'f', 'y', '2'},	// move E
    {'a', 'g', 'h', 'i', 'j', '4'},	// move SW
    {'l', 'm', 'n', 'o', ' ', '5'}, // move SE
    {'d', 'q', 'r', 'v', 'z', '1'},	// rotate clockwise
    {'k', 's', 't', 'u', 'w', 'x'},	// rotate counter-clockwise
    //\t, \n, \r	(ignored)
  };

  switch (mv)
  {
    case moved::W:   return encodings[0];
    case moved::E:   return encodings[1];
    case moved::SW:  return encodings[2];
    case moved::SE:  return encodings[3];
    case moved::CW:  return encodings[4];
    case moved::CCW: return encodings[5];
  }
}

static string
solve_phrases(list<moved>& moves)
{
  char buf[moves.size()];
  int i = 0;
  for (moved m : moves)
  {
    array<char,6> enc = encodings(m);
    buf[i++] = enc[0];
  }

  string r(buf, moves.size());
  return r;
}
