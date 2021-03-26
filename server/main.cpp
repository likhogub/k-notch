#include "TCPSocket.cpp"
#include <thread>
#include <atomic>
#include <chrono>

#define MINUTES 15
#define DEVICES 6
using namespace std;

// Measurement structure
struct Record {
    int id;
    float ax, ay, az, gx, gy, gz, mx, my, mz;
};

// Measurements array
Record storage[MINUTES*60*1000][DEVICES];

// Measuring state
atomic<bool> isMeasuring (false);

long int getMS() {
    return chrono::duration<<long int, std::ratio<1, 1000000000>>::rep / 1000000;
}

int main() {
    isMeasuring = true;
    return 0;
}