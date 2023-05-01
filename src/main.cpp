#include <iostream>
#include <vector>
#include <memory>
#include "note.hpp"
#include "audio.hpp"
#include "file.hpp"

bool has_extension(const std::string& filename, const std::string& ext) {
  return filename.length() >= ext.length() &&
         std::equal(ext.rbegin(), ext.rend(), filename.rbegin(), [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}

int main(int argc, char *argv[]) {
  // Check for the correct number of arguments
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <input file>" << std::endl;
    return 1;
  }

  // Check if the input file has the correct extension
  if (!has_extension(argv[1], ".txt")) {
    std::cerr << "Error: Input file must have a .txt extension" << std::endl;
    return 1;
  }

  // Read the input file
  double tempo;
  double total_beats;
  std::vector<Note> notes;
  read_input_file(argv[1], tempo, total_beats, notes);

  // Generate audio samples
  std::unique_ptr<std::vector<double>> buffer = generate_audio_samples(tempo, total_beats, notes);

  // Write the samples to the output file
  std::string wav_file = argv[1];
  write_wav_file(wav_file.replace(wav_file.end() - 3, wav_file.end(), "wav"), buffer, tempo, total_beats);

  return 0;
}