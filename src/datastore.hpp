// datastore.hpp: keeps the downloaded candles in one CSV file per coin.
// Syllabus: Unit 1 classes and information hiding, Unit 5 primitive data processing.

#ifndef DATASTORE_HPP
#define DATASTORE_HPP

#include <string>
#include <vector>

#include "candle.hpp"

namespace quant {

class DataStore {
public:
    DataStore(const std::string& folderName = "data");

    std::string pathFor(const std::string& symbol) const;
    bool hasFile(const std::string& symbol) const;
    void save(const std::string& symbol, const std::vector<Candle>& candles) const;
    void append(const std::string& symbol, const std::vector<Candle>& candles) const;
    std::vector<Candle> load(const std::string& symbol) const;

private:
    std::string folder;
};

// True when the cache is missing (firstOpenTime below 0) or starts more than one day
// too late for the number of days asked for.
bool needsFullRefetch(long long firstOpenTime, long long nowMs, int days);

}  // namespace quant

#endif
