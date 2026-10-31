// errors.hpp: the error types used by the whole program.
// Syllabus: Unit 4 user-defined exceptions and exceptions with inheritance.

#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <stdexcept>
#include <string>

namespace quant {

// Unit 4: user-defined exception, catch (QuantError&) also catches every child below.
class QuantError : public std::runtime_error {
public:
    QuantError(const std::string& message) : std::runtime_error(message) {}
};

class NetworkError : public QuantError {
public:
    // httpStatus stays 0 when the failure happened before any HTTP reply arrived.
    NetworkError(const std::string& message, long httpStatus = 0)
        : QuantError(message), status(httpStatus) {}

    long httpStatus() const { return status; }

private:
    long status;
};

class DataError : public QuantError {
public:
    DataError(const std::string& message) : QuantError(message) {}
};

// Only the concepts demo throws this one.
class DivideByZeroError : public QuantError {
public:
    DivideByZeroError(const std::string& message) : QuantError(message) {}
};

}  // namespace quant

#endif
