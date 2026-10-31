// candle.cpp: reading and writing candles as CSV text, and printing them for people.
// Syllabus: Unit 2 constructors, Unit 5 stream operators and memory as a stream.

#include "candle.hpp"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace quant {

std::string verdictName(Verdict verdict) {
    switch (verdict) {
        case Verdict::Recommended: return "RECOMMENDED";
        case Verdict::Weak: return "WEAK";
        case Verdict::Reject: return "REJECT";
        case Verdict::Insufficient: return "INSUFFICIENT";
        case Verdict::Baseline: return "BASELINE";
    }
    return "UNKNOWN";
}

std::string timeText(long long timeMs) {
    std::time_t seconds = timeMs / 1000;
    std::ostringstream text;
    text << std::put_time(std::gmtime(&seconds), "%Y-%m-%d %H:%M");
    return text.str();
}

Candle::Candle() {
    openTime = 0;
    open = 0;
    high = 0;
    low = 0;
    close = 0;
    volume = 0;
}

Candle::Candle(long long time, double o, double c, double h, double l, double v) {
    openTime = time;
    open = o;
    close = c;
    high = h;
    low = l;
    volume = v;
    if (high == 0) {
        high = open;
        if (close > high) high = close;
    }
    if (low == 0) {
        low = open;
        if (close < low) low = close;
    }
}

std::string Candle::toText() const {
    std::ostringstream text;
    text << timeText(openTime) << std::setprecision(10) << " O=" << open << " C=" << close;
    return text.str();
}

std::ostream& operator<<(std::ostream& out, const Candle& candle) {
    // 17 digits always read back as the same double, so the CSV cache loses nothing.
    std::streamsize oldPrecision = out.precision(17);
    out << candle.openTime << ',' << candle.open << ',' << candle.high << ',' << candle.low << ','
        << candle.close << ',' << candle.volume;
    out.precision(oldPrecision);
    return out;
}

std::istream& operator>>(std::istream& in, Candle& candle) {
    std::string line;
    if (!std::getline(in, line)) return in;

    // Commas become spaces so >> can read the fields one after another.
    int length = line.size();
    for (int i = 0; i < length; i++) {
        if (line[i] == ',') line[i] = ' ';
    }

    std::istringstream fields(line);  // Unit 5: memory as a stream object
    Candle parsed;
    fields >> parsed.openTime >> parsed.open >> parsed.high >> parsed.low >> parsed.close >>
        parsed.volume;

    if (fields.fail()) {
        in.setstate(std::ios::failbit);  // Unit 5: a bad line becomes a stream error
    } else {
        candle = parsed;
    }
    return in;
}

}  // namespace quant
