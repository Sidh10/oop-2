// commands.hpp: the actions behind the menu items and the command-line options.
// Syllabus: Unit 1 functions and message passing.

#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>

#include "datastore.hpp"

namespace quant {

const std::string bestParamsPath = "reports/best_params.csv";

// Reads one whole line. Returns -1 unless it is a single number from 0 to max.
int askNumber(const std::string& prompt, int max);

void downloadData(const DataStore& store);
void showSummary(const DataStore& store);
void recommend(const DataStore& store);
void backtestOne(const DataStore& store);
void liveSignals();  // this one is in live.cpp

}  // namespace quant

#endif
