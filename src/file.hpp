#ifndef __FILE_HPP__
#define __FILE_HPP__

#include <string>
#include <vector>
#include "note.hpp"

void read_input_file(const std::string& filename, double& tempo, double& total_beats, std::vector<Note>& notes);
void write_wav_file(const std::string& filename, const std::unique_ptr<std::vector<double>>& buffer, double tempo, double total_beats);

#endif