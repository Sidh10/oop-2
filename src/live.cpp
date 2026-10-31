// live.cpp: shows what each chosen strategy says on fresh candles. It never places orders.
// Syllabus: Unit 2 pointers to objects, new and delete, Unit 3 runtime polymorphism.

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "advisor.hpp"
#include "commands.hpp"
#include "fetcher.hpp"
#include "ratio.hpp"

namespace quant {

// Replays the strategy over the fresh candles and shows what it says at the last closed bar.
static void showLiveSignal(const Choice& choice, const std::vector<Candle>& bars) {
    Strategy* strategy = makeStrategy(choice.family, choice.params);
    strategy->reset();
    Position signal = FLAT;
    int count = bars.size();
    for (int i = 0; i < count; i++) {
        signal = strategy->signal(bars, i);
    }

    std::ostringstream text;
    text << std::left << std::setw(9) << choice.asset << std::setw(26) << strategy->describe()
         << std::setw(6) << (signal == LONG ? "LONG" : "FLAT") << " last close " << std::setprecision(8)
         << bars.back().close << " at " << timeText(bars.back().openTime) << " UTC  (" << choice.verdict << ")";
    std::cout << text.str() << "\n";
    delete strategy;
}

void liveSignals() {
    std::vector<Choice> choices = loadChoices(bestParamsPath);
    DataFetcher fetcher;
    std::vector<Candle> btc = fetcher.fetchLatest("BTCUSDT");
    std::vector<Candle> eth = fetcher.fetchLatest("ETHUSDT");
    std::vector<Candle> ratio = makeRatioSeries(eth, btc);

    bool anyRecommended = false;
    int count = choices.size();
    for (int c = 0; c < count; c++) {
        if (choices[c].asset == "BTCUSDT") showLiveSignal(choices[c], btc);
        if (choices[c].asset == "ETHUSDT") showLiveSignal(choices[c], eth);
        if (choices[c].asset == pairAsset) showLiveSignal(choices[c], ratio);
        if (choices[c].verdict == verdictName(Verdict::Recommended)) anyRecommended = true;
    }
    if (!anyRecommended) {
        std::cout << "Signals are shown for information only: no strategy passed validation.\n";
    }
    std::cout << "No orders are ever placed. Research only. Not financial advice.\n";
}

}  // namespace quant
