// backtester.hpp: plays a strategy through the bars and measures how it did.
// Syllabus: Unit 1 information hiding, Unit 2 constructor with default arguments.

#ifndef BACKTESTER_HPP
#define BACKTESTER_HPP

#include <vector>

#include "candle.hpp"
#include "strategy.hpp"

namespace quant {

struct Metrics {
    double totalReturn;
    double maxDrawdown;
    double sharpe;
};

// All the metric maths in one function, so the tests can call it directly.
Metrics computeMetrics(const std::vector<double>& equity, double barsPerYear);

// Unit 1: information hiding, private fields read through public getters.
class BacktestResult {
public:
    BacktestResult(double totalReturn, double maxDrawdown, double sharpe, int trades,
                   const std::vector<double>& equity);

    double totalReturn() const { return totalReturn_; }
    double maxDrawdown() const { return maxDrawdown_; }
    double sharpe() const { return sharpe_; }
    int trades() const { return trades_; }
    const std::vector<double>& equity() const { return equity_; }

private:
    double totalReturn_;
    double maxDrawdown_;
    double sharpe_;
    int trades_;
    std::vector<double> equity_;
};

class Backtester {
public:
    // Unit 2: constructor with default arguments.
    Backtester(double cash = 10000, double fee = 0.0012, double bars = 8760);

    BacktestResult run(Strategy& strategy, const std::vector<Candle>& bars, int begin, int end) const;

private:
    double startCash;
    double costPerSide;
    double barsPerYear;
};

}  // namespace quant

#endif
