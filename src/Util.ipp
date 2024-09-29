#pragma once

#include <stdint.h>
#include <string>
#include <iostream>
#include <iomanip>

void printStringHex(std::string str, size_t hex){
    std::cout << str << " 0x" << std::hex << hex << std::dec << std::endl;
}
