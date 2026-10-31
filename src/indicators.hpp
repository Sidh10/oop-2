// indicators.hpp: the moving average and z-score used by the strategies.
// Syllabus: Unit 1 namespaces and control structures, Unit 2 inline functions.

#ifndef INDICATORS_HPP
#define INDICATORS_HPP

#include <cmath>
#include <vector>

#include "candle.hpp"

namespace quant {

// Unit 1: namespaces, a nested namespace keeps these short names apart.
namespace indicators {

// Both indicators read only indexes up to i, so they cannot see the future.

inline double sma(const std::vector<double>& closes, int i, int n) {
    double sum = 0;
    for (int k = i - n + 1; k <= i; k++) {
        sum += closes[k];
    }
    return sum / n;
}

inline double zscore(const std::vector<double>& values, int i, int window) {
    double average = sma(values, i, window);
    double squares = 0;
    for (int k = i - window + 1; k <= i; k++) {
        squares += (values[k] - average) * (values[k] - average);
    }
    double deviation = std::sqrt(squares / (window - 1));
    if (deviation == 0) return 0;
    return (values[i] - average) / deviation;
}

// The closes of the n bars ending at i. Strategies copy only these on each bar,
// because copying the whole history every bar would be very slow.
inline std::vector<double> lastCloses(const std::vector<Candle>& bars, int i, int n) {
    std::vector<double> closes;
    for (int k = i - n + 1; k <= i; k++) {
        closes.push_back(bars[k].close);
    }
    return closes;
}

}  // namespace indicators
}  // namespace quant

#endif
