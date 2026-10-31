// report.cpp: prints the report as a table, and saves and loads the chosen settings.
// Syllabus: Unit 5 printer output, memory as a stream, disk file I/O, primitive data.

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "advisor.hpp"
#include "errors.hpp"

namespace quant {

static const double barsPerMonth = 8760.0 / 12;

// Recommended rows go first, then the highest Sharpe.
static bool rowComesFirst(const ReportRow& a, const ReportRow& b) {
    bool aRecommended = a.verdict == Verdict::Recommended;
    bool bRecommended = b.verdict == Verdict::Recommended;
    if (aRecommended != bRecommended) return aRecommended;
    return a.testSharpe > b.testSharpe;
}

static std::string paramsText(const std::vector<double>& params) {
    std::ostringstream text;
    int count = params.size();
    for (int i = 0; i < count; i++) {
        if (i > 0) text << "/";
        text << params[i];
    }
    return text.str();
}

// Unit 5: printer output, setw and setprecision line the report up in columns.
static void writeRow(std::ostream& out, const ReportRow& row) {
    out << std::left << std::setw(10) << row.asset << std::setw(14) << row.family << std::setw(10)
        << paramsText(row.params) << std::right << std::fixed << std::setprecision(2)
        << std::setw(8) << row.trainSharpe << std::setw(9) << pct(row.testReturn) << std::setw(8)
        << row.testSharpe << std::setw(8) << pct(row.testDrawdown) << std::setw(7) << row.testTrades
        << std::setw(9) << pct(row.buyHoldReturn) << "  " << verdictName(row.verdict) << "\n";
}

static void writeHeader(std::ostream& out) {
    out << "Strategy report, test window only (the newest 30% of the bars)\n\n";
    out << std::left << std::setw(10) << "Asset" << std::setw(14) << "Family" << std::setw(10)
        << "Params" << std::right << std::setw(8) << "Train" << std::setw(9) << "Return%"
        << std::setw(8) << "Sharpe" << std::setw(8) << "MaxDD%" << std::setw(7) << "Trades"
        << std::setw(9) << "Hold%" << "  Verdict\n";
}

static void writeNotes(std::ostream& out, int tested, bool anyRecommended, double months) {
    out << "\n";
    if (!anyRecommended) {
        out << "No strategy passed validation. Holding buy-and-hold was as good or better.\n";
    }
    out << std::setprecision(1) << "Test window: " << months << " months; Sharpe error about +/-"
        << 1 / std::sqrt(months / 12) << "\n";
    out << tested << " strategy/asset combinations were tested; "
        << "a lucky RECOMMENDED is possible even on pure noise\n";
    out << "If any test Sharpe is above 3, check for lookahead before trusting it\n";
    out << "Research only. Not financial advice.\n";
}

std::string buildReport(const std::vector<ReportRow>& rows) {
    std::vector<ReportRow> sorted = rows;
    std::sort(sorted.begin(), sorted.end(), rowComesFirst);

    int tested = 0;
    bool anyRecommended = false;
    int count = rows.size();
    for (int i = 0; i < count; i++) {
        if (rows[i].verdict != Verdict::Baseline) tested++;
        if (rows[i].verdict == Verdict::Recommended) anyRecommended = true;
    }

    std::ostringstream report;  // Unit 5: memory as a stream object, printed and saved afterwards
    writeHeader(report);
    for (int i = 0; i < count; i++) {
        writeRow(report, sorted[i]);
    }
    writeNotes(report, tested, anyRecommended, rows.front().testBars / barsPerMonth);
    return report.str();
}

void saveReport(const std::string& path, const std::string& report) {
    std::ofstream out(path);
    if (!out.is_open()) throw DataError("cannot create " + path);
    out << report;
    if (out.fail()) throw DataError("could not write " + path);
}

void saveChoices(const std::string& path, const std::vector<ReportRow>& rows) {
    std::ofstream out(path);
    if (!out.is_open()) throw DataError("cannot create " + path);
    out << "asset,family,verdict,p1,p2,p3\n";
    int count = rows.size();
    for (int i = 0; i < count; i++) {
        if (rows[i].verdict == Verdict::Baseline) continue;
        out << rows[i].asset << ',' << rows[i].family << ',' << verdictName(rows[i].verdict);
        int paramCount = rows[i].params.size();
        for (int p = 0; p < 3; p++) {
            out << ',';
            if (p < paramCount) out << rows[i].params[p];
        }
        out << '\n';
    }
    if (out.fail()) throw DataError("could not write " + path);
}

static Choice readChoice(const std::string& line, const std::string& path) {
    std::istringstream fields(line);
    Choice choice;
    std::string value;
    std::getline(fields, choice.asset, ',');
    std::getline(fields, choice.family, ',');
    std::getline(fields, choice.verdict, ',');
    try {
        while (std::getline(fields, value, ',')) {
            if (!value.empty()) choice.params.push_back(std::stod(value));
        }
    } catch (const std::logic_error&) {
        throw DataError("bad number in " + path + ": " + line);
    }
    return choice;
}

std::vector<Choice> loadChoices(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) throw DataError(path + " is missing, run option 4 first");

    std::string line;
    std::getline(in, line);  // skip the header

    std::vector<Choice> choices;
    while (std::getline(in, line)) {
        choices.push_back(readChoice(line, path));
    }
    return choices;
}

}  // namespace quant
