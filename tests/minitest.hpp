// minitest.hpp: a very small test harness that counts checks and prints each failure.
// minitest::finish() gives the exit code for main.

#ifndef MINITEST_HPP
#define MINITEST_HPP

#include <cmath>
#include <iostream>

namespace minitest {

static int checks = 0;
static int failures = 0;

static void report(bool passed, const char* text, const char* file, int line) {
    checks++;
    if (!passed) {
        failures++;
        std::cout << "FAIL " << file << ":" << line << "  " << text << "\n";
    }
}

static bool isNear(double a, double b, double tolerance) {
    return std::fabs(a - b) <= tolerance;
}

static int finish() {
    std::cout << checks << " checks, " << failures << " failures\n";
    if (failures == 0) return 0;
    return 1;
}

}  // namespace minitest

// These are macros so a failure can print the checked expression and its file and line.
#define CHECK(expr) minitest::report((expr), #expr, __FILE__, __LINE__)

#define CHECK_NEAR(a, b, tolerance) \
    minitest::report(minitest::isNear((a), (b), (tolerance)), #a " near " #b, __FILE__, __LINE__)

#define CHECK_THROWS(expr, ExceptionType) \
    do { \
        bool threw = false; \
        try { \
            expr; \
        } catch (const ExceptionType&) { \
            threw = true; \
        } catch (...) { \
        } \
        minitest::report(threw, #expr " throws " #ExceptionType, __FILE__, __LINE__); \
    } while (0)

#endif
