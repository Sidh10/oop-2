// advisor.cpp: the train and test split, choosing settings, and the verdict for each row.
// Syllabus: Unit 1 message passing, Unit 2 new and delete through a base class pointer.

#include "advisor.hpp"

#include <cmath>

#include "ratio.hpp"

namespace quant {

// The highest train Sharpe wins. On a tie the first combination stays.
static std::vector<double> bestOnTrain(const Backtester& backtester, const std::string& family,
                                       const std::vector<Candle>& bars, int split, double& bestSharpe) {
    std::vector<std::vector<double>> grid = paramGrid(family);
    std::vector<double> best;
    int count = grid.size();
    for (int g = 0; g < count; g++) {
        Strategy* strategy = makeStrategy(family, grid[g]);
        double sharpe = backtester.run(*strategy, bars, 0, split).sharpe();
        delete strategy;
        if (g == 0 || sharpe > bestSharpe) {
            best = grid[g];
            bestSharpe = sharpe;
        }
    }
    return best;
}

// A row holding only the test-window numbers. As it stands, that is a baseline row.
static ReportRow rowFromTest(const std::string& asset, const std::string& family,
                             const BacktestResult& test, int testBars) {
    ReportRow row;
    row.asset = asset;
    row.family = family;
    row.trainSharpe = 0;
    row.testReturn = test.totalReturn();
    row.testSharpe = test.sharpe();
    row.testDrawdown = test.maxDrawdown();
    row.testTrades = test.trades();
    row.buyHoldReturn = test.totalReturn();
    row.verdict = Verdict::Baseline;
    row.testBars = testBars;
    return row;
}

Backtester backtesterFor(const std::string& asset) {
    if (asset == pairAsset) return Backtester(10000, 0.0024);
    return Backtester(10000, 0.0012);
}

int splitIndex(int barCount) {
    return std::floor(0.7 * barCount);
}

Verdict verdictFor(int testTrades, double testSharpe, double testReturn, double buyHoldReturn) {
    if (testTrades < 10) return Verdict::Insufficient;
    if (testSharpe >= 0.5 && testReturn > buyHoldReturn && testReturn > 0) return Verdict::Recommended;
    if (testReturn > 0) return Verdict::Weak;
    return Verdict::Reject;
}

ReportRow Advisor::evaluate(const std::string& asset, const std::string& family,
                            const std::vector<Candle>& bars) const {
    int barCount = bars.size();
    int split = splitIndex(barCount);
    Backtester backtester = backtesterFor(asset);

    double trainSharpe = 0;
    std::vector<double> params = bestOnTrain(backtester, family, bars, split, trainSharpe);

    // Unit 1: message passing, the Advisor asks the Backtester, which asks the Strategy.
    Strategy* chosen = makeStrategy(family, params);
    BacktestResult test = backtester.run(*chosen, bars, split, barCount);
    delete chosen;
    BuyAndHold hold;
    double holdReturn = backtester.run(hold, bars, split, barCount).totalReturn();

    ReportRow row = rowFromTest(asset, family, test, barCount - split);
    row.params = params;
    row.trainSharpe = trainSharpe;
    row.buyHoldReturn = holdReturn;
    row.verdict = verdictFor(test.trades(), test.sharpe(), test.totalReturn(), holdReturn);
    return row;
}

ReportRow Advisor::baseline(const std::string& asset, const std::vector<Candle>& bars) const {
    int barCount = bars.size();
    int split = splitIndex(barCount);
    BuyAndHold hold;
    BacktestResult test = backtesterFor(asset).run(hold, bars, split, barCount);
    return rowFromTest(asset, "BuyAndHold", test, barCount - split);
}

std::vector<ReportRow> Advisor::recommend(const std::vector<Candle>& btc, const std::vector<Candle>& eth) const {
    std::vector<Candle> ratio = makeRatioSeries(eth, btc);
    std::vector<ReportRow> rows;
    rows.push_back(evaluate("BTCUSDT", "SmaCrossover", btc));
    rows.push_back(evaluate("BTCUSDT", "KnnStrategy", btc));
    rows.push_back(evaluate("ETHUSDT", "SmaCrossover", eth));
    rows.push_back(evaluate("ETHUSDT", "KnnStrategy", eth));
    rows.push_back(evaluate(pairAsset, "PairsSpread", ratio));
    rows.push_back(baseline("BTCUSDT", btc));
    rows.push_back(baseline("ETHUSDT", eth));
    rows.push_back(baseline(pairAsset, ratio));
    return rows;
}

}  // namespace quant
