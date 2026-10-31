// klines.cpp: turns the JSON text from Binance into checked Candle objects.
// Syllabus: Unit 4 multiple catch blocks and rethrowing as a clearer error type.

#include <chrono>
#include <nlohmann/json.hpp>
#include <stdexcept>

#include "errors.hpp"
#include "fetcher.hpp"

namespace quant {

long long currentTimeMs() {
    std::chrono::system_clock::duration sinceStart = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(sinceStart).count();
}

// One JSON row is an array: [0] time, [1] open, [2] high, [3] low, [4] close, [5] volume.
static Candle readRow(const nlohmann::json& row) {
    long long time = row.at(0).get<long long>();
    double open = std::stod(row.at(1).get<std::string>());
    double high = std::stod(row.at(2).get<std::string>());
    double low = std::stod(row.at(3).get<std::string>());
    double close = std::stod(row.at(4).get<std::string>());
    double volume = std::stod(row.at(5).get<std::string>());
    return Candle(time, open, close, high, low, volume);
}

std::vector<Candle> parseKlines(const std::string& json, long long nowMs) {
    std::vector<Candle> candles;
    // Unit 4: multiple catch, the JSON library and std::stod report errors in different types.
    try {
        nlohmann::json rows = nlohmann::json::parse(json);
        if (!rows.is_array()) throw DataError("Binance reply is not a list of candles");
        int count = rows.size();
        for (int i = 0; i < count; i++) {
            long long closeTime = rows.at(i).at(6).get<long long>();
            if (closeTime < nowMs) candles.push_back(readRow(rows.at(i)));  // skip a candle still forming
        }
    } catch (const nlohmann::json::exception& error) {
        throw DataError(std::string("malformed Binance JSON: ") + error.what());
    } catch (const std::logic_error& error) {
        throw DataError(std::string("bad number in Binance JSON: ") + error.what());
    }
    validateCandles(candles);
    return candles;
}

void validateCandles(const std::vector<Candle>& candles) {
    int count = candles.size();
    for (int i = 0; i < count; i++) {
        const Candle& c = candles[i];
        if (i > 0 && c.openTime <= candles[i - 1].openTime) {
            throw DataError("candle times do not increase at " + c.toText());
        }
        if (c.open <= 0 || c.high <= 0 || c.low <= 0 || c.close <= 0) {
            throw DataError("price is not positive at " + c.toText());
        }
        if (c.high < c.low) throw DataError("high is below low at " + c.toText());
    }
}

}  // namespace quant
