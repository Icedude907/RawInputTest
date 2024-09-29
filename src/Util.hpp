#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "Console.hpp"

namespace Util{

    inline void printStringHex(std::string str, size_t hex){
        std::stringstream ss;
        ss << str << " 0x" << std::hex << hex << std::dec << std::endl;
        cout.AppendMessage(ss.str());
    }

}