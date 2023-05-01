#ifndef __AUDIO_HPP__
#define __AUDIO_HPP__

#include <vector>
#include <string>
#include <map>
#include "note.hpp"

const int SAMPLE_RATE = 44100;

std::map<std::string, double> create_base_frequencies();
double calculate_frequency(const std::string &pitch);
void generate_note_samples(std::vector<double> &buffer, double frequency, double duration, double start_time, double amplitude);
std::unique_ptr<std::vector<double>> generate_audio_samples(double tempo, double total_beats, const std::vector<Note>& notes);

#endif