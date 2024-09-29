#pragma once

#include <string>

#include "IntTypes.hpp"

// Virtual terminal sequences. https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
// If an argument is given, it must be between 1-32767, negatives aren't allowed, 0 is treated as 1.
namespace ConsoleManip {
    constexpr auto MoveToBottom = "\x1B[32766;1H";
    constexpr auto InsertLineAbove = "\x1B[""1""L";
    constexpr auto ScrollUp = "\x1B[1S"; // Scrolls text up by one line
    constexpr auto ScrollDown = "\x1B[1T";
    constexpr auto ClearCurrentLine = "\x1B[0K";
    constexpr auto DeleteCurrentLine = "\x1B[1M"; // Deletes the line and everything below scrolls up.
    // constexpr auto InsertLineBelow = "\n""\x1B[""1""L""\x1B[1A";
    inline std::string MoveCursorUp(i16 n = 1){
        if (n < 1) return "";
        return std::string("\x1B[") + std::to_string(n) + 'A';
    }
}

class Console{
    public:
    i16 statusLines = 0;
    bool showOptions = false;
    
    private:
    inline i16 getMessagePadding(){ return statusLines; }

    public:
    void Setup();
    // Appends a single line status message.
    void AppendMessage(std::string str);
    // Returns the index of the statusline created.
    i16 CreateStatusLine(std::string str);
    /* Edits the status line based off the given index.
        Index is 0 based, with 0 being the topmost status line.
        Indexes referring to status lines yet to be created will do nothing.
    */
    void EditStatusLine(u16 index, std::string str);

    // Returns the number of status lines remaining
    // Fails if the status line doesn't exist (returns current statusLine)
    i16 DeleteStatusLine(u16 index);

    // Returns the index of the next status line to be allocated
    i16 getNextStatusLine();

    void ShowOptions();
    void HideOptions();

    // Name must be less than 255 chars
    static void SetConsoleName(std::string name);
};

static Console cout;
#include "Util.hpp"