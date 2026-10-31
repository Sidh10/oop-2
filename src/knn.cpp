// knn.cpp: the k-nearest-neighbours strategy, which learns from past price patterns.
// Syllabus: Unit 1 arrays and structures, Unit 3 overriding reset and warmUp.

#include <algorithm>
#include <cmath>

#include "strategy.hpp"

namespace quant {

static const int memorySize = 1000;

struct Neighbour {
    double distance;
    double label;
};

static bool isCloser(const Neighbour& a, const Neighbour& b) {
    return a.distance < b.distance;
}

static double logReturn(const std::vector<Candle>& bars, int i) {
    return std::log(bars[i].close / bars[i - 1].close);
}

static void readSituation(const std::vector<Candle>& bars, int i, double situation[situationSize]) {
    for (int j = 0; j < situationSize; j++) {
        situation[j] = logReturn(bars, i - j);
    }
}

static double squaredDistance(const double a[situationSize], const double b[situationSize]) {
    double sum = 0;
    for (int j = 0; j < situationSize; j++) {
        sum += (a[j] - b[j]) * (a[j] - b[j]);
    }
    return sum;
}

KnnStrategy::KnnStrategy(int neighbours, double edge) {
    k = neighbours;
    minEdge = edge;
}

std::vector<double> KnnStrategy::params() const {
    std::vector<double> values;
    values.push_back(k);
    values.push_back(minEdge);
    return values;
}

Position KnnStrategy::signal(const std::vector<Candle>& bars, int i) {
    learn(bars, i);
    int remembered = memory.size();
    if (i < situationSize || remembered < k) return FLAT;

    double situation[situationSize];
    readSituation(bars, i, situation);
    if (predict(situation) > minEdge) {
        position = LONG;
    } else {
        position = FLAT;
    }
    return position;
}

void KnnStrategy::reset() {
    Strategy::reset();
    memory.clear();
}

// Replays only bars before begin, so the test window starts with a trained memory and no
// knowledge of itself. Anything older than the last 1000 bars would fall straight out.
void KnnStrategy::warmUp(const std::vector<Candle>& bars, int begin) {
    int first = begin - memorySize;
    if (first < 0) first = 0;
    for (int i = first; i < begin; i++) {
        learn(bars, i);
    }
}

void KnnStrategy::remember(const Sample& sample) {
    memory.push_back(sample);
    int remembered = memory.size();
    if (remembered > memorySize) memory.pop_front();
}

double KnnStrategy::predict(const double situation[situationSize]) const {
    int remembered = memory.size();
    std::vector<Neighbour> neighbours;
    for (int s = 0; s < remembered; s++) {
        Neighbour neighbour;
        neighbour.distance = squaredDistance(situation, memory[s].x);
        neighbour.label = memory[s].label;
        neighbours.push_back(neighbour);
    }

    int used = k;
    if (used > remembered) used = remembered;
    if (used == 0) return 0;
    std::sort(neighbours.begin(), neighbours.end(), isCloser);

    double sum = 0;
    for (int n = 0; n < used; n++) {
        sum += neighbours[n].label;
    }
    return sum / used;
}

// At the close of bar i the return of bar i is known, so it can label the situation of bar i-1.
void KnnStrategy::learn(const std::vector<Candle>& bars, int i) {
    if (i < situationSize + 1) return;  // bar i-1 needs 5 returns behind it
    Sample sample;
    readSituation(bars, i - 1, sample.x);
    sample.label = logReturn(bars, i);
    remember(sample);
}

}  // namespace quant
