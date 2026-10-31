// unit4_templates.cpp: small examples of function and class templates.
// Syllabus: Unit 4.

#include <vector>

#include "demo/demo.hpp"

namespace quant {

// Unit 4: function template, works for any type that has <.
template <typename T>
T clampTo(T value, T lowest, T highest) {
    if (value < lowest) return lowest;
    if (highest < value) return highest;
    return value;
}

// Unit 4: overloading function templates, one takes a pointer and a length.
template <typename T>
double mean(const T* values, int count) {
    double sum = 0;
    for (int i = 0; i < count; i++) {
        sum += values[i];
    }
    return sum / count;
}

// Unit 4: overloading function templates, this one takes a whole vector.
template <typename T>
double mean(const std::vector<T>& values) {
    return mean(values.data(), values.size());
}

// Unit 4: the typename keyword, it tells the compiler this name is a type.
template <typename T>
T lastOf(const std::vector<T>& values) {
    typename std::vector<T>::const_iterator last = values.end();
    last--;
    return *last;
}

// Unit 4: class template with a non-type parameter, N is the fixed capacity.
template <typename T, int N>
class RollingWindow {
public:
    RollingWindow() {
        oldest = 0;
        count = 0;
    }

    void push(const T& value) {
        data[(oldest + count) % N] = value;
        if (count < N) {
            count++;
        } else {
            oldest = (oldest + 1) % N;  // full, so the oldest value was just overwritten
        }
    }

    // Unit 4: template friend, a generic function that may read the private array.
    friend std::ostream& operator<<(std::ostream& out, const RollingWindow<T, N>& window) {
        out << '[';
        for (int i = 0; i < window.count; i++) {
            if (i > 0) out << ", ";
            out << window.data[(window.oldest + i) % N];
        }
        return out << ']';
    }

private:
    T data[N];
    int oldest;
    int count;
};

static void showTemplates(std::ostream& out) {
    out << "Unit 4 | function template: clampTo(5, 0, 3) is " << clampTo(5, 0, 3)
        << ", clampTo(1.5, 0.0, 3.0) is " << clampTo(1.5, 0.0, 3.0) << "\n";

    double array[4] = {1, 2, 3, 6};
    std::vector<double> prices;
    for (int i = 0; i < 4; i++) {
        prices.push_back(array[i]);
    }
    out << "Unit 4 | overloaded templates: mean of vector " << mean(prices) << ", mean of array "
        << mean(array, 4) << "\n";
    out << "Unit 4 | typename: last price is " << lastOf(prices) << "\n";

    RollingWindow<int, 3> window;
    for (int i = 1; i <= 4; i++) {
        window.push(i);
    }
    out << "Unit 4 | class template: RollingWindow<int, 3> after pushing 1 to 4 is " << window << "\n";
    out << "Unit 4 | export keyword: removed from C++\n";
}

void unit4TemplatesExceptions(std::ostream& out) {
    showTemplates(out);
    unit4Exceptions(out);
}

}  // namespace quant
