// unit2_objects.cpp: small examples of static members, deep copy, this and object size.
// Syllabus: Unit 2.

#include <sstream>

#include "candle.hpp"
#include "demo/demo.hpp"

namespace quant {

// Unit 2: static data member and static member function, one count shared by every object.
class RunCounter {
public:
    RunCounter() { alive++; }
    ~RunCounter() { alive--; }
    static int aliveCount() { return alive; }

private:
    static int alive;
};

int RunCounter::alive = 0;

static void showStaticMembers(std::ostream& out) {
    int before = RunCounter::aliveCount();
    {
        RunCounter first;
        RunCounter second;
        out << "Unit 2 | static members: " << RunCounter::aliveCount() << " objects alive\n";
    }
    out << "Unit 2 | static members: alive count back to " << RunCounter::aliveCount()
        << " (started at " << before << ")\n";
}

// Owns memory from new[], so it needs its own copy constructor, assignment and destructor.
class PriceBuffer {
public:
    PriceBuffer(int n) {
        prices = new double[n]();
        count = n;
    }

    // Unit 2: copy constructor, a deep copy gets its own array.
    PriceBuffer(const PriceBuffer& other) {
        count = other.count;
        prices = new double[count];
        for (int i = 0; i < count; i++) {
            prices[i] = other.prices[i];
        }
    }

    PriceBuffer& operator=(const PriceBuffer& other) {
        if (this == &other) return *this;  // Unit 2: this pointer, a = a must not free its own array
        double* copy = new double[other.count];
        for (int i = 0; i < other.count; i++) {
            copy[i] = other.prices[i];
        }
        delete[] prices;
        prices = copy;
        count = other.count;
        return *this;
    }

    ~PriceBuffer() { delete[] prices; }  // Unit 2: destructor frees what the constructor took

    void set(int i, double value) { prices[i] = value; }
    double get(int i) const { return prices[i]; }
    int size() const { return count; }

private:
    double* prices;
    int count;
};

static void showDeepCopy(std::ostream& out) {
    PriceBuffer original(2);
    original.set(0, 100);
    PriceBuffer copy(original);
    out << "Unit 2 | copy constructor: the copy starts at " << copy.get(0) << "\n";
    copy.set(0, 999);
    out << "Unit 2 | deep copy: copy changed to " << copy.get(0) << ", original still "
        << original.get(0) << "\n";

    PriceBuffer assigned(1);
    assigned = original;
    PriceBuffer& sameObject = original;
    original = sameObject;
    out << "Unit 2 | assignment and this: assigned " << assigned.get(0)
        << ", self-assignment left " << original.get(0) << "\n";
}

// Unit 2: dynamic initialization, the object's size is only known at run time.
static void showDynamicInitialization(std::ostream& out) {
    std::istringstream typed("3");  // stands in for cin, so the demo never waits for a key
    int size = 0;
    typed >> size;
    PriceBuffer buffer(size);
    out << "Unit 2 | dynamic initialization: buffer built with " << buffer.size() << " prices\n";
}

// Unit 2: this pointer, returning *this lets calls be chained.
class OrderTicket {
public:
    OrderTicket() {
        quantity = 0;
        price = 0;
    }
    OrderTicket& setQuantity(double value) {
        quantity = value;
        return *this;
    }
    OrderTicket& setPrice(double value) {
        price = value;
        return *this;
    }
    double total() const { return quantity * price; }

private:
    double quantity;
    double price;
};

void unit2Objects(std::ostream& out) {
    showStaticMembers(out);
    showDeepCopy(out);
    showDynamicInitialization(out);
    out << "Unit 2 | method chaining: total " << OrderTicket().setQuantity(2).setPrice(50).total() << "\n";
    // Unit 2: objects and memory requirements, an object is as big as its data members.
    out << "Unit 2 | sizeof: Candle " << sizeof(Candle) << " bytes, PriceBuffer "
        << sizeof(PriceBuffer) << " bytes (the array is elsewhere)\n";
}

}  // namespace quant
