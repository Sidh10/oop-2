// strategy.cpp: creating strategies by name and listing their settings.
// Syllabus: Unit 2 new and pointer to derived class, Unit 5 memory as a stream.

#include "strategy.hpp"

#include <sstream>

#include "errors.hpp"

namespace quant {

std::string Strategy::describe() const {
    std::vector<double> values = params();
    std::ostringstream text;
    text << name() << '(';
    int count = values.size();
    for (int i = 0; i < count; i++) {
        if (i > 0) text << ", ";
        text << values[i];
    }
    text << ')';
    return text.str();
}

// Unit 2: pointer to derived class, each new object is handed back as a Strategy*.
Strategy* makeStrategy(const std::string& family, const std::vector<double>& p) {
    int count = p.size();
    if (family == "BuyAndHold" && count == 0) return new BuyAndHold();
    if (family == "SmaCrossover" && count == 2) return new SmaCrossover(p[0], p[1]);
    if (family == "KnnStrategy" && count == 2) return new KnnStrategy(p[0], p[1]);
    if (family == "PairsSpread" && count == 2) return new PairsSpread(p[0], p[1]);
    throw QuantError("unknown strategy or wrong parameters: " + family);
}

std::vector<std::string> strategyFamilies() {
    std::vector<std::string> families;
    families.push_back("SmaCrossover");
    families.push_back("KnnStrategy");
    families.push_back("PairsSpread");
    return families;
}

static void addSetting(std::vector<std::vector<double>>& grid, double first, double second) {
    std::vector<double> setting;
    setting.push_back(first);
    setting.push_back(second);
    grid.push_back(setting);
}

std::vector<std::vector<double>> paramGrid(const std::string& family) {
    std::vector<std::vector<double>> grid;
    if (family == "SmaCrossover") {
        double fasts[3] = {10, 20, 50};
        double slows[3] = {50, 100, 200};
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (fasts[i] < slows[j]) addSetting(grid, fasts[i], slows[j]);
            }
        }
    } else if (family == "KnnStrategy") {
        addSetting(grid, 10, 0.0002);
        addSetting(grid, 10, 0.0005);
        addSetting(grid, 30, 0.0002);
        addSetting(grid, 30, 0.0005);
    } else if (family == "PairsSpread") {
        double windows[3] = {48, 120, 240};
        for (int i = 0; i < 3; i++) {
            addSetting(grid, windows[i], 1.5);
            addSetting(grid, windows[i], 2.0);
        }
    } else {
        throw QuantError("no parameter grid for " + family);
    }
    return grid;
}

}  // namespace quant
