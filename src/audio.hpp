#ifndef __AUDIO_HPP__
#define __AUDIO_HPP__

#include <string>
#include <fstream>
#include <vector>

// Sampling rate for audio
const int SAMPLE_RATE = 44100;

// Enum for the type of the note
enum NoteType {
  PIANO
};

// Structure to store note information
struct Note {
  double startBeat;
  NoteType type;
  double frequency;
  double duration;
};

// Class for handling audio file processing
class AudioFile {
private:
  std::ifstream inputFileStream;
  std::ofstream outputFileStream;

public:
  double tempo;
  double totalBeats;
  std::vector<Note> notes;
  std::unique_ptr<std::vector<double>> samplesBuffer;

  AudioFile(std::string& filename);
  ~AudioFile();
  bool hasValidExtension(const std::string& filename, const std::string& extension);
  double calculateFrequency(std::string pitch);
  void readInputFile(void);
  void generateAudioSamples(void);
  void writeWavHeader(int numSamples);
  void writeWavFile(const std::unique_ptr<std::vector<double>>& buffer);
};

#endif