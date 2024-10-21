#include <iostream>
#include "src/USSR/tower.h"

// 假设这里是 Counters 类的定义

int main() {
    // 1-bit counters test
    Counters oneBitCounters(8, 1); // 8 counters, 1 bit each
    oneBitCounters.increment(0);
    oneBitCounters.increment(0); // Should overflow and stay at 1
    std::cout << "1-bit counter[0]: " << oneBitCounters.index(0) << " (Expected: 1)" << std::endl;

    // 2-bit counters test
    Counters twoBitCounters(4, 2); // 4 counters, 2 bits each
    for (int i = 0; i < 3; ++i) {
        twoBitCounters.increment(1);
    }
    std::cout << "2-bit counter[1]: " << twoBitCounters.index(1) << " (Expected: 3)" << std::endl;

    // 4-bit counters test
    Counters fourBitCounters(2, 4); // 2 counters, 4 bits each
    for (int i = 0; i < 9; ++i) {
        fourBitCounters.increment(0);
    }
    std::cout << "4-bit counter[0]: " << fourBitCounters.index(0) << " (Expected: 9)" << std::endl;

    // 8-bit (1 byte) counters test
    Counters eightBitCounters(2, 8); // 2 counters, 8 bits each
    for (int i = 0; i < 256; ++i) {
        eightBitCounters.increment(1);
    }
    // Note: Due to overflow, the counter should max out at 255
    std::cout << "8-bit counter[1]: " << eightBitCounters.index(1) << " (Expected: 255)" << std::endl;

    return 0;
}
