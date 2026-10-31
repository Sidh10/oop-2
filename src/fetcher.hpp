// fetcher.hpp: downloads candles from Binance and keeps the cache up to date.
// Syllabus: Unit 2 constructor and destructor, Unit 4 exceptions in a constructor.

#ifndef FETCHER_HPP
#define FETCHER_HPP

#include <string>
#include <vector>

#include "candle.hpp"
#include "datastore.hpp"

namespace quant {

long long currentTimeMs();

// Turns one Binance reply into candles, dropping any that are still open at nowMs.
std::vector<Candle> parseKlines(const std::string& json, long long nowMs);

// Throws DataError unless times increase, prices are above 0 and high >= low.
void validateCandles(const std::vector<Candle>& candles);

// The constructor opens the libcurl handle and the destructor closes it,
// so the handle cannot leak even if an exception is thrown while it is in use.
class DataFetcher {
public:
    DataFetcher();  // Unit 4: exception in a constructor, throws NetworkError if libcurl cannot start
    ~DataFetcher() noexcept;  // Unit 4: exception specification, destructors must not throw

    // All closed candles from startMs up to now, over as many pages as it takes.
    std::vector<Candle> fetchRange(const std::string& symbol, long long startMs);

    // The newest closed candles, straight from Binance and never from the cache.
    std::vector<Candle> fetchLatest(const std::string& symbol);

    // Makes the CSV file cover the last days days, downloading only what is missing.
    void ensureHistory(const DataStore& store, const std::string& symbol, int days);

private:
    // Two objects sharing one handle would both close it, so copying is turned off.
    DataFetcher(const DataFetcher&) = delete;
    DataFetcher& operator=(const DataFetcher&) = delete;

    std::string httpGet(const std::string& url);
    std::string getFromAnyBase(const std::string& pathAndQuery);

    // libcurl's CURL type is just void, so a void pointer can hold the handle.
    // Only fetcher.cpp includes curl.h, which also brings in the Windows headers.
    void* handle;  // Unit 2: pointer to an object owned by this class
};

}  // namespace quant

#endif
