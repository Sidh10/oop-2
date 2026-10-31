// unit2_pointers.cpp: small examples of pointers, arrays of pointers and function pointers.
// Syllabus: Unit 2.

#include <vector>

#include "backtester.hpp"
#include "demo/demo.hpp"

namespace quant {

// Unit 2: pointers, declaring one and using the indirection operator *.
static void showPointerBasics(std::ostream& out) {
    double price = 100;
    double* pointer = &price;
    *pointer = 101;
    out << "Unit 2 | pointers: writing through *pointer made price " << price << "\n";
}

// Unit 2: passing pointers to functions, list points at the first Strategy pointer.
static void printAll(std::ostream& out, Strategy** list, int count) {
    for (int i = 0; i < count; i++) {
        out << "Unit 2 | array of pointers: " << list[i]->describe() << "\n";
    }
}

// Unit 2: array of pointers and pointer to pointer, three different strategies in one list.
static void showArrayOfPointers(std::ostream& out) {
    Strategy** list = new Strategy*[3];
    list[0] = new BuyAndHold();
    list[1] = new SmaCrossover(20, 100);
    list[2] = new PairsSpread(120, 2);
    printAll(out, list, 3);
    for (int i = 0; i < 3; i++) {
        delete list[i];
    }
    delete[] list;
}

// Unit 2: function pointer, the caller chooses how results are ranked.
typedef double (*ScoreFn)(const BacktestResult&);

static double bySharpe(const BacktestResult& result) {
    return result.sharpe();
}

static double byReturn(const BacktestResult& result) {
    return result.totalReturn();
}

static int bestOf(const std::vector<BacktestResult>& results, ScoreFn score) {
    int best = 0;
    int count = results.size();
    for (int i = 1; i < count; i++) {
        if (score(results[i]) > score(results[best])) best = i;
    }
    return best;
}

static void showFunctionPointer(std::ostream& out) {
    std::vector<double> noEquity;
    std::vector<BacktestResult> results;
    results.push_back(BacktestResult(0.10, 0.05, 1.5, 12, noEquity));
    results.push_back(BacktestResult(0.40, 0.30, 0.8, 12, noEquity));
    results.push_back(BacktestResult(0.20, 0.10, 1.0, 12, noEquity));
    out << "Unit 2 | function pointer: best by Sharpe is result " << bestOf(results, bySharpe)
        << ", best by return is result " << bestOf(results, byReturn) << "\n";
}

void unit2Pointers(std::ostream& out) {
    unit2Objects(out);
    showPointerBasics(out);
    showArrayOfPointers(out);
    showFunctionPointer(out);
}

}  // namespace quant
