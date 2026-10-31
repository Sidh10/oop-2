// test_all.cpp: every test for the program, in one executable.
// Each testSomething function checks one part, and main runs them all.

#include <cmath>
#include <cstdio>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "advisor.hpp"
#include "backtester.hpp"
#include "candle.hpp"
#include "datastore.hpp"
#include "demo/demo.hpp"
#include "errors.hpp"
#include "fetcher.hpp"
#include "indicators.hpp"
#include "minitest.hpp"
#include "ratio.hpp"
#include "strategy.hpp"

using namespace quant;

const double eps = 1e-9;

// Turns "1 2 3" into a vector, which keeps the test data short and easy to read.
std::vector<double> numbers(const std::string& text) {
    std::istringstream in(text);
    std::vector<double> values;
    double value = 0;
    while (in >> value) {
        values.push_back(value);
    }
    return values;
}

void testCandleBasics() {
    CHECK_NEAR(pct(0.25), 25.0, eps);

    Candle empty;
    CHECK(empty.openTime == 0);

    // high and low left out: they fall back to the open and close.
    Candle shortForm(1000, 100, 110);
    CHECK_NEAR(shortForm.high, 110.0, eps);
    CHECK_NEAR(shortForm.low, 100.0, eps);
    CHECK_NEAR(shortForm.volume, 0.0, eps);

    Candle fullForm(1000, 100, 110, 115, 95, 7);
    CHECK_NEAR(fullForm.high, 115.0, eps);
    CHECK_NEAR(fullForm.low, 95.0, eps);
    CHECK_NEAR(fullForm.volume, 7.0, eps);

    CHECK(timeText(1740837600000LL) == "2025-03-01 14:00");
    CHECK(Candle(1740837600000LL, 100.5, 101.25).toText() == "2025-03-01 14:00 O=100.5 C=101.25");
    CHECK(verdictName(Verdict::Recommended) == "RECOMMENDED");
    CHECK(verdictName(Verdict::Baseline) == "BASELINE");
}

void testIndicators() {
    std::vector<double> closes = numbers("1 2 3 4 5");
    CHECK_NEAR(indicators::sma(closes, 4, 3), 4.0, eps);
    CHECK_NEAR(indicators::sma(closes, 2, 2), 2.5, eps);

    // Mean 5, sum of squares 32, so std = sqrt(32/7) and z = 4 / std.
    std::vector<double> known = numbers("2 4 4 4 5 5 7 9");
    CHECK_NEAR(indicators::zscore(known, 7, 8), 1.8708286933869707, eps);
    std::vector<double> flat = numbers("7 7 7 7 7");
    CHECK_NEAR(indicators::zscore(flat, 4, 5), 0.0, eps);

    // Changing a later value must not change a result at an earlier index.
    std::vector<double> later = known;
    later[7] = 1000;
    CHECK_NEAR(indicators::sma(later, 6, 3), indicators::sma(known, 6, 3), eps);
    CHECK_NEAR(indicators::zscore(later, 6, 5), indicators::zscore(known, 6, 5), eps);

    std::vector<Candle> bars;
    for (int i = 0; i < 5; i++) {
        bars.push_back(Candle(i, closes[i], closes[i]));
    }
    std::vector<double> lastTwo = indicators::lastCloses(bars, 3, 2);
    CHECK(lastTwo.size() == 2);
    CHECK_NEAR(lastTwo[0], 3.0, eps);
    CHECK_NEAR(lastTwo[1], 4.0, eps);
}

void testExceptions() {
    CHECK_THROWS(throw DataError("x"), QuantError);
    CHECK_THROWS(throw NetworkError("x"), QuantError);
    CHECK_THROWS(throw DivideByZeroError("x"), QuantError);
    CHECK_THROWS(throw DataError("x"), std::runtime_error);

    try {
        throw NetworkError("blocked", 451);
    } catch (const QuantError& error) {
        CHECK(std::string(error.what()) == "blocked");
    }
    CHECK(NetworkError("x", 429).httpStatus() == 429);
    CHECK(NetworkError("x").httpStatus() == 0);
}

void testCandleStreams() {
    // Values chosen so that anything less than full precision would change them.
    Candle original(1740837600000LL, 1.0 / 3.0, 2.0 / 3.0, 1234567.1234567891, 1e-7, 0.1 + 0.2);
    std::stringstream stream;
    stream << original << '\n';
    Candle copy;
    stream >> copy;
    CHECK(!stream.fail());
    CHECK(copy.openTime == original.openTime);
    CHECK(copy.open == original.open);
    CHECK(copy.high == original.high);
    CHECK(copy.low == original.low);
    CHECK(copy.close == original.close);
    CHECK(copy.volume == original.volume);

    // Writing a candle must not leave the caller's stream at 17 digits.
    std::ostringstream precision;
    precision << original << ' ';
    CHECK(precision.precision() == 6);

    std::istringstream bad("not,a,candle\n");
    Candle untouched(5, 1, 2);
    bad >> untouched;
    CHECK(bad.fail());
    CHECK(untouched.openTime == 5);
}

// The store uses the current folder and removes its files afterwards.
void testDataStore() {
    DataStore store(".");
    const std::string symbol = "TESTONLY";
    std::string path = store.pathFor(symbol);
    std::remove(path.c_str());

    CHECK(!store.hasFile(symbol));
    CHECK_THROWS(store.load(symbol), DataError);

    std::vector<Candle> threeLines;
    threeLines.push_back(Candle(1000, 10, 11, 12, 9, 100));
    threeLines.push_back(Candle(2000, 11, 12, 13, 10, 200));
    threeLines.push_back(Candle(3000, 12, 13, 14, 11, 300));
    store.save(symbol, threeLines);
    CHECK(store.hasFile(symbol));

    std::vector<Candle> loaded = store.load(symbol);
    CHECK(loaded.size() == 3);
    CHECK(loaded[1].openTime == 2000);
    CHECK_NEAR(loaded[2].volume, 300.0, eps);

    std::vector<Candle> newer;
    newer.push_back(Candle(4000, 13, 14, 15, 12, 400));
    store.append(symbol, newer);
    CHECK(store.load(symbol).size() == 4);
    CHECK(store.load(symbol).back().openTime == 4000);

    // Only the header: the file exists but holds no candles.
    store.save(symbol, std::vector<Candle>());
    CHECK(store.load(symbol).empty());

    // A corrupt line in the middle must be reported, not skipped.
    std::ofstream broken(path.c_str(), std::ios::binary);
    broken << "openTime,open,high,low,close,volume\n1000,1,1,1,1,1\ngarbage\n2000,1,1,1,1,1\n";
    broken.close();
    CHECK_THROWS(store.load(symbol), DataError);

    std::remove(path.c_str());
}

