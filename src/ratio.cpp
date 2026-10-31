// ratio.cpp: lines up the two coins by time and divides ETH by BTC.
// Syllabus: Unit 1 control structures, Unit 4 throwing user-defined exceptions.

#include "ratio.hpp"

#include <cmath>

#include "errors.hpp"

namespace quant {

void alignByOpenTime(const std::vector<Candle>& a, const std::vector<Candle>& b,
                     std::vector<Candle>& alignedA, std::vector<Candle>& alignedB) {
    // Both lists are sorted by time, so one pass moves whichever side is behind.
    int i = 0;
    int j = 0;
    int countA = a.size();
    int countB = b.size();
    while (i < countA && j < countB) {
        if (a[i].openTime < b[j].openTime) {
            i++;
        } else if (a[i].openTime > b[j].openTime) {
            j++;
        } else {
            alignedA.push_back(a[i]);
            alignedB.push_back(b[j]);
            i++;
            j++;
        }
    }
    if (alignedA.empty()) throw DataError("the two series have no candle times in common");
}

std::vector<Candle> makeRatioSeries(const std::vector<Candle>& eth, const std::vector<Candle>& btc) {
    std::vector<Candle> alignedEth;
    std::vector<Candle> alignedBtc;
    alignByOpenTime(eth, btc, alignedEth, alignedBtc);

    std::vector<Candle> ratio;
    int count = alignedEth.size();
    for (int i = 0; i < count; i++) {
        const Candle& e = alignedEth[i];
        const Candle& b = alignedBtc[i];
        if (b.open == 0 || b.close == 0) throw DataError("a zero BTC price cannot form an ETH/BTC ratio");
        ratio.push_back(Candle(e.openTime, e.open / b.open, e.close / b.close));  // high and low from the defaults
    }
    return ratio;
}

double logReturnCorrelation(const std::vector<Candle>& a, const std::vector<Candle>& b) {
    std::vector<Candle> alignedA;
    std::vector<Candle> alignedB;
    alignByOpenTime(a, b, alignedA, alignedB);

    double sumX = 0, sumY = 0, sumXX = 0, sumYY = 0, sumXY = 0;
    int count = alignedA.size();
    count = count - 1;
    for (int i = 1; i <= count; i++) {
        double x = std::log(alignedA[i].close / alignedA[i - 1].close);
        double y = std::log(alignedB[i].close / alignedB[i - 1].close);
        sumX += x;
        sumY += y;
        sumXX += x * x;
        sumYY += y * y;
        sumXY += x * y;
    }
    double varianceX = sumXX - sumX * sumX / count;
    double varianceY = sumYY - sumY * sumY / count;
    if (count < 2 || varianceX == 0 || varianceY == 0) return 0;
    return (sumXY - sumX * sumY / count) / std::sqrt(varianceX * varianceY);
}

}  // namespace quant
