// candle.hpp: one hourly price bar, plus the Position and Verdict enums.
// Syllabus: Unit 1 structure, enumeration, Unit 2 constructors, friends, Unit 5 << and >>.


#ifndef CANDLE_HPP
#define CANDLE_HPP

#include <iostream>
#include <string>

namespace quant {

// Unit 1: enumeration, a plain enum so FLAT and LONG read like ordinary constants.
enum Position { FLAT = 0, LONG = 1 };

// Unit 1: enumeration, enum class keeps the names inside Verdict.
enum class Verdict { Recommended, Weak, Reject, Insufficient, Baseline };

std::string verdictName(Verdict verdict);

// Gives "2025-03-01 14:00" in UTC from a Binance time in milliseconds.
std::string timeText(long long timeMs);

// Unit 2: inline function, small enough to be copied into the place it is called.
inline double pct(double x) {
    return x * 100;
}

// Unit 1: structure, one hourly price bar.
struct Candle {
    long long openTime;
    double open;
    double high;
    double low;
    double close;
    double volume;

    // Unit 2: default constructor.
    Candle();

    // Unit 2: parameterised constructor with default arguments. A high or low of 0 means
    // "not given", because real prices are above 0, so it uses the open and close instead.
    Candle(long long time, double o, double c, double h = 0, double l = 0, double v = 0);

    std::string toText() const;

    // Unit 2: friend functions. Unit 5: overloading the insertion and extraction operators.
    friend std::ostream& operator<<(std::ostream& out, const Candle& candle);
    friend std::istream& operator>>(std::istream& in, Candle& candle);
};

}  // namespace quant

#endif