// A small cache must grow when a larger range is asked for.
void testNeedsFullRefetch() {
    const long long day = 86400000LL;
    const long long now = 1000 * day;
    CHECK(needsFullRefetch(-1, now, 30));  // no cache
    CHECK(!needsFullRefetch(now - 30 * day, now, 30));  // covers exactly
    CHECK(!needsFullRefetch(now - 40 * day, now, 30));  // covers more than asked
    CHECK(!needsFullRefetch(now - 29 * day, now, 30));  // exactly one day late is tolerated
    CHECK(needsFullRefetch(now - 29 * day + 1, now, 30));
    CHECK(needsFullRefetch(now - 30 * day, now, 730));  // a 30-day cache, 730 days asked for
}

// A canned reply in Binance's real shape, so no network is needed.
void testParsing() {
    const std::string threeCandles =
        "[[1000,\"10.5\",\"12.0\",\"9.5\",\"11.5\",\"100.25\",4599,\"1\",5,\"1\",\"1\",\"0\"],"
        "[5000,\"11.5\",\"13.0\",\"10.5\",\"12.5\",\"200.5\",8999,\"1\",5,\"1\",\"1\",\"0\"],"
        "[9000,\"12.5\",\"14.0\",\"11.5\",\"13.5\",\"300.75\",12999,\"1\",5,\"1\",\"1\",\"0\"]]";

    // At time 10000 the third candle is still open, because it closes at 12999.
    std::vector<Candle> candles = parseKlines(threeCandles, 10000);
    CHECK(candles.size() == 2);
    CHECK(candles[0].openTime == 1000);
    CHECK_NEAR(candles[0].open, 10.5, eps);
    CHECK_NEAR(candles[0].high, 12.0, eps);
    CHECK_NEAR(candles[0].low, 9.5, eps);
    CHECK_NEAR(candles[0].close, 11.5, eps);
    CHECK_NEAR(candles[0].volume, 100.25, eps);
    CHECK(candles[1].openTime == 5000);
    CHECK_NEAR(candles[1].close, 12.5, eps);

    CHECK(parseKlines(threeCandles, 12999).size() == 2);  // closeTime == now still counts as open
    CHECK(parseKlines(threeCandles, 13000).size() == 3);
    CHECK(parseKlines("[]", 10000).empty());

    CHECK_THROWS(parseKlines("[[1000,\"10.5\"", 10000), DataError);  // cut off
    CHECK_THROWS(parseKlines("{\"code\":-1003}", 10000), DataError);  // an error object
    CHECK_THROWS(parseKlines("[[1000,\"10.5\"]]", 10000), DataError);  // too few fields
    CHECK_THROWS(parseKlines("[[1,\"abc\",\"1\",\"1\",\"1\",\"1\",2]]", 10000), DataError);  // not a number
    CHECK_THROWS(parseKlines("[[1,2,3,4,5,6,7]]", 10000), DataError);  // numbers, not strings
}

void testValidation() {
    std::vector<Candle> good;
    good.push_back(Candle(1000, 10, 11, 12, 9, 1));
    good.push_back(Candle(2000, 11, 12, 13, 10, 1));
    validateCandles(good);
    validateCandles(std::vector<Candle>());

    std::vector<Candle> repeatedTime = good;
    repeatedTime[1].openTime = 1000;
    CHECK_THROWS(validateCandles(repeatedTime), DataError);

    std::vector<Candle> zeroPrice = good;
    zeroPrice[1].close = 0;
    CHECK_THROWS(validateCandles(zeroPrice), DataError);

    std::vector<Candle> highBelowLow = good;
    highBelowLow[1].high = 1;
    CHECK_THROWS(validateCandles(highBelowLow), DataError);
}

void testFetcherObject() {
    CHECK(!std::is_copy_constructible<DataFetcher>::value);
    CHECK(!std::is_copy_assignable<DataFetcher>::value);
    DataFetcher fetcher;  // starts and cleans up without touching the network
    CHECK(currentTimeMs() > 1700000000000LL);
}

// Replays a fixed list of signals, so the fills can be tested on their own.
class ScriptedStrategy : public Strategy {
public:
    ScriptedStrategy(const std::string& decisions) { script = decisions; }
    std::string name() const override { return "Scripted"; }
    std::vector<double> params() const override { return std::vector<double>(); }
    Position signal(const std::vector<Candle>&, int i) override {
        if (script[i] == '1') return LONG;
        return FLAT;
    }

private:
    std::string script;
};

std::vector<Candle> flatBars(int count, double price) {
    std::vector<Candle> bars;
    for (int i = 0; i < count; i++) {
        bars.push_back(Candle(i, price, price));
    }
    return bars;
}

// Bars with these closes, each opening at the previous close.
std::vector<Candle> barsFromCloses(const std::vector<double>& closes) {
    std::vector<Candle> bars;
    int count = closes.size();
    for (int i = 0; i < count; i++) {
        double open = closes[0];
        if (i > 0) open = closes[i - 1];
        bars.push_back(Candle(i, open, closes[i]));
    }
    return bars;
}

