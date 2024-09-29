#pragma once

#include <stdint.h>
#include <string>
#include <iostream>
#include <iomanip>

using u8 = uint8_t; using u16 = uint16_t; using u32 = uint32_t; using u64 = uint64_t;
using i8 =  int8_t; using i16 =  int16_t; using i32 =  int32_t; using i64 =  int64_t;

void printStringHex(std::string str, size_t hex){
    std::cout << str << " 0x" << std::hex << hex << std::dec << std::endl;
}
