#include "unity.cpp"

int main(int argc, char* argv[])
{
  settings Settings = parse_settings(argc, argv);

  if (Settings.print_usage_and_exit)
  {
    cerr << "usage: play_icfp2015 [OPTIONS]\n"
      "	-f	FILENAME	File containing JSON encoded input\n"
      "	-t	NUMBER	Time limit, in seconds, to produce output\n"
      "	-m	NUMBER	Memory limit, in megabytes, to produce output\n"
      "	-c	NUMBER	Number of processor cores available\n"
      "	-p	STRING	Phrase of power"
    << endl;
    return 2;
  }

  run_controller(Settings.files, Settings.phrases, Settings.time_limit, Settings.memory_limit);

  return 0;
}
