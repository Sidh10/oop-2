// unit5_files.cpp: small examples of files, file pointers, stream errors and binary records.
// Syllabus: Unit 5.

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#include "demo/demo.hpp"

namespace quant {

static const char* textFile = "demo_temp.csv";
static const char* binaryFile = "demo_temp.dat";

// Unit 5: disk file I/O, writing a small file and reading it back.
static void showWriteThenRead(std::ostream& out) {
    std::ofstream file(textFile, std::ios::binary);
    file << "openTime,close\n1000,100\n2000,110\n3000,121\n";
    file.close();

    std::ifstream in(textFile, std::ios::binary);
    std::string line;
    int lines = 0;
    while (std::getline(in, line)) {
        lines++;
    }
    out << "Unit 5 | disk file I/O: wrote and read back " << lines << " lines\n";
}

// Unit 5: file pointers, jump to the end and walk back instead of reading the whole file.
static void showFilePointers(std::ostream& out) {
    std::ifstream in(textFile, std::ios::binary);
    in.seekg(0, std::ios::end);
    long long position = static_cast<long long>(in.tellg()) - 2;  // step over the last newline
    char ch = 0;
    while (position > 0) {
        in.seekg(position);
        in.get(ch);
        if (ch == '\n') break;  // get() moved one on, so we now sit at the start of the last line
        position--;
    }
    std::string lastLine;
    std::getline(in, lastLine);
    out << "Unit 5 | file pointers: last line is " << lastLine << "\n";
}

// Unit 5: stream errors, the state flags say what went wrong.
static void showStreamErrors(std::ostream& out) {
    std::ifstream missing("no_such_file.csv");
    out << "Unit 5 | stream errors: missing file is_open() is " << missing.is_open() << "\n";

    std::istringstream text("abc");
    int number = 0;
    text >> number;
    out << "Unit 5 | stream errors: reading abc as a number fail() is " << text.fail() << "\n";

    std::ifstream in(textFile);
    std::string line;
    while (std::getline(in, line)) {
    }
    out << "Unit 5 | stream errors: after the last line eof() is " << in.eof() << ", bad() is "
        << in.bad() << "\n";
}

struct SavedChoice {
    char asset[16];
    char family[24];
    int verdict;
    double p[3];
};

// Unit 5: object data processing, a whole record goes to disk and comes back unchanged.
static void showBinaryRecord(std::ostream& out) {
    SavedChoice saved;
    std::memset(&saved, 0, sizeof(saved));
    std::strcpy(saved.asset, "BTCUSDT");
    std::strcpy(saved.family, "SmaCrossover");
    saved.verdict = 2;
    saved.p[0] = 50;
    saved.p[1] = 200;

    std::ofstream file(binaryFile, std::ios::binary);
    file.write(reinterpret_cast<const char*>(&saved), sizeof(saved));
    file.close();

    SavedChoice loaded;
    std::memset(&loaded, 0, sizeof(loaded));
    std::ifstream in(binaryFile, std::ios::binary);
    in.read(reinterpret_cast<char*>(&loaded), sizeof(loaded));
    bool equal = std::memcmp(&saved, &loaded, sizeof(saved)) == 0;
    out << "Unit 5 | binary object I/O: read back " << loaded.asset << " " << loaded.family << " "
        << loaded.p[0] << "/" << loaded.p[1] << ", " << sizeof(saved) << " bytes, equal: "
        << (equal ? "yes" : "no") << "\n";
}

// Unit 5: memory as a stream object, the same << and >> work on a string.
static void showMemoryStreams(std::ostream& out) {
    std::ostringstream written;
    written << "price=" << 101.5;
    std::istringstream read("7 8");
    int first = 0;
    int second = 0;
    read >> first >> second;
    out << "Unit 5 | memory as a stream: wrote " << written.str() << ", read " << first + second << "\n";
}

// Unit 5: printer output, setw and setprecision line values up in columns.
static void showPrinterTable(std::ostream& out) {
    std::ostringstream table;
    table << std::left << std::setw(10) << "Asset" << std::right << std::setw(10) << "Return%" << "\n"
          << std::left << std::setw(10) << "BTCUSDT" << std::right << std::setw(10) << std::fixed
          << std::setprecision(2) << 16.629 << "\n";
    out << "Unit 5 | printer output:\n" << table.str();
}

void unit5Files(std::ostream& out) {
    showWriteThenRead(out);
    showFilePointers(out);
    showStreamErrors(out);
    showBinaryRecord(out);
    showMemoryStreams(out);
    showPrinterTable(out);
    std::remove(textFile);
    std::remove(binaryFile);
}

}  // namespace quant
