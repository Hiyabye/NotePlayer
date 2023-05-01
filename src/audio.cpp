#include <cmath>
#include <iostream>
#include <algorithm>
#include <cctype>
#include "audio.hpp"
#include "note.hpp"

// Create base frequencies for each note
std::map<std::string, double> create_base_frequencies() {
  std::map<std::string, double> base_frequencies = {
    {"Cb", 493.88}, {"C0", 523.25}, {"C#", 554.37},
    {"Db", 554.37}, {"D0", 587.33}, {"D#", 622.25},
    {"Eb", 622.25}, {"E0", 659.25}, {"E#", 698.46},
    {"Fb", 659.25}, {"F0", 698.46}, {"F#", 739.99},
    {"Gb", 739.99}, {"G0", 783.99}, {"G#", 830.61},
    {"Ab", 830.61}, {"A0", 880.00}, {"A#", 932.33},
    {"Bb", 932.33}, {"B0", 987.77}, {"B#", 1046.50}
  };
  return base_frequencies;
}

// Calculate frequency for a given note
double calculate_frequency(const std::string &pitch) {
  static const std::map<std::string, double> base_frequencies = create_base_frequencies();

  std::string base_note = pitch.substr(0, 2);
  std::string octave_str = pitch.substr(3);

  if (!std::all_of(octave_str.begin(), octave_str.end(), ::isdigit)) {
    std::cerr << "Invalid octave: " << octave_str << std::endl;
    exit(1);
  }

  int octave = std::stoi(octave_str);

  auto it = base_frequencies.find(base_note);
  if (it != base_frequencies.end()) {
    return it->second * std::pow(2, octave - 5);
  } else {
    std::cerr << "Invalid note: " << pitch << std::endl;
    exit(1);
  }
}

// Generate audio samples for a single note
void generate_note_samples(std::vector<double> &buffer, double frequency, double duration, double start_time, double amplitude) {
  std::vector<Harmonic> harmonics = {
    {1.00, 1},
    {0.75, 2},
    {0.50, 4},
    {0.14, 8},
    {0.05, 16},
  };

  int num_samples = duration * SAMPLE_RATE;
  double sample_duration = 1.0 / SAMPLE_RATE;
  int start_sample = start_time * SAMPLE_RATE;
  int buffer_size = buffer.size(); // Get the size of the buffer

  for (const auto& harmonic : harmonics) {
    double amp = harmonic.amplitude;
    double freq_mult = harmonic.frequency_multiplier;

    for (int i = 0; i < num_samples; ++i) {
      int buffer_index = start_sample + i;
      if (buffer_index >= buffer_size) {
        break; // Stop writing samples if the buffer is full
      }

      double time = i * sample_duration;
      buffer[buffer_index] += amplitude * amp * std::sin(2 * M_PI * (frequency * freq_mult) * time);
    }
  }
}

// Generate audio samples for each note
std::unique_ptr<std::vector<double>> generate_audio_samples(double tempo, double total_beats, const std::vector<Note>& notes) {
  auto buffer = std::make_unique<std::vector<double>>(static_cast<size_t>(total_beats * 60.0 / tempo * SAMPLE_RATE), 0.0);

  // Generate the notes
  for (const auto& note : notes) {
    double freq = calculate_frequency(note.pitch);
    double start_time = note.start_beat * 60.0 / tempo;
    double duration_seconds = note.duration * 60.0 / tempo;
    generate_note_samples(*buffer, freq, duration_seconds, start_time, 0.1);
  }
  std::cout << "Generated " << notes.size() << " notes" << std::endl;

  return buffer;
}