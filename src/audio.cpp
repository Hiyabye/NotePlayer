#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <map>
#include "audio.hpp"
#include "piano.hpp"

// Constructor
AudioFile::AudioFile(std::string& filename) {
  // Checking if the file has the correct extension
  if (!hasValidExtension(filename, ".txt")) {
    std::cerr << "Error: Input file must have a .txt extension" << std::endl;
    exit(1);
  }
  
  // Opening the input file
  inputFileStream.open(filename);
  if (!inputFileStream) {
    std::cerr << "Unable to open input file: " << filename << std::endl;
    exit(1);
  }

  // Opening the output file
  outputFileStream.open(filename.replace(filename.end()-3, filename.end(), "wav"), std::ios::binary);
  if (!outputFileStream) {
    std::cerr << "Unable to open output file: " << filename << std::endl;
    exit(1);
  }
}

// Destructor
AudioFile::~AudioFile() {
  inputFileStream.close();
  outputFileStream.close();
}

// Checks if the filename has the correct extension
bool AudioFile::hasValidExtension(const std::string& filename, const std::string& extension) {
  return filename.size() >= extension.size() && filename.compare(filename.size()-extension.size(), extension.size(), extension) == 0;
}

// Calculates the frequency of a note
double AudioFile::calculateFrequency(std::string pitch) {
  // Base frequencies for different pitches
  std::map<std::string, double> baseFrequencies = {
    {"Cb", 493.88}, {"C0", 523.25}, {"C#", 554.37},
    {"Db", 554.37}, {"D0", 587.33}, {"D#", 622.25},
    {"Eb", 622.25}, {"E0", 659.25}, {"E#", 698.46},
    {"Fb", 659.25}, {"F0", 698.46}, {"F#", 739.99},
    {"Gb", 739.99}, {"G0", 783.99}, {"G#", 830.61},
    {"Ab", 830.61}, {"A0", 880.00}, {"A#", 932.33},
    {"Bb", 932.33}, {"B0", 987.77}, {"B#", 1046.50}
  };

  std::string baseNote = pitch.substr(0, 2);
  std::string octaveString = pitch.substr(3);

  if (!std::all_of(octaveString.begin(), octaveString.end(), ::isdigit)) {
    std::cerr << "Invalid octave: " << octaveString << std::endl;
    exit(1);
  }

  int octave = std::stoi(octaveString);

  auto it = baseFrequencies.find(baseNote);
  if (it != baseFrequencies.end()) {
    return it->second * pow(2, octave-5);
  } else {
    std::cerr << "Invalid pitch: " << pitch << std::endl;
    exit(1);
  }
}

// Reads the input file
void AudioFile::readInputFile(void) {
  std::string line, pitch;
  std::getline(inputFileStream, line); // First line is tempo (bpm)
  tempo = std::stod(line);
  std::getline(inputFileStream, line); // Second line is total beats
  totalBeats = std::stod(line);
  this->samplesBuffer = std::make_unique<std::vector<double>>(static_cast<size_t>(totalBeats * 60.0 / tempo * SAMPLE_RATE), 0.0);

  // Reading the notes
  while (inputFileStream.good()) {
    Note note;
    inputFileStream >> note.startBeat;
    inputFileStream >> pitch;
    note.frequency = calculateFrequency(pitch);
    inputFileStream >> note.duration;
    std::getline(inputFileStream, line);
    note.type = PIANO;

    notes.push_back(note);
  }
}

// Generates the audio samples for each note
void AudioFile::generateAudioSamples(void) {
  for (const auto& note : notes) {
    double startTime = note.startBeat * 60.0 / tempo;
    double durationSeconds = note.duration * 60.0 / tempo;
    generatePianoNoteSample(*this->samplesBuffer, note.frequency, durationSeconds, startTime, 0.1);
  }
}

// Writes the WAV header to the output file
void AudioFile::writeWavHeader(int numSamples) {
  int chunkSize = numSamples * 2 + 36;
  int byteRate = SAMPLE_RATE * 2;
  int subchunk2Size = numSamples * 2;

  outputFileStream << "RIFF";
  outputFileStream.write(reinterpret_cast<const char *>(&chunkSize), 4);
  outputFileStream << "WAVEfmt ";
  outputFileStream.write("\x10\x00\x00\x00", 4);          // Subchunk1Size (16 for PCM)
  outputFileStream.write("\x01\x00", 2);                  // AudioFormat (1 for PCM)
  outputFileStream.write("\x01\x00", 2);                  // NumChannels (1 for mono)
  outputFileStream.write(reinterpret_cast<const char *>(&SAMPLE_RATE), 4); // SampleRate
  outputFileStream.write(reinterpret_cast<const char *>(&byteRate), 4); // ByteRate
  outputFileStream.write("\x02\x00", 2);                  // BlockAlign (NumChannels * BitsPerSample / 8)
  outputFileStream.write("\x10\x00", 2);                  // BitsPerSample (16 for PCM)
  outputFileStream << "data";
  outputFileStream.write(reinterpret_cast<const char *>(&subchunk2Size), 4); // Subchunk2Size
}

// Writes the audio samples to the output file
void AudioFile::writeWavFile(const std::unique_ptr<std::vector<double>>& buffer) {
  std::vector<double>::size_type numSamples = static_cast<std::vector<double>::size_type>(totalBeats * 60.0 / tempo * SAMPLE_RATE);

  if (buffer->size() < numSamples) {
    std::cerr << "Error: Buffer size is smaller than expected" << std::endl;
    exit(1);
  }

  writeWavHeader(static_cast<int>(numSamples));

  for (std::vector<double>::size_type i=0; i<numSamples; i++) {
    double clippedValue = std::min(1.0, std::max(-1.0, (*buffer)[i])); // Clip values to [-1, 1]
    int16_t sample = static_cast<int16_t>(clippedValue * 32767); // Convert to 16-bit integer
    outputFileStream.write(reinterpret_cast<const char *>(&sample), sizeof(sample));
    if (!outputFileStream.good()) {
      std::cerr << "Error: Unable to write data to output file" << std::endl;
      exit(1);
    }
  }

  std::cout << "Wrote " << numSamples << " samples to output file" << std::endl;
}