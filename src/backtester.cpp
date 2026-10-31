// backtester.cpp: the trading loop, the fills, and total return, drawdown and Sharpe.
// Syllabus: Unit 1 encapsulation, Unit 3 runtime polymorphism through Strategy&.

#include "backtester.hpp"

#include <cmath>

#include "errors.hpp"

namespace quant {

static double maxDrawdownOf(const std::vector<double>& equity) {
    double peak = equity[0];
    double worst = 0;
    int count = equity.size();
    for (int k = 0; k < count; k++) {
        if (equity[k] > peak) peak = equity[k];
        double drawdown = (peak - equity[k]) / peak;
        if (drawdown > worst) worst = drawdown;
    }
    return worst;
}

static double sharpeOf(const std::vector<double>& equity, double barsPerYear) {
    std::vector<double> returns;
    int barCount = equity.size();
    for (int k = 1; k < barCount; k++) {
        returns.push_back(equity[k] / equity[k - 1] - 1);
    }
    int count = returns.size();
    if (count < 2) return 0;  // a sample deviation needs two values

    double sum = 0;
    for (int k = 0; k < count; k++) {
        sum += returns[k];
    }
    double average = sum / count;

    double squares = 0;
    for (int k = 0; k < count; k++) {
        squares += (returns[k] - average) * (returns[k] - average);
    }
    double deviation = std::sqrt(squares / (count - 1));
    if (deviation == 0) return 0;
    return average / deviation * std::sqrt(barsPerYear);
}

Metrics computeMetrics(const std::vector<double>& equity, double barsPerYear) {
    Metrics metrics;
    metrics.totalReturn = 0;
    metrics.maxDrawdown = 0;
    metrics.sharpe = 0;
    if (equity.empty()) return metrics;

    metrics.totalReturn = equity.back() / equity.front() - 1;
    metrics.maxDrawdown = maxDrawdownOf(equity);
    metrics.sharpe = sharpeOf(equity, barsPerYear);
    return metrics;
}

BacktestResult::BacktestResult(double totalReturn, double maxDrawdown, double sharpe, int trades,
                               const std::vector<double>& equity) {
    totalReturn_ = totalReturn;
    maxDrawdown_ = maxDrawdown;
    sharpe_ = sharpe;
    trades_ = trades;
    equity_ = equity;
}

Backtester::Backtester(double cash, double fee, double bars) {
    startCash = cash;
    costPerSide = fee;
    barsPerYear = bars;
}

// Buys with all the cash, or sells all the units, at this bar's open.
static void fill(Position target, double open, double cost, double& cash, double& units) {
    if (target == LONG) {
        units = cash * (1 - cost) / open;
        cash = 0;
    } else {
        cash = units * open * (1 - cost);
        units = 0;
    }
}

BacktestResult Backtester::run(Strategy& strategy, const std::vector<Candle>& bars, int begin, int end) const {
    int barCount = bars.size();
    if (begin < 0 || end > barCount || begin >= end) throw DataError("backtest range is outside the data");

    strategy.reset();
    strategy.warmUp(bars, begin);

    // Unit 1: encapsulation, the account exists only inside this function.
    double cash = startCash;
    double units = 0;
    Position held = FLAT;
    Position pending = FLAT;
    std::vector<double> equity;
    int trades = 0;

    for (int t = begin; t < end; t++) {
        if (pending != held) {
            fill(pending, bars[t].open, costPerSide, cash, units);
            held = pending;
            trades++;
        }
        equity.push_back(cash + units * bars[t].close);

        // Unit 3: runtime polymorphism, the virtual call reaches whichever strategy this really is.
        // A decision at the close of t fills at the open of t+1, so one on the last bar never fills.
        if (t < end - 1) pending = strategy.signal(bars, t);
    }

    Metrics metrics = computeMetrics(equity, barsPerYear);
    return BacktestResult(metrics.totalReturn, metrics.maxDrawdown, metrics.sharpe, trades, equity);
}

}  // namespace quant
