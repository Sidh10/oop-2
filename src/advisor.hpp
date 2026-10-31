// advisor.hpp: chooses settings on old data, tests them on new data, and writes the report.
// Syllabus: Unit 1 classes, structures and message passing, Unit 5 file output.

#ifndef ADVISOR_HPP
#define ADVISOR_HPP

#include <string>
#include <vector>

#include "backtester.hpp"
#include "candle.hpp"

namespace quant {

// The name of the ETH/BTC ratio. Rows with this name pay the two-leg cost.
const std::string pairAsset = "ETH/BTC";

// The pair has two legs, so it pays the cost twice on every buy and sell.
Backtester backtesterFor(const std::string& asset);

// The first bar of the test window: bars before it train, bars from it on test.
int splitIndex(int barCount);

Verdict verdictFor(int testTrades, double testSharpe, double testReturn, double buyHoldReturn);

struct ReportRow {
    std::string asset;
    std::string family;
    std::vector<double> params;
    double trainSharpe;
    double testReturn;
    double testSharpe;
    double testDrawdown;
    int testTrades;
    double buyHoldReturn;
    Verdict verdict;
    int testBars;
};

class Advisor {
public:
    // One row: choose on the train window, then run that choice once on the test window.
    ReportRow evaluate(const std::string& asset, const std::string& family,
                       const std::vector<Candle>& bars) const;

    ReportRow baseline(const std::string& asset, const std::vector<Candle>& bars) const;

    std::vector<ReportRow> recommend(const std::vector<Candle>& btc, const std::vector<Candle>& eth) const;
};

// One line of best_params.csv.
struct Choice {
    std::string asset;
    std::string family;
    std::string verdict;
    std::vector<double> params;
};

// These four are in report.cpp.
std::string buildReport(const std::vector<ReportRow>& rows);
void saveReport(const std::string& path, const std::string& report);
void saveChoices(const std::string& path, const std::vector<ReportRow>& rows);
std::vector<Choice> loadChoices(const std::string& path);

}  // namespace quant

#endif
