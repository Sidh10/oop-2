// commands.cpp: download, summary, report and one backtest.
// Syllabus: Unit 2 pointers to objects, new and delete, Unit 3 runtime polymorphism.

#include "commands.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "advisor.hpp"
#include "fetcher.hpp"
#include "ratio.hpp"

namespace quant {

static const int historyDays = 730;
static const std::string reportPath = "reports/recommendation.txt";

int askNumber(const std::string& prompt, int max) {
    std::cout << prompt << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) return -1;

    std::istringstream text(line);
    int number = 0;
    char extra = 0;
    if (!(text >> number) || (text >> extra) || number < 0 || number > max) return -1;
    return number;
}

static void showCoin(const std::string& symbol, const std::vector<Candle>& candles) {
    std::cout << symbol << ": " << candles.size() << " candles, " << timeText(candles.front().openTime)
              << " to " << timeText(candles.back().openTime) << " UTC, last close "
              << candles.back().close << "\n";
}

void downloadData(const DataStore& store) {
    DataFetcher fetcher;
    fetcher.ensureHistory(store, "BTCUSDT", historyDays);
    fetcher.ensureHistory(store, "ETHUSDT", historyDays);
    showCoin("BTCUSDT", store.load("BTCUSDT"));
    showCoin("ETHUSDT", store.load("ETHUSDT"));
}

void showSummary(const DataStore& store) {
    std::vector<Candle> btc = store.load("BTCUSDT");
    std::vector<Candle> eth = store.load("ETHUSDT");
    showCoin("BTCUSDT", btc);
    showCoin("ETHUSDT", eth);
    std::ostringstream text;
    text << std::fixed << std::setprecision(3) << logReturnCorrelation(btc, eth);
    std::cout << "Correlation of hourly log returns: " << text.str() << "\n";
}

void recommend(const DataStore& store) {
    std::vector<ReportRow> rows = Advisor().recommend(store.load("BTCUSDT"), store.load("ETHUSDT"));
    std::string report = buildReport(rows);
    std::cout << report;
    saveReport(reportPath, report);
    saveChoices(bestParamsPath, rows);
    std::cout << "Saved " << reportPath << " and " << bestParamsPath << "\n";
}

static void showResult(const std::string& label, const BacktestResult& result) {
    std::ostringstream text;
    text << std::left << std::setw(26) << label << std::right << std::fixed << std::setprecision(2)
         << "return " << pct(result.totalReturn()) << "%  max drawdown " << pct(result.maxDrawdown())
         << "%  Sharpe " << result.sharpe() << "  trades " << result.trades();
    std::cout << text.str() << "\n";
}

static std::string chooseFamily(int assetNumber) {
    if (assetNumber == 3) return "PairsSpread";
    int family = askNumber("Strategy: 1) SmaCrossover  2) KnnStrategy: ", 2);
    if (family == 1) return "SmaCrossover";
    if (family == 2) return "KnnStrategy";
    return "";
}

// Returns an index into the grid, or -1 for a bad answer.
static int chooseSettings(const std::vector<std::vector<double>>& grid) {
    int count = grid.size();
    for (int g = 0; g < count; g++) {
        std::cout << "  " << g + 1 << ") " << grid[g][0] << " " << grid[g][1] << "\n";
    }
    return askNumber("Settings: ", count) - 1;
}

static std::vector<Candle> loadAsset(const DataStore& store, int assetNumber) {
    if (assetNumber == 1) return store.load("BTCUSDT");
    if (assetNumber == 2) return store.load("ETHUSDT");
    return makeRatioSeries(store.load("ETHUSDT"), store.load("BTCUSDT"));
}

// Runs the strategy and buy-and-hold over the same bars, so the two lines can be compared.
static void compareWithHold(const std::string& family, const std::vector<double>& settings, bool isPair,
                            const std::vector<Candle>& bars, int begin) {
    int end = bars.size();
    Backtester backtester = backtesterFor("BTCUSDT");
    if (isPair) backtester = backtesterFor(pairAsset);

    Strategy* strategy = makeStrategy(family, settings);
    showResult(strategy->describe(), backtester.run(*strategy, bars, begin, end));
    delete strategy;
    BuyAndHold hold;
    showResult("Buy and hold", backtester.run(hold, bars, begin, end));
}

void backtestOne(const DataStore& store) {
    int assetNumber = askNumber("Asset: 1) BTCUSDT  2) ETHUSDT  3) ETH/BTC pair: ", 3);
    std::string family;
    if (assetNumber >= 1) family = chooseFamily(assetNumber);

    int settings = -1;
    std::vector<std::vector<double>> grid;
    if (!family.empty()) {
        grid = paramGrid(family);
        settings = chooseSettings(grid);
    }
    int window = -1;
    if (settings >= 0) window = askNumber("Window: 1) all history  2) test window: ", 2);
    if (window < 1) {
        std::cout << "Cancelled.\n";
        return;
    }

    std::vector<Candle> bars = loadAsset(store, assetNumber);
    int begin = 0;
    if (window == 2) begin = splitIndex(bars.size());
    compareWithHold(family, grid[settings], assetNumber == 3, bars, begin);
}

}  // namespace quant
