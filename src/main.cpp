#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <map>
#include <algorithm>
#include <memory>

const int SAMPLE_RATE = 44100;

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

// Read input file to parse tempo, total_beats, and notes
void read_input_file(const std::string& filename, double& tempo, double& total_beats, std::vector<Note>& notes) {
  std::ifstream input(filename);
  if (!input) {
    std::cerr << "Unable to open input file: " << filename << std::endl;
    exit(1);
  }

  std::string line;
  std::getline(input, line);  // First line contains tempo measured in beats per minute
  tempo = std::stod(line);
  std::getline(input, line);  // Second line contains total time of song in beats
  total_beats = std::stod(line);

  while (input.good()) {
    Note note;
    input >> note.start_beat;
    input >> note.pitch;
    input >> note.duration;
    std::getline(input, line);

    notes.push_back(note);
  }
  input.close();
}

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
  int octave = std::atoi(&pitch[3]);

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

// Write the WAV header to the file
void write_wav_header(std::ofstream &file, int num_samples) {
  int chunk_size = num_samples * 2 + 36;
  int byte_rate = SAMPLE_RATE * 2;
  int subchunk2_size = num_samples * 2;

  file << "RIFF";
  file.write(reinterpret_cast<const char *>(&chunk_size), 4);
  file << "WAVEfmt ";
  file.write("\x10\x00\x00\x00", 4);          // Subchunk1Size (16 for PCM)
  file.write("\x01\x00", 2);                  // AudioFormat (1 for PCM)
  file.write("\x01\x00", 2);                  // NumChannels (1 for mono)
  file.write(reinterpret_cast<const char *>(&SAMPLE_RATE), 4); // SampleRate
  file.write(reinterpret_cast<const char *>(&byte_rate), 4); // ByteRate
  file.write("\x02\x00", 2);                  // BlockAlign (NumChannels * BitsPerSample / 8)
  file.write("\x10\x00", 2);                  // BitsPerSample (16 for PCM)
  file << "data";
  file.write(reinterpret_cast<const char *>(&subchunk2_size), 4); // Subchunk2Size
}

// Write audio samples to the WAV file
void write_wav_file(const std::string& filename, const std::unique_ptr<std::vector<double>>& buffer, double tempo, double total_beats) {
  std::ofstream file(filename, std::ios::binary);
  int num_samples = static_cast<int>(total_beats * 60.0 / tempo * SAMPLE_RATE);
  write_wav_header(file, num_samples);
  for (int i = 0; i < num_samples; i++) {
    double clipped_value = std::min(1.0, std::max(-1.0, (*buffer)[i])); // Clip the value between -1 and 1
    int16_t sample_int = static_cast<int16_t>(clipped_value * 32767);
    file.write(reinterpret_cast<char*>(&sample_int), sizeof(sample_int));
  }
  file.close();
  std::cout << "Wrote " << filename << std::endl;
}

int main(int argc, char *argv[]) {
  // Check for the correct number of arguments
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <input file>" << std::endl;
    return 1;
  }

  // Read the input file
  double tempo;
  double total_beats;
  std::vector<Note> notes;
  read_input_file(argv[1], tempo, total_beats, notes);

  // Generate audio samples
  auto buffer = generate_audio_samples(tempo, total_beats, notes);

  // Write the samples to the output file
  std::string wav_file = argv[1];
  write_wav_file(wav_file.replace(wav_file.end() - 3, wav_file.end(), "wav"), buffer, tempo, total_beats);

  return 0;
}