void testToyBacktest() {
    std::vector<Candle> bars;
    bars.push_back(Candle(0, 100, 100));
    bars.push_back(Candle(1, 100, 110));
    bars.push_back(Candle(2, 110, 121));
    bars.push_back(Candle(3, 121, 110));

    BuyAndHold alwaysLong;
    BacktestResult result = Backtester(10000, 0.001).run(alwaysLong, bars, 0, 4);
    CHECK(result.equity().size() == 4);
    CHECK_NEAR(result.equity()[0], 10000.0, 1e-6);
    CHECK_NEAR(result.equity()[1], 10989.0, 1e-6);
    CHECK_NEAR(result.equity()[2], 12087.9, 1e-6);
    CHECK_NEAR(result.equity()[3], 10989.0, 1e-6);
    CHECK_NEAR(result.totalReturn(), 0.0989, 1e-6);
    CHECK_NEAR(result.maxDrawdown(), 1 - 110.0 / 121.0, 1e-6);
    CHECK(result.trades() == 1);
}

void testFills() {
    ScriptedStrategy buyThenSell("1000");
    BacktestResult roundTrip = Backtester(10000, 0.001).run(buyThenSell, flatBars(4, 100), 0, 4);
    CHECK_NEAR(roundTrip.equity()[1], 9990.0, 1e-6);
    CHECK_NEAR(roundTrip.equity()[2], 9980.01, 1e-6);
    CHECK_NEAR(roundTrip.equity()[3], 9980.01, 1e-6);
    CHECK(roundTrip.trades() == 2);

    ScriptedStrategy lastBarOnly("0001");
    BacktestResult lastBar = Backtester(10000, 0.001).run(lastBarOnly, flatBars(4, 100), 0, 4);
    CHECK(lastBar.trades() == 0);
    for (int k = 0; k < 4; k++) {
        CHECK_NEAR(lastBar.equity()[k], 10000.0, 1e-6);
    }

    // A fill uses the open of the next bar, not the close of the signal bar.
    std::vector<Candle> gapUp;
    gapUp.push_back(Candle(0, 100, 100));
    gapUp.push_back(Candle(1, 200, 200));
    gapUp.push_back(Candle(2, 200, 400));
    ScriptedStrategy buyFirst("111");
    CHECK_NEAR(Backtester(10000, 0).run(buyFirst, gapUp, 0, 3).equity()[2], 20000.0, 1e-6);

    // A window that starts later still begins flat with all the cash, and only sees its own bars.
    BuyAndHold hold;
    BacktestResult window = Backtester(10000, 0).run(hold, gapUp, 1, 3);
    CHECK(window.equity().size() == 2);
    CHECK_NEAR(window.equity()[0], 10000.0, 1e-6);
    CHECK_NEAR(window.equity()[1], 20000.0, 1e-6);

    CHECK_THROWS(Backtester().run(hold, gapUp, 0, 4), DataError);
    CHECK_THROWS(Backtester().run(hold, gapUp, 2, 2), DataError);
}

void testMetrics() {
    Metrics metrics = computeMetrics(numbers("100 120 90 130"), 8760);
    CHECK_NEAR(metrics.maxDrawdown, 0.25, eps);
    CHECK_NEAR(metrics.totalReturn, 0.3, eps);

    // Returns +1 and -0.5: mean 0.25, sample std sqrt(1.125), 4 bars per year.
    Metrics known = computeMetrics(numbers("100 200 100"), 4);
    CHECK_NEAR(known.sharpe, 0.25 / std::sqrt(1.125) * 2, eps);

    // Constant growth has zero variance, and doubling is exact in floating point.
    CHECK_NEAR(computeMetrics(numbers("100 200 400 800"), 8760).sharpe, 0.0, eps);
    CHECK_NEAR(computeMetrics(numbers("100 100 100"), 8760).sharpe, 0.0, eps);
    CHECK_NEAR(computeMetrics(numbers("100 110"), 8760).sharpe, 0.0, eps);  // one return, no std
    CHECK_NEAR(computeMetrics(numbers("100"), 8760).totalReturn, 0.0, eps);
    CHECK_NEAR(computeMetrics(std::vector<double>(), 8760).sharpe, 0.0, eps);
}

// A child whose destructor can be seen, to prove delete through a Strategy* reaches it.
class CountsDestruction : public BuyAndHold {
public:
    CountsDestruction(int* counter) { destroyed = counter; }
    ~CountsDestruction() override { (*destroyed)++; }

private:
    int* destroyed;
};

void testSmaAndPairs() {
    SmaCrossover crossover(1, 2);
    std::vector<Candle> rising = barsFromCloses(numbers("1 2 3"));
    CHECK(crossover.signal(rising, 0) == FLAT);  // not enough history
    CHECK(crossover.signal(rising, 1) == LONG);  // 2 > (1+2)/2
    std::vector<Candle> falling = barsFromCloses(numbers("3 2 1"));
    CHECK(crossover.signal(falling, 2) == FLAT);

    // Window 3: a flat stretch, a sharp dip (enter), then a recovery above the average (exit).
    std::vector<Candle> dip = barsFromCloses(numbers("100 100 100 80 80 100 120"));
    PairsSpread pairs(3, 1.0);
    CHECK(pairs.signal(dip, 1) == FLAT);  // not enough history
    CHECK(pairs.signal(dip, 2) == FLAT);  // z = 0
    CHECK(pairs.signal(dip, 3) == LONG);  // z = (80 - 93.3) / 11.5 = -1.15
    CHECK(pairs.signal(dip, 4) == LONG);  // still below the average
    CHECK(pairs.signal(dip, 5) == FLAT);  // z rises above 0
    pairs.reset();
    CHECK(pairs.signal(dip, 4) == FLAT);  // after reset it is flat, and z = -0.58 is no entry

    // The exit is at z >= 0: at exactly 0 (90 is the mean of 100, 80, 90) it must already leave.
    std::vector<Candle> exactMean = barsFromCloses(numbers("100 100 100 80 90"));
    PairsSpread exitCheck(3, 1.0);
    exitCheck.signal(exactMean, 3);
    CHECK(exitCheck.signal(exactMean, 4) == FLAT);

    CHECK(SmaCrossover(10, 50).describe() == "SmaCrossover(10, 50)");
    CHECK(PairsSpread(48, 1.5).describe() == "PairsSpread(48, 1.5)");
    CHECK(KnnStrategy(30, 0.0005).describe() == "KnnStrategy(30, 0.0005)");
    CHECK(BuyAndHold().describe() == "BuyAndHold()");

    // A Strategy* holding a child: the virtual calls and the delete reach the child.
    Strategy* made = makeStrategy("BuyAndHold", std::vector<double>());
    CHECK(made->name() == "BuyAndHold");
    CHECK(made->signal(rising, 0) == LONG);
    delete made;

    int destroyed = 0;
    Strategy* child = new CountsDestruction(&destroyed);
    delete child;
    CHECK(destroyed == 1);
}

