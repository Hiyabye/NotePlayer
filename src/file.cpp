#include <iostream>
#include <fstream>
#include <cstdint>
#include "file.hpp"
#include "note.hpp"
#include "audio.hpp"

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
  std::vector<double>::size_type num_samples = static_cast<std::vector<double>::size_type>(total_beats * 60.0 / tempo * SAMPLE_RATE);

  if (buffer->size() < num_samples) {
    std::cerr << "Error: Buffer size is smaller than expected" << std::endl;
    exit(1);
  }

  std::ofstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Error: Unable to open output file \"" << filename << "\"" << std::endl;
    exit(1);
  }

  write_wav_header(file, static_cast<int>(num_samples));

  for (std::vector<double>::size_type i = 0; i < num_samples; i++) {
    double clipped_value = std::min(1.0, std::max(-1.0, (*buffer)[i])); // Clip the value between -1 and 1
    int16_t sample_int = static_cast<int16_t>(clipped_value * 32767);
    file.write(reinterpret_cast<char*>(&sample_int), sizeof(sample_int));
    if (!file) {
      std::cerr << "Error: Unable to write data to output file \"" << filename << "\"" << std::endl;
      exit(1);
    }
  }

  file.close();
  if (!file) {
    std::cerr << "Error: Unable to close output file \"" << filename << "\"" << std::endl;
    exit(1);
  }

  std::cout << "Wrote " << filename << std::endl;
}