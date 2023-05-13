#include <iostream>
#include <vector>
#include <memory>
#include "audio.hpp"

// Main function
int main(int argc, char *argv[]) {
  // Checking if the input arguments are as expected
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <input file>" << std::endl;
    return 1;
  }

  // Reading the input file
  std::string inputFile(argv[1]);
  AudioFile audioFile(inputFile);
  audioFile.readInputFile();

  // Generating audio samples
  audioFile.generateAudioSamples();

  // Writing the samples to the output file
  audioFile.writeWavFile(audioFile.samplesBuffer);

  return 0;
}