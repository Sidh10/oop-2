// strategies.cpp: the signals from BuyAndHold, SmaCrossover and PairsSpread.
// Syllabus: Unit 3 overriding virtual functions and using a protected member.

#include "indicators.hpp"
#include "strategy.hpp"

namespace quant {

std::vector<double> BuyAndHold::params() const {
    return std::vector<double>();
}

Position BuyAndHold::signal(const std::vector<Candle>&, int) {
    position = LONG;
    return position;
}

SmaCrossover::SmaCrossover(int fastLength, int slowLength) {
    fast = fastLength;
    slow = slowLength;
}

std::vector<double> SmaCrossover::params() const {
    std::vector<double> values;
    values.push_back(fast);
    values.push_back(slow);
    return values;
}

Position SmaCrossover::signal(const std::vector<Candle>& bars, int i) {
    if (i + 1 < slow) return FLAT;
    std::vector<double> closes = indicators::lastCloses(bars, i, slow);
    int last = slow - 1;
    if (indicators::sma(closes, last, fast) > indicators::sma(closes, last, slow)) {
        position = LONG;
    } else {
        position = FLAT;
    }
    return position;
}

PairsSpread::PairsSpread(int windowLength, double entryLevel) {
    window = windowLength;
    entryZ = entryLevel;
}

std::vector<double> PairsSpread::params() const {
    std::vector<double> values;
    values.push_back(window);
    values.push_back(entryZ);
    return values;
}

Position PairsSpread::signal(const std::vector<Candle>& bars, int i) {
    if (i + 1 < window) return FLAT;
    std::vector<double> closes = indicators::lastCloses(bars, i, window);
    double z = indicators::zscore(closes, window - 1, window);
    if (position == FLAT && z < -entryZ) {
        position = LONG;
    } else if (position == LONG && z >= 0) {
        position = FLAT;
    }
    return position;
}

}  // namespace quant