void testGrids() {
    CHECK(strategyFamilies().size() == 3);
    std::vector<std::vector<double>> crossovers = paramGrid("SmaCrossover");
    CHECK(crossovers.size() == 8);
    CHECK(crossovers[0] == numbers("10 50"));
    CHECK(crossovers[7] == numbers("50 200"));  // 50/50 was skipped
    CHECK(paramGrid("KnnStrategy").size() == 4);
    CHECK(paramGrid("KnnStrategy")[3] == numbers("30 0.0005"));
    CHECK(paramGrid("PairsSpread").size() == 6);

    CHECK_THROWS(paramGrid("NoSuchFamily"), QuantError);
    CHECK_THROWS(makeStrategy("NoSuchFamily", std::vector<double>()), QuantError);
    CHECK_THROWS(makeStrategy("SmaCrossover", numbers("10")), QuantError);  // wrong parameter count
}

// A seeded random walk with no drift, so the results repeat exactly.
std::vector<Candle> randomWalk(unsigned seed, int count, double deviation) {
    std::mt19937 generator(seed);
    std::normal_distribution<double> noise(0, deviation);
    std::vector<Candle> bars;
    double price = 100;
    for (int i = 0; i < count; i++) {
        double next = price * std::exp(noise(generator));
        bars.push_back(Candle(i * 3600000LL, price, next));
        price = next;
    }
    return bars;
}

// Cheats by reading the next bar.
class PeekNextBar : public Strategy {
public:
    std::string name() const override { return "PeekNextBar"; }
    std::vector<double> params() const override { return std::vector<double>(); }
    Position signal(const std::vector<Candle>& bars, int i) override {
        int count = bars.size();
        if (i + 1 >= count) return FLAT;
        if (bars[i + 1].close > bars[i].close) return LONG;
        return FLAT;
    }
};

// Cheats during warm-up: it should only see bars before begin, but keeps the very last close.
class PeekInWarmUp : public Strategy {
public:
    PeekInWarmUp() { lastClose = 0; }
    std::string name() const override { return "PeekInWarmUp"; }
    std::vector<double> params() const override { return std::vector<double>(); }
    void warmUp(const std::vector<Candle>& bars, int) override { lastClose = bars.back().close; }
    Position signal(const std::vector<Candle>& bars, int i) override {
        if (lastClose > bars[i].close) return LONG;
        return FLAT;
    }

private:
    double lastClose;
};

// Cheats by reading the last bar of the series, however far away it is.
class PeekLastBar : public Strategy {
public:
    std::string name() const override { return "PeekLastBar"; }
    std::vector<double> params() const override { return std::vector<double>(); }
    Position signal(const std::vector<Candle>& bars, int i) override {
        if (bars.back().close > bars[i].close) return LONG;
        return FLAT;
    }
};

// The signals for begin..last, made the way the backtester makes them.
std::vector<Position> signalsOf(Strategy& strategy, const std::vector<Candle>& bars, int begin, int last) {
    std::vector<Position> signals;
    strategy.reset();
    strategy.warmUp(bars, begin);
    for (int i = begin; i <= last; i++) {
        signals.push_back(strategy.signal(bars, i));
    }
    return signals;
}

// Signals up to k must not change when every bar after k is removed.
bool hasLookahead(Strategy& strategy, const std::vector<Candle>& bars, int begin) {
    int lastBar = bars.size();
    lastBar = lastBar - 1;
    std::vector<Position> full = signalsOf(strategy, bars, begin, lastBar);
    for (int k = begin; k < lastBar; k += 7) {
        std::vector<Candle> cut(bars.begin(), bars.begin() + k + 1);
        std::vector<Position> truncated = signalsOf(strategy, cut, begin, k);
        int count = truncated.size();
        for (int j = 0; j < count; j++) {
            if (truncated[j] != full[j]) return true;
        }
    }
    return false;
}

void testLeakage() {
    // The checker has to work, or a clean result below means nothing.
    PeekNextBar peekNext;
    PeekLastBar peekLast;
    PeekInWarmUp peekWarmUp;
    std::vector<Candle> checkerBars = randomWalk(1, 300, 0.01);
    CHECK(hasLookahead(peekNext, checkerBars, 0));
    CHECK(hasLookahead(peekNext, checkerBars, 100));
    CHECK(hasLookahead(peekLast, checkerBars, 0));
    CHECK(hasLookahead(peekWarmUp, checkerBars, 100));

    BuyAndHold hold;
    CHECK(!hasLookahead(hold, checkerBars, 0));

    std::vector<std::string> families = strategyFamilies();
    for (int f = 0; f < 3; f++) {
        std::vector<std::vector<double>> grid = paramGrid(families[f]);
        int gridSize = grid.size();
        bool sawLong = false;
        bool sawFlat = false;
        for (int seed = 2; seed <= 4; seed++) {
            std::vector<Candle> bars = randomWalk(seed, 700, 0.02);
            for (int g = 0; g < gridSize; g++) {
                Strategy* strategy = makeStrategy(families[f], grid[g]);
                CHECK(!hasLookahead(*strategy, bars, 0));
                CHECK(!hasLookahead(*strategy, bars, 400));
                std::vector<Position> signals = signalsOf(*strategy, bars, 0, 699);
                for (int j = 0; j < 700; j++) {
                    if (signals[j] == LONG) sawLong = true;
                    if (signals[j] == FLAT) sawFlat = true;
                }
                delete strategy;
            }
        }
        // A strategy that never trades would pass the leakage check for no good reason.
        CHECK(sawLong);
        CHECK(sawFlat);
    }
}

Sample sampleAt(double value, double label) {
    Sample sample;
    for (int j = 0; j < situationSize; j++) {
        sample.x[j] = value;
    }
    sample.label = label;
    return sample;
}

