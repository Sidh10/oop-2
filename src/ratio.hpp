// ratio.hpp: builds the ETH/BTC ratio series that the pair strategy trades.
// Syllabus: Unit 1 functions and arrays of objects, Unit 4 throwing user-defined exceptions.

#ifndef RATIO_HPP
#define RATIO_HPP

#include <vector>

#include "candle.hpp"

namespace quant {

// Keeps only the candles whose openTime appears in both series. Throws DataError if none do.
void alignByOpenTime(const std::vector<Candle>& a, const std::vector<Candle>& b,
                     std::vector<Candle>& alignedA, std::vector<Candle>& alignedB);

// Being long this series is roughly long ETH and short BTC, so the backtester can trade it as it is.
std::vector<Candle> makeRatioSeries(const std::vector<Candle>& eth, const std::vector<Candle>& btc);

// Correlation of the two coins' one-bar log returns: a rough check that they move together.
double logReturnCorrelation(const std::vector<Candle>& a, const std::vector<Candle>& b);

}  // namespace quant

#endif
