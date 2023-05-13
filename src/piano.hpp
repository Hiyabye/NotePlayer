#ifndef PIANO_HPP
#define PIANO_HPP

#include <vector>

// Harmonic structure contains information about amplitude and frequency multiplier for the harmonics
struct Harmonic {
  double amplitude;
  double frequencyMultiplier;
};

// Generates the samples for a piano note with given frequency and amplitude
void generatePianoNoteSample(std::vector<double> &buffer, double frequency, double duration, double startTime, double amplitude);

#endif