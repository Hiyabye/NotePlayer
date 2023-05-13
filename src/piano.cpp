#include "audio.hpp"
#include "piano.hpp"

// Generates the samples for a piano note with given frequency and amplitude
void generatePianoNoteSample(std::vector<double> &buffer, double frequency, double duration, double startTime, double amplitude) {
  // Define the harmonics for a piano sound
  std::vector<Harmonic> harmonics = {
    {1.00, 1},
    {0.75, 2},
    {0.50, 4},
    {0.14, 8},
    {0.05, 16},
  };

  int numSamples = duration * SAMPLE_RATE;
  double sampleDuration = 1.0 / SAMPLE_RATE;
  int startSample = startTime * SAMPLE_RATE;
  int bufferSize = buffer.size(); // Get the size of the buffer

  for (const auto& harmonic : harmonics) {
    double amplitudeForHarmonic = harmonic.amplitude;
    double frequencyMultiplier = harmonic.frequencyMultiplier;

    for (int i=0; i<numSamples; ++i) {
      int bufferIndex = startSample + i;
      if (bufferIndex >= bufferSize) {
        break; // Stop writing samples if the buffer is full
      }

      double time = i * sampleDuration;
      buffer[bufferIndex] += amplitude * amplitudeForHarmonic * std::sin(2 * M_PI * (frequency * frequencyMultiplier) * time);
    }
  }
}