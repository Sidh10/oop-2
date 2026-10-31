// unit4_exceptions.cpp: small examples of exception handling.
// Syllabus: Unit 4.

#include <string>

#include "demo/demo.hpp"
#include "errors.hpp"

namespace quant {

// Unit 4: simple exception handling, divide by zero raises our own error type.
static double safeDivide(double a, double b) {
    if (b == 0) throw DivideByZeroError("division by zero");
    return a / b;
}

// Unit 4: exception specification, noexcept promises this function never throws.
static double midPrice(double bid, double ask) noexcept {
    return (bid + ask) / 2;
}

// Unit 4: user-defined exception, derived from QuantError.
class StalePriceError : public QuantError {
public:
    StalePriceError(const std::string& message) : QuantError(message) {}
};

static void raiseError(int kind) {
    if (kind == 0) throw StalePriceError("price is too old");
    if (kind == 1) throw DataError("bad candle");
    throw NetworkError("no connection");
}

// Unit 4: multiple catch blocks, the first one that matches wins, so children go first.
static void showMultipleCatch(std::ostream& out) {
    for (int kind = 0; kind < 3; kind++) {
        try {
            raiseError(kind);
        } catch (const StalePriceError& error) {
            out << "Unit 4 | multiple catch: StalePriceError block got: " << error.what() << "\n";
        } catch (const DataError& error) {
            out << "Unit 4 | multiple catch: DataError block got: " << error.what() << "\n";
        } catch (const QuantError& error) {  // Unit 4: exception and inheritance, reached by NetworkError
            out << "Unit 4 | exception and inheritance: QuantError& block got: " << error.what() << "\n";
        }
    }
}

// Unit 4: rethrowing, this level reports the error and passes it on.
static void showRethrow(std::ostream& out) {
    try {
        try {
            safeDivide(1, 0);
        } catch (const DivideByZeroError& error) {
            out << "Unit 4 | divide by zero: inner catch got: " << error.what() << "\n";
            throw;
        }
    } catch (const QuantError& error) {
        out << "Unit 4 | rethrow: outer catch got it too: " << error.what() << "\n";
    }
}

class FeedLog {
public:
    FeedLog(std::ostream& stream) : out(stream) { out << "Unit 4 | constructor: member built\n"; }
    ~FeedLog() { out << "Unit 4 | destructor: member destroyed during the throw\n"; }

private:
    std::ostream& out;
};

// Unit 4: exception in a constructor, members already built are still destroyed.
class PriceFeed {
public:
    PriceFeed(std::ostream& out) : log(out) { throw DataError("feed could not start"); }

private:
    FeedLog log;
};

static void showConstructorThrow(std::ostream& out) {
    try {
        PriceFeed feed(out);
    } catch (const DataError& error) {
        out << "Unit 4 | exception in a constructor: caught: " << error.what() << "\n";
    }
}

// Unit 4: unexpected exceptions, catch (...) takes anything, even a plain int.
static void showCatchAll(std::ostream& out) {
    try {
        throw 42;
    } catch (...) {
        out << "Unit 4 | catch all: caught something unexpected\n";
    }
}

void unit4Exceptions(std::ostream& out) {
    out << "Unit 4 | noexcept: midPrice(99, 101) is " << midPrice(99, 101) << "\n";
    showMultipleCatch(out);
    showRethrow(out);
    showConstructorThrow(out);
    showCatchAll(out);
}

}  // namespace quant
