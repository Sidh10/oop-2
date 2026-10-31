// main.cpp: the menu, the command-line options and the last-resort error handling.
// Syllabus: Unit 4 multiple catch blocks and catch (...), Unit 5 command-line arguments.

#include <exception>
#include <iostream>
#include <string>

#include "commands.hpp"
#include "demo/demo.hpp"
#include "errors.hpp"

using namespace quant;

static void printUsage() {
    std::cout << "Usage: quant_advisor [--download | --summary | --recommend | --live | --demo | --help]\n"
              << "With no option it opens the menu.\n";
}

static void printMenu() {
    std::cout << "\n1) Download / update data     4) Recommend strategies (train/test)\n"
              << "2) Show data summary          5) Live signals\n"
              << "3) Backtest one strategy      6) OOP concepts demo\n"
              << "0) Exit\n";
}

static void runChoice(int choice, const DataStore& store) {
    if (choice == 1) downloadData(store);
    if (choice == 2) showSummary(store);
    if (choice == 3) backtestOne(store);
    if (choice == 4) recommend(store);
    if (choice == 5) liveSignals();
    if (choice == 6) runAllDemos(std::cout);
}

static void runMenu(const DataStore& store) {
    while (true) {
        printMenu();
        int choice = askNumber("Choose: ", 6);
        if (std::cin.eof() || choice == 0) return;  // end of input also exits, so this cannot spin
        if (choice < 0) {
            std::cout << "Please type a number from 0 to 6.\n";
            continue;
        }
        // A failed step should not end the session, so the error stops here.
        try {
            runChoice(choice, store);
        } catch (const QuantError& error) {
            std::cout << "Error: " << error.what() << "\n";
        }
    }
}

// Unit 5: command-line arguments, argv is an array of C strings.
static int runProgram(int argc, char* argv[]) {
    DataStore store;
    if (argc == 1) {
        runMenu(store);
        return 0;
    }
    std::string option = argv[1];
    if (argc == 2 && option == "--download") {
        downloadData(store);
    } else if (argc == 2 && option == "--summary") {
        showSummary(store);
    } else if (argc == 2 && option == "--recommend") {
        recommend(store);
    } else if (argc == 2 && option == "--live") {
        liveSignals();
    } else if (argc == 2 && option == "--demo") {
        runAllDemos(std::cout);
    } else if (argc == 2 && option == "--help") {
        printUsage();
    } else {
        printUsage();
        return 2;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    // Unit 4: multiple catch blocks, children first, because a QuantError& would catch them all.
    try {
        return runProgram(argc, argv);
    } catch (const NetworkError& error) {
        std::cout << "Network error: " << error.what() << "\n";
    } catch (const DataError& error) {
        std::cout << "Data error: " << error.what() << "\n";
    } catch (const QuantError& error) {
        std::cout << "Error: " << error.what() << "\n";
    } catch (const std::exception& error) {
        std::cout << "Unexpected error: " << error.what() << "\n";
    } catch (...) {  // Unit 4: unexpected exceptions, anything thrown still ends with a message
        std::cout << "Unknown error.\n";
    }
    return 1;
}
