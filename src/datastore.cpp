// datastore.cpp: saves candles to CSV files and loads them back.
// Syllabus: Unit 5 disk file I/O, member-function file I/O, stream errors, primitive data.

#include "datastore.hpp"

#include <fstream>

#include "errors.hpp"

namespace quant {

static const long long msPerDay = 86400000LL;

static void writeCandles(std::ofstream& out, const std::string& path, const std::vector<Candle>& candles) {
    int count = candles.size();
    for (int i = 0; i < count; i++) {
        out << candles[i] << '\n';
    }
    out.close();
    if (out.fail()) throw DataError("could not write " + path);  // Unit 5: stream errors
}

DataStore::DataStore(const std::string& folderName) {
    folder = folderName;
}

std::string DataStore::pathFor(const std::string& symbol) const {
    return folder + "/" + symbol + "_1h.csv";
}

bool DataStore::hasFile(const std::string& symbol) const {
    std::ifstream in(pathFor(symbol));
    return in.is_open();
}

// Binary mode keeps every line ending as a plain \n on Windows too.
void DataStore::save(const std::string& symbol, const std::vector<Candle>& candles) const {
    std::string path = pathFor(symbol);
    std::ofstream out;  // Unit 5: disk file I/O with member functions
    out.open(path, std::ios::binary);
    if (!out.is_open()) throw DataError("cannot create " + path);
    out << "openTime,open,high,low,close,volume\n";
    writeCandles(out, path, candles);
}

void DataStore::append(const std::string& symbol, const std::vector<Candle>& candles) const {
    std::string path = pathFor(symbol);
    std::ofstream out;
    out.open(path, std::ios::binary | std::ios::app);
    if (!out.is_open()) throw DataError("cannot append to " + path);
    writeCandles(out, path, candles);
}

std::vector<Candle> DataStore::load(const std::string& symbol) const {
    std::string path = pathFor(symbol);
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) throw DataError("cannot open " + path + ", download the data first");

    std::string header;
    std::getline(in, header);

    std::vector<Candle> candles;
    Candle candle;
    while (in >> candle) {
        candles.push_back(candle);
    }

    // Unit 5: error handling in file I/O, the loop must have stopped at the end of the file.
    if (in.bad()) throw DataError("read error in " + path);
    if (!in.eof()) throw DataError("bad line in " + path);
    return candles;
}

bool needsFullRefetch(long long firstOpenTime, long long nowMs, int days) {
    if (firstOpenTime < 0) return true;
    long long wantedStart = nowMs - days * msPerDay;
    return firstOpenTime > wantedStart + msPerDay;
}

}  // namespace quant