void testKnnPrediction() {
    double nearZero[situationSize] = {0.1, 0, 0, 0, 0};
    double nearOne[situationSize] = {0.9, 1, 1, 1, 1};

    KnnStrategy empty(1, 0);
    CHECK_NEAR(empty.predict(nearZero), 0.0, eps);

    // The hand case: two known samples, k = 1 returns the label of the nearer one.
    KnnStrategy nearest(1, 0);
    nearest.remember(sampleAt(0, 0.01));
    nearest.remember(sampleAt(1, -0.02));
    CHECK_NEAR(nearest.predict(nearZero), 0.01, eps);
    CHECK_NEAR(nearest.predict(nearOne), -0.02, eps);

    KnnStrategy both(2, 0);
    both.remember(sampleAt(0, 0.01));
    both.remember(sampleAt(1, -0.02));
    CHECK_NEAR(both.predict(nearOne), -0.005, eps);  // the mean of both labels

    KnnStrategy tooMany(15, 0);
    tooMany.remember(sampleAt(0, 0.01));
    tooMany.remember(sampleAt(1, -0.02));
    CHECK_NEAR(tooMany.predict(nearOne), -0.005, eps);  // k larger than the memory
}

void testKnnLearning() {
    // A situation is the last 5 log returns, newest first. The label is the next bar's return.
    std::vector<Candle> bars = barsFromCloses(numbers("100 110 100 120 90 100 130 65"));
    KnnStrategy learner(1, 0);
    learner.warmUp(bars, 6);
    CHECK(learner.samples().empty());  // bar 4 has only 4 returns behind it
    learner.warmUp(bars, 7);
    CHECK(learner.samples().size() == 1);
    CHECK_NEAR(learner.samples()[0].label, std::log(130.0 / 100.0), eps);
    CHECK_NEAR(learner.samples()[0].x[0], std::log(100.0 / 90.0), eps);  // the situation of bar 5
    CHECK_NEAR(learner.samples()[0].x[4], std::log(110.0 / 100.0), eps);

    // warmUp learns only from bars before begin, and old samples fall out at 1000.
    std::vector<Candle> walk = randomWalk(5, 1300, 0.01);
    KnnStrategy warmed(1, 0);
    warmed.warmUp(walk, 20);
    CHECK(warmed.samples().size() == 14);  // bars 6 to 19
    CHECK_NEAR(warmed.samples()[13].label, std::log(walk[19].close / walk[18].close), eps);
    warmed.reset();
    CHECK(warmed.samples().empty());
    for (int i = 0; i < 1200; i++) {
        warmed.signal(walk, i);
    }
    CHECK(warmed.samples().size() == 1000);
    CHECK_NEAR(warmed.samples()[999].label, std::log(walk[1199].close / walk[1198].close), eps);

    // A series that alternates up and down: having learned it, kNN expects each next move.
    std::vector<double> closes;
    for (int i = 0; i < 60; i++) {
        if (i % 2 == 0) {
            closes.push_back(100);
        } else {
            closes.push_back(101);
        }
    }
    std::vector<Candle> zigzag = barsFromCloses(closes);
    KnnStrategy knn(1, 0.0001);
    CHECK(knn.signal(zigzag, 0) == FLAT);  // nothing learned yet
    for (int i = 1; i < 50; i++) {
        knn.signal(zigzag, i);
    }
    CHECK(knn.signal(zigzag, 50) == LONG);  // bar 50 fell to 100, so bar 51 should rise
    CHECK(knn.signal(zigzag, 51) == FLAT);  // bar 51 rose to 101, so bar 52 should fall
}

void testRatioCorrelation(const std::vector<Candle>& noCommon) {
    // Known answers, and only common times count.
    std::vector<Candle> walk = randomWalk(3, 200, 0.01);
    std::vector<Candle> doubled;
    std::vector<Candle> inverse;
    std::vector<Candle> flatCoin;
    for (int i = 0; i < 200; i++) {
        doubled.push_back(Candle(walk[i].openTime, walk[i].open * 2, walk[i].close * 2));
        inverse.push_back(Candle(walk[i].openTime, 1 / walk[i].open, 1 / walk[i].close));
        flatCoin.push_back(Candle(walk[i].openTime, 100, 100));
    }
    CHECK_NEAR(logReturnCorrelation(walk, doubled), 1.0, eps);
    CHECK_NEAR(logReturnCorrelation(walk, inverse), -1.0, eps);
    CHECK_NEAR(logReturnCorrelation(walk, flatCoin), 0.0, eps);  // a coin that never moves
    CHECK_NEAR(logReturnCorrelation(flatCoin, walk), 0.0, eps);

    std::vector<Candle> otherWalk = randomWalk(4, 200, 0.01);
    double unrelated = logReturnCorrelation(walk, otherWalk);
    CHECK(unrelated > -0.3 && unrelated < 0.3);

    std::vector<Candle> fewer(doubled.begin() + 50, doubled.end());
    CHECK_NEAR(logReturnCorrelation(walk, fewer), 1.0, eps);
    CHECK_THROWS(logReturnCorrelation(walk, noCommon), DataError);
}

