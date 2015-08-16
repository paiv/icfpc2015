
class lcg_random
{
  unsigned int seed;

public:
  lcg_random() : seed(0) {}
  lcg_random(unsigned int seed) : seed(seed) {}

  // 0 - 0x7FFF
  int next()
  {
    unsigned int t = (seed >> 16) & 0x7FFF;
    seed = 1103515245 * seed + 12345;
    return t;
  }
};
