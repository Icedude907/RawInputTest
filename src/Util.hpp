#pragma once

#include <string>
#include <iostream>
#include <iomanip>

inline void printStringHex(std::string str, size_t hex){
    std::cout << str << " 0x" << std::hex << hex << std::dec << std::endl;
}