void testRatioSeries() {
    std::vector<Candle> eth;
    eth.push_back(Candle(1000, 10, 20));
    eth.push_back(Candle(2000, 30, 15));
    eth.push_back(Candle(4000, 40, 60));
    std::vector<Candle> btc;
    btc.push_back(Candle(2000, 100, 300));
    btc.push_back(Candle(3000, 200, 200));
    btc.push_back(Candle(4000, 400, 300));
    btc.push_back(Candle(5000, 500, 500));

    std::vector<Candle> alignedEth;
    std::vector<Candle> alignedBtc;
    alignByOpenTime(eth, btc, alignedEth, alignedBtc);
    CHECK(alignedEth.size() == 2);
    CHECK(alignedBtc.size() == 2);
    CHECK(alignedEth[0].openTime == 2000);
    CHECK(alignedBtc[1].openTime == 4000);

    std::vector<Candle> ratio = makeRatioSeries(eth, btc);
    CHECK(ratio.size() == 2);
    CHECK(ratio[0].openTime == 2000);
    CHECK_NEAR(ratio[0].open, 0.3, eps);  // 30 / 100
    CHECK_NEAR(ratio[0].close, 0.05, eps);  // 15 / 300
    CHECK_NEAR(ratio[0].high, 0.3, eps);
    CHECK_NEAR(ratio[0].low, 0.05, eps);
    CHECK_NEAR(ratio[1].open, 0.1, eps);  // 40 / 400
    CHECK_NEAR(ratio[1].close, 0.2, eps);  // 60 / 300
    CHECK_NEAR(ratio[1].volume, 0.0, eps);

    std::vector<Candle> noCommon;
    noCommon.push_back(Candle(9000, 1, 1));
    CHECK_THROWS(makeRatioSeries(eth, noCommon), DataError);

    std::vector<Candle> zeroPrice = btc;
    zeroPrice[0].close = 0;
    CHECK_THROWS(makeRatioSeries(eth, zeroPrice), DataError);

    testRatioCorrelation(noCommon);
}

// One case per branch, plus the edges of each threshold.
void testVerdicts() {
    CHECK(verdictFor(9, 5.0, 1.0, 0.0) == Verdict::Insufficient);
    CHECK(verdictFor(10, 0.5, 0.2, 0.1) == Verdict::Recommended);
    CHECK(verdictFor(10, 0.49, 0.2, 0.1) == Verdict::Weak);  // Sharpe too low
    CHECK(verdictFor(10, 2.0, 0.1, 0.2) == Verdict::Weak);  // did not beat buy-and-hold
    CHECK(verdictFor(10, 2.0, 0.1, 0.1) == Verdict::Weak);  // a tie does not beat it either
    CHECK(verdictFor(10, 2.0, 0.0, -0.5) == Verdict::Reject);  // beat it, but made nothing
    CHECK(verdictFor(10, -1.0, -0.1, -0.5) == Verdict::Reject);
    CHECK(verdictName(verdictFor(0, 0, 0, 0)) == "INSUFFICIENT");
}

void testChoiceOnTrain(const Advisor& advisor, const std::vector<Candle>& bars, const ReportRow& row) {
    // The train Sharpe must be the best of the grid, on the train window only.
    std::vector<std::vector<double>> grid = paramGrid("SmaCrossover");
    double best = -1e9;
    int count = grid.size();
    for (int g = 0; g < count; g++) {
        Strategy* strategy = makeStrategy("SmaCrossover", grid[g]);
        double sharpe = Backtester().run(*strategy, bars, 0, 700).sharpe();
        delete strategy;
        if (sharpe > best) best = sharpe;
    }
    CHECK_NEAR(row.trainSharpe, best, eps);

    // On flat prices every combination scores 0, so the first one must win the tie.
    CHECK(advisor.evaluate("FLAT", "SmaCrossover", flatBars(1000, 100)).params == grid[0]);

    // Training must never see the test window: wreck those bars and the choice stays the same.
    std::vector<Candle> poisoned = bars;
    for (int i = 700; i < 1000; i++) {
        poisoned[i] = Candle(i * 3600000LL, 1, 2000 + i * (i % 2));
    }
    ReportRow again = advisor.evaluate("NOISE", "SmaCrossover", poisoned);
    CHECK(again.params == row.params);
    CHECK_NEAR(again.trainSharpe, row.trainSharpe, eps);
    CHECK(again.testReturn != row.testReturn);  // the test window itself did change
}

void testPairCost(const Advisor& advisor) {
    // The pair pays double cost, and only the pair does.
    std::vector<Candle> ratioLike = randomWalk(9, 2000, 0.01);
    ReportRow pair = advisor.evaluate(pairAsset, "PairsSpread", ratioLike);
    CHECK(pair.testTrades > 0);
    PairsSpread twoLegs(pair.params[0], pair.params[1]);
    double atDoubleCost = Backtester(10000, 0.0024).run(twoLegs, ratioLike, 1400, 2000).totalReturn();
    double atSingleCost = Backtester(10000, 0.0012).run(twoLegs, ratioLike, 1400, 2000).totalReturn();
    CHECK_NEAR(pair.testReturn, atDoubleCost, eps);
    CHECK(std::fabs(pair.testReturn - atSingleCost) > 1e-6);
    BuyAndHold holdRatio;
    CHECK_NEAR(advisor.baseline(pairAsset, ratioLike).testReturn,
               Backtester(10000, 0.0024).run(holdRatio, ratioLike, 1400, 2000).totalReturn(), eps);
}

void testAdvisorRows() {
    CHECK(splitIndex(10) == 7);
    CHECK(splitIndex(1000) == 700);
    CHECK(splitIndex(17519) == 12263);

    Advisor advisor;
    std::vector<Candle> bars = randomWalk(7, 1000, 0.01);
    ReportRow row = advisor.evaluate("NOISE", "SmaCrossover", bars);
    CHECK(row.testBars == 300);
    CHECK(row.params.size() == 2);

    // The chosen strategy, run by hand on the test window, must give the row's numbers.
    SmaCrossover sameChoice(row.params[0], row.params[1]);
    BacktestResult byHand = Backtester().run(sameChoice, bars, 700, 1000);
    CHECK_NEAR(row.testReturn, byHand.totalReturn(), eps);
    CHECK_NEAR(row.testSharpe, byHand.sharpe(), eps);
    CHECK_NEAR(row.testDrawdown, byHand.maxDrawdown(), eps);
    CHECK(row.testTrades == byHand.trades());

    testChoiceOnTrain(advisor, bars, row);

    ReportRow base = advisor.baseline("NOISE", bars);
    CHECK(base.verdict == Verdict::Baseline);
    CHECK(base.testTrades == 1);
    CHECK_NEAR(base.testReturn, row.buyHoldReturn, eps);

    testPairCost(advisor);
}

bool hasLine(const std::string& text, const std::string& wanted) {
    return text.find(wanted) != std::string::npos;
}

