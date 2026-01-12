#include "time.h"

#include <chrono>

double CurrentTime() {
    using namespace std::chrono;
    constexpr double MICROSECONDS_TO_SECONDS = 1.0 / 1e6;
    auto Now = system_clock::now();
    auto Microseconds = duration_cast<microseconds>(Now.time_since_epoch());
    long long MicrosecondCount = Microseconds.count();
    return (double)MicrosecondCount * MICROSECONDS_TO_SECONDS;
}