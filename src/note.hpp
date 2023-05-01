#ifndef __NOTE_HPP__
#define __NOTE_HPP__

#include <string>

// Note structure to store the note properties
struct Note {
  double start_beat;
  std::string pitch;
  double duration;
};

struct Harmonic {
  double amplitude;
  double frequency_multiplier;
};

#endif