void testReport() {
    Advisor advisor;
    std::vector<ReportRow> rows = advisor.recommend(randomWalk(8, 1000, 0.01), randomWalk(18, 1000, 0.01));
    CHECK(rows.size() == 8);  // 5 strategy rows and 3 baselines
    int pairRows = 0;
    int baselines = 0;
    for (int i = 0; i < 8; i++) {
        if (rows[i].asset == pairAsset) pairRows++;
        if (rows[i].verdict == Verdict::Baseline) baselines++;
    }
    CHECK(pairRows == 2);
    CHECK(baselines == 3);

    CHECK(hasLine(buildReport(rows), "5 strategy/asset combinations were tested"));

    for (int i = 0; i < 5; i++) {
        rows[i].verdict = Verdict::Reject;
    }
    std::string report = buildReport(rows);
    CHECK(hasLine(report, "No strategy passed validation. Holding buy-and-hold was as good or better."));
    CHECK(hasLine(report, "Sharpe error about +/-"));
    CHECK(hasLine(report, "combinations were tested; a lucky RECOMMENDED is possible even on pure noise"));
    CHECK(hasLine(report, "If any test Sharpe is above 3, check for lookahead before trusting it"));
    CHECK(hasLine(report, "Research only. Not financial advice."));

    // Give the worst Sharpe a Recommended: it must now be printed first, and the warning must go.
    int worst = 0;
    for (int i = 0; i < 5; i++) {
        if (rows[i].testSharpe < rows[worst].testSharpe) worst = i;
    }
    rows[worst].verdict = Verdict::Recommended;
    rows[worst].asset = "FIRSTROW";
    report = buildReport(rows);
    CHECK(!hasLine(report, "No strategy passed"));
    CHECK(report.find("RECOMMENDED") < report.find("REJECT"));
    CHECK(report.find("FIRSTROW") < report.find("REJECT"));

    const std::string path = "./TESTONLY_report.txt";
    saveReport(path, "hello\n");
    std::ifstream saved(path.c_str());
    std::string line;
    std::getline(saved, line);
    saved.close();
    CHECK(line == "hello");
    std::remove(path.c_str());
}

void testChoicesFile() {
    ReportRow row;
    row.asset = "ETHUSDT";
    row.family = "SmaCrossover";
    row.params = numbers("20 100");
    row.verdict = Verdict::Weak;

    ReportRow knn = row;
    knn.family = "KnnStrategy";
    knn.params = numbers("10 0.0002");
    knn.verdict = Verdict::Recommended;

    ReportRow base = row;
    base.verdict = Verdict::Baseline;

    std::vector<ReportRow> rows;
    rows.push_back(row);
    rows.push_back(base);
    rows.push_back(knn);
    const std::string path = "./TESTONLY_choices.csv";
    saveChoices(path, rows);

    std::ifstream saved(path.c_str());
    std::string line;
    std::getline(saved, line);
    CHECK(line == "asset,family,verdict,p1,p2,p3");
    std::getline(saved, line);
    CHECK(line == "ETHUSDT,SmaCrossover,WEAK,20,100,");
    std::getline(saved, line);
    CHECK(line == "ETHUSDT,KnnStrategy,RECOMMENDED,10,0.0002,");  // the baseline is not a choice
    CHECK(!std::getline(saved, line));
    saved.close();

    // What was written must come back, and must build the same strategy.
    std::vector<Choice> choices = loadChoices(path);
    CHECK(choices.size() == 2);
    CHECK(choices[0].asset == "ETHUSDT");
    CHECK(choices[0].family == "SmaCrossover");
    CHECK(choices[0].verdict == "WEAK");
    CHECK(choices[0].params == row.params);
    CHECK(choices[1].verdict == "RECOMMENDED");
    CHECK_NEAR(choices[1].params[1], 0.0002, eps);
    Strategy* rebuilt = makeStrategy(choices[1].family, choices[1].params);
    CHECK(rebuilt->describe() == "KnnStrategy(10, 0.0002)");
    delete rebuilt;

    std::ofstream broken(path.c_str());
    broken << "asset,family,verdict,p1,p2,p3\nBTCUSDT,SmaCrossover,WEAK,abc,100,\n";
    broken.close();
    CHECK_THROWS(loadChoices(path), DataError);

    std::remove(path.c_str());
    CHECK_THROWS(loadChoices(path), DataError);  // live mode says to run option 4 first
}

// On pure noise the Advisor must rarely say Recommended.
void testHonesty() {
    Advisor advisor;
    std::vector<std::string> families = strategyFamilies();
    int total = 0;
    int recommended = 0;
    int weak = 0;
    int insufficient = 0;
    for (int seed = 100; seed < 130; seed++) {
        std::vector<Candle> bars = randomWalk(seed, 4000, 0.005);
        for (int f = 0; f < 3; f++) {
            std::string asset = "NOISE";
            if (families[f] == "PairsSpread") asset = pairAsset;
            Verdict verdict = advisor.evaluate(asset, families[f], bars).verdict;
            total++;
            if (verdict == Verdict::Recommended) recommended++;
            if (verdict == Verdict::Weak) weak++;
            if (verdict == Verdict::Insufficient) insufficient++;
        }
    }
    std::cout << "honesty: " << recommended << " of " << total << " rows Recommended on noise, "
              << weak << " Weak, " << insufficient << " Insufficient\n";
    CHECK(total == 90);
    CHECK(recommended < 0.15 * total);
}

// The opposite case: when a real pattern exists, the Advisor must be able to say yes.
void testAdvisorCanSayYes() {
    std::vector<double> closes;
    for (int i = 0; i < 4000; i++) {
        closes.push_back(100 + 20 * std::sin(i * 2 * 3.14159265 / 120));
    }
    ReportRow row = Advisor().evaluate("WAVE", "SmaCrossover", barsFromCloses(closes));
    CHECK(row.verdict == Verdict::Recommended);
}

