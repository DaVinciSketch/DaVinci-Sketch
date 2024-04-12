#include <iostream>

int main() {
    uint8_t uintValue = 0b10000001; // uint8 value with first bit set to 1
    std::cout << "uintValue: " << (uint8_t)uintValue << std::endl;
    int8_t intValue = (uintValue); // assign uint8 value to int8 variable
    int32_t intValue3 = intValue; // assign uint8 value to int32 variable
    int32_t intValue0 = static_cast<int32_t>(uintValue); // assign uint8 value to int32 variable
    int32_t intValue1 = static_cast<int32_t>(uintValue); // assign uint8 value to int32 variable
    int32_t intValue2 = (int)(uintValue); // assign uint8 value to int32 variable
    

    std::cout << "Assigned value: " << intValue << std::endl;
    std::cout << "Assigned value: " << intValue3 << std::endl;
    std::cout << "Assigned value: " << intValue0 << std::endl;
    std::cout << "Assigned value: " << intValue1 << std::endl;
    std::cout << "Assigned value: " << intValue2 << std::endl;

    return 0;
}
