// unit3_inheritance.cpp: small examples of binding, inheritance types and operator overloading.
// Syllabus: Unit 1 static and dynamic binding, Unit 3.

#include <string>
#include <vector>

#include "demo/demo.hpp"

namespace quant {

class Venue {
public:
    virtual ~Venue() {}
    std::string staticName() const { return "Venue"; }
    virtual std::string dynamicName() const { return "Venue"; }
};

// Unit 3: single and public inheritance, one public base class.
class Exchange : public Venue {
public:
    std::string staticName() const { return "Exchange"; }
    std::string dynamicName() const override { return "Exchange"; }
};

// Unit 3: multilevel inheritance, Venue -> Exchange -> SpotExchange.
class SpotExchange : public Exchange {
public:
    std::string dynamicName() const override { return "SpotExchange"; }
};

// Unit 1: static binding uses the pointer's type, dynamic binding uses the real object.
static void showBinding(std::ostream& out) {
    Exchange exchange;
    Venue* venue = &exchange;
    out << "Unit 3 | static binding: non-virtual call through Venue* reached " << venue->staticName() << "\n";
    out << "Unit 3 | dynamic binding: virtual call through Venue* reached " << venue->dynamicName() << "\n";
    SpotExchange spot;
    venue = &spot;
    out << "Unit 3 | multilevel: Venue -> Exchange -> " << venue->dynamicName() << "\n";
}

// Unit 3: virtual base class, shared by the two classes below instead of being copied twice.
class Instrument {
public:
    Instrument(const std::string& name = "unknown") { symbol = name; }
    virtual ~Instrument() {}
    virtual std::string describe() const = 0;

protected:
    std::string symbol;
};

class Tradable : virtual public Instrument {
public:
    std::string describe() const override { return symbol + " can be traded"; }
};

class Quotable : virtual public Instrument {
public:
    std::string describe() const override { return symbol + " has a quote"; }
};

// Unit 3: multiple and hybrid inheritance, two parents that share one grandparent.
class CryptoPair : public Tradable, public Quotable {
public:
    CryptoPair() : Instrument("ETH/BTC") {}

    // Unit 3: ambiguity, both parents define describe(), so we must name the one we mean.
    std::string describe() const override { return Tradable::describe() + "; " + Quotable::describe(); }
};

// Unit 3: private inheritance, built on vector but offering only add and count.
class AuditLog : private std::vector<std::string> {
public:
    void add(const std::string& line) { push_back(line); }
    int count() const { return size(); }
};

class Auditor;

class Wallet {
public:
    Wallet(double amount) { balance = amount; }

private:
    friend class Auditor;  // Unit 3: friend class, Auditor may read the private balance
    double balance;
};

class Auditor {
public:
    double inspect(const Wallet& wallet) const { return wallet.balance; }
};

class Money {
public:
    Money(double value) { amount = value; }

    Money operator-() const { return Money(-amount); }  // Unit 3: overloading a unary operator

    Money& operator++() {  // Unit 3: overloading unary prefix ++
        amount += 1;
        return *this;
    }

    Money operator+(const Money& other) const { return Money(amount + other.amount); }  // Unit 3: binary operator

    // Unit 3: function overloading, the argument type picks the version.
    void add(const Money& other) { amount += other.amount; }
    void add(double extra) { amount += extra; }

    double value() const { return amount; }

private:
    double amount;
};

static void showOperators(std::ostream& out) {
    Money ten(10);
    Money five(5);
    out << "Unit 3 | unary minus: -10 is " << (-ten).value() << "\n";
    out << "Unit 3 | unary ++: 10 becomes " << (++ten).value() << "\n";
    out << "Unit 3 | binary +: 11 + 5 is " << (ten + five).value() << "\n";
    ten.add(five);
    ten.add(2.5);
    out << "Unit 3 | function overloading: add(Money) then add(double) gives " << ten.value() << "\n";
}

void unit3Inheritance(std::ostream& out) {
    showBinding(out);
    CryptoPair pair;
    out << "Unit 3 | ambiguity resolved: " << pair.describe() << "\n";
    AuditLog log;
    log.add("first entry");
    log.add("second entry");
    out << "Unit 3 | private inheritance: AuditLog holds " << log.count() << " lines\n";
    out << "Unit 3 | friend class: Auditor read a private balance of " << Auditor().inspect(Wallet(250)) << "\n";
    showOperators(out);
}

}  // namespace quant