void testDemoUnit2() {
    std::ostringstream unit2;
    unit2Pointers(unit2);
    std::string text = unit2.str();
    CHECK(hasLine(text, "Unit 2 | static members: 2 objects alive"));
    CHECK(hasLine(text, "Unit 2 | static members: alive count back to 0 (started at 0)"));
    CHECK(hasLine(text, "Unit 2 | copy constructor: the copy starts at 100"));
    CHECK(hasLine(text, "Unit 2 | deep copy: copy changed to 999, original still 100"));
    CHECK(hasLine(text, "Unit 2 | assignment and this: assigned 100, self-assignment left 100"));
    CHECK(hasLine(text, "Unit 2 | dynamic initialization: buffer built with 3 prices"));
    CHECK(hasLine(text, "Unit 2 | method chaining: total 100"));
    CHECK(hasLine(text, "Unit 2 | sizeof: Candle 48 bytes"));
    CHECK(hasLine(text, "Unit 2 | pointers: writing through *pointer made price 101"));
    CHECK(hasLine(text, "Unit 2 | array of pointers: BuyAndHold()"));
    CHECK(hasLine(text, "Unit 2 | array of pointers: SmaCrossover(20, 100)"));
    CHECK(hasLine(text, "Unit 2 | array of pointers: PairsSpread(120, 2)"));
    CHECK(hasLine(text, "Unit 2 | function pointer: best by Sharpe is result 0, best by return is result 1"));
}

void testDemoUnit3() {
    std::ostringstream unit3;
    unit3Inheritance(unit3);
    std::string text = unit3.str();
    CHECK(hasLine(text, "Unit 3 | static binding: non-virtual call through Venue* reached Venue"));
    CHECK(hasLine(text, "Unit 3 | dynamic binding: virtual call through Venue* reached Exchange"));
    CHECK(hasLine(text, "Unit 3 | multilevel: Venue -> Exchange -> SpotExchange"));
    CHECK(hasLine(text, "Unit 3 | ambiguity resolved: ETH/BTC can be traded; ETH/BTC has a quote"));
    CHECK(hasLine(text, "Unit 3 | private inheritance: AuditLog holds 2 lines"));
    CHECK(hasLine(text, "Unit 3 | friend class: Auditor read a private balance of 250"));
    CHECK(hasLine(text, "Unit 3 | unary minus: -10 is -10"));
    CHECK(hasLine(text, "Unit 3 | unary ++: 10 becomes 11"));
    CHECK(hasLine(text, "Unit 3 | binary +: 11 + 5 is 16"));
    CHECK(hasLine(text, "Unit 3 | function overloading: add(Money) then add(double) gives 18.5"));
}

void testDemoUnit4() {
    std::ostringstream unit4;
    unit4TemplatesExceptions(unit4);
    std::string text = unit4.str();
    CHECK(hasLine(text, "Unit 4 | function template: clampTo(5, 0, 3) is 3, clampTo(1.5, 0.0, 3.0) is 1.5"));
    CHECK(hasLine(text, "Unit 4 | overloaded templates: mean of vector 3, mean of array 3"));
    CHECK(hasLine(text, "Unit 4 | typename: last price is 6"));
    CHECK(hasLine(text, "RollingWindow<int, 3> after pushing 1 to 4 is [2, 3, 4]"));
    CHECK(hasLine(text, "Unit 4 | export keyword:"));
    CHECK(hasLine(text, "Unit 4 | noexcept: midPrice(99, 101) is 100"));
    CHECK(hasLine(text, "Unit 4 | multiple catch: StalePriceError block got: price is too old"));
    CHECK(hasLine(text, "Unit 4 | multiple catch: DataError block got: bad candle"));
    CHECK(hasLine(text, "Unit 4 | exception and inheritance: QuantError& block got: no connection"));
    CHECK(hasLine(text, "Unit 4 | divide by zero: inner catch got: division by zero"));
    CHECK(hasLine(text, "Unit 4 | rethrow: outer catch got it too: division by zero"));
    CHECK(hasLine(text, "Unit 4 | destructor: member destroyed during the throw"));
    CHECK(hasLine(text, "Unit 4 | exception in a constructor: caught: feed could not start"));
    CHECK(hasLine(text, "Unit 4 | catch all: caught something unexpected"));
}

void testDemoUnit5() {
    std::ostringstream unit5;
    unit5Files(unit5);
    std::string text = unit5.str();
    CHECK(hasLine(text, "Unit 5 | disk file I/O: wrote and read back 4 lines"));
    CHECK(hasLine(text, "Unit 5 | file pointers: last line is 3000,121"));
    CHECK(hasLine(text, "missing file is_open() is 0"));
    CHECK(hasLine(text, "reading abc as a number fail() is 1"));
    CHECK(hasLine(text, "after the last line eof() is 1, bad() is 0"));
    CHECK(hasLine(text, "Unit 5 | binary object I/O: read back BTCUSDT SmaCrossover 50/200"));
    CHECK(hasLine(text, "equal: yes"));
    CHECK(hasLine(text, "Unit 5 | memory as a stream: wrote price=101.5, read 15"));
    CHECK(hasLine(text, "BTCUSDT        16.63"));  // 10 wide on the left, 10 wide on the right

    // The temp files must not be left behind.
    std::ifstream leftOverText("demo_temp.csv");
    CHECK(!leftOverText.is_open());
    std::ifstream leftOverBinary("demo_temp.dat");
    CHECK(!leftOverBinary.is_open());

    std::ostringstream all;
    runAllDemos(all);
    CHECK(hasLine(all.str(), "Unit 2 |"));
    CHECK(hasLine(all.str(), "Unit 3 |"));
    CHECK(hasLine(all.str(), "Unit 4 |"));
    CHECK(hasLine(all.str(), "Unit 5 |"));
}

int main() {
    testCandleBasics();
    testIndicators();
    testExceptions();
    testCandleStreams();
    testDataStore();
    testNeedsFullRefetch();
    testParsing();
    testValidation();
    testFetcherObject();
    testToyBacktest();
    testFills();
    testMetrics();
    testSmaAndPairs();
    testGrids();
    testLeakage();
    testKnnPrediction();
    testKnnLearning();
    testRatioSeries();
    testVerdicts();
    testAdvisorRows();
    testReport();
    testChoicesFile();
    testHonesty();
    testAdvisorCanSayYes();
    testDemoUnit2();
    testDemoUnit3();
    testDemoUnit4();
    testDemoUnit5();
    return minitest::finish();
}
