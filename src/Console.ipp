#pragma once

#include <stdint.h>
#include <string>
#include <iostream>

#include "Util.ipp"

static uint16_t statusLines = 0;

// Virtual terminal sequences. https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
// If an argument is given, it must be between 1-32767, negatives aren't allowed, 0 is treated as 1.
namespace ConsoleManip {
    constexpr auto SaveCursorPos = "\x1B""7";
    constexpr auto LoadCursorPos = "\x1B""8";
    constexpr auto LineUp = "\x1B[1A""\x1B[1G";
    constexpr auto InsertLineAbove = "\x1B[""1""L";
    constexpr auto InsertLineBelow = "\n""\x1B[""1""L""\x1B[1A";
    constexpr auto SetCursorBottomLine = "\x1B[32767;1H";

    inline void moveToStartOfStatusLines(){
        // Move to bottom left. Move up by status line count
        std::cout << SetCursorBottomLine;
        if(statusLines != 0){
            std::cout << "\x1B[" << std::to_string(statusLines) << 'F';
        }
    }
}
// Appends a message and inserts a new line after the fact
// Saves cursor pos after that
void AppendMessage(std::string str){
    // For some reason I can't append contexpr strings bruh.
    std::cout << ConsoleManip::LoadCursorPos << ConsoleManip::InsertLineAbove 
            << str << '\n' << ConsoleManip::SaveCursorPos;
}
// Current position is below the message queue.
uint16_t CreateStatusLine(std::string str){
    // std::cout << ConsoleManip::InsertLineBelow << str;
    ConsoleManip::moveToStartOfStatusLines();
    std::cout << str << '\n';
    return statusLines++;
}