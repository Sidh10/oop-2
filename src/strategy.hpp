// strategy.hpp: the abstract Strategy class and its four children.
// Syllabus: Unit 1 abstraction, Unit 3 abstract class, inheritance, virtual functions.


#ifndef STRATEGY_HPP
#define STRATEGY_HPP

#include <deque>
#include <string>
#include <vector>

#include "candle.hpp"

namespace quant {

// Unit 3: abstract class, the pure virtual functions mean it cannot be created directly.
// Unit 1: abstraction, the backtester only ever talks to this interface.
class Strategy {
public:
    Strategy() { position = FLAT; }
    virtual ~Strategy() {}  // Unit 3: virtual destructor, so delete through Strategy* reaches the child

    virtual std::string name() const = 0;  // Unit 3: pure virtual function
    virtual std::vector<double> params() const = 0;

    // Decided at the close of bar i. May only read bars[0..i].
    virtual Position signal(const std::vector<Candle>& bars, int i) = 0;

    virtual void reset() { position = FLAT; }
    virtual void warmUp(const std::vector<Candle>&, int) {}  // most strategies have nothing to learn
    std::string describe() const;

protected:
    Position position;  // Unit 3: protected member, children read and change it directly
};

// Unit 3: hierarchical inheritance, all four strategies derive directly from Strategy.
class BuyAndHold : public Strategy {
public:
    std::string name() const override { return "BuyAndHold"; }
    std::vector<double> params() const override;
    Position signal(const std::vector<Candle>& bars, int i) override;
};

class SmaCrossover : public Strategy {
public:
    SmaCrossover(int fastLength, int slowLength);
    std::string name() const override { return "SmaCrossover"; }
    std::vector<double> params() const override;
    Position signal(const std::vector<Candle>& bars, int i) override;

private:
    int fast;
    int slow;
};

// The statistical arbitrage: buys the ETH/BTC ratio when it is cheap against its recent
// average and sells when it is back at that average.
class PairsSpread : public Strategy {
public:
    PairsSpread(int windowLength, double entryLevel);
    std::string name() const override { return "PairsSpread"; }
    std::vector<double> params() const override;
    Position signal(const std::vector<Candle>& bars, int i) override;

private:
    int window;
    double entryZ;
};

const int situationSize = 5;

// One remembered example: this situation was followed by this return.
struct Sample {
    double x[situationSize];  // the last 5 one-bar log returns, newest first
    double label;  // the log return of the next bar
};

// The "AI": find the k most similar past situations and see what the price did next.
class KnnStrategy : public Strategy {
public:
    KnnStrategy(int neighbours, double edge);
    std::string name() const override { return "KnnStrategy"; }
    std::vector<double> params() const override;
    Position signal(const std::vector<Candle>& bars, int i) override;
    void reset() override;  // Unit 3: overriding, the memory has to be cleared as well
    void warmUp(const std::vector<Candle>& bars, int begin) override;

    void remember(const Sample& sample);
    double predict(const double situation[situationSize]) const;
    const std::deque<Sample>& samples() const { return memory; }

private:
    void learn(const std::vector<Candle>& bars, int i);

    int k;
    double minEdge;
    std::deque<Sample> memory;  // the newest 1000 samples, which is the online learning
};

// Unit 2: new, the caller runs the strategy and then deletes it.
Strategy* makeStrategy(const std::string& family, const std::vector<double>& params);

std::vector<std::string> strategyFamilies();

// Every combination of settings to try for this family.
std::vector<std::vector<double>> paramGrid(const std::string& family);

}  // namespace quant

#endif
