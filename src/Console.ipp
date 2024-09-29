#pragma once

#include <string>
#include <iostream>

#include "Util.ipp"

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

/* TODO: Maybe I should return "handles" that keep track of the line?
    Currently, only one thing can work with the status lines. Removing will ruin everything else.
*/
class Console {
    public:
    i16 statusLines = 0;
    bool showOptions = false;

    private:
    inline i16 getMessagePadding(){ return statusLines; }

    public:

    void Setup(){
        std::cout << "\n\n\n" << ConsoleManip::MoveToBottom;
    }
    // Appends a single line status message.
    void AppendMessage(std::string str){
        std::cout << ConsoleManip::MoveToBottom;
        std::cout << ConsoleManip::MoveCursorUp(getMessagePadding()+1);
        std::cout << ConsoleManip::ScrollUp;
        std::cout << ConsoleManip::InsertLineAbove;
        std::cout << str;
        // Move back to bottom in case of termination being nice. Should just do this by intercepting a termination event
        std::cout << ConsoleManip::MoveToBottom;
    }
    // Returns the index of the statusline created.
    i16 CreateStatusLine(std::string str){
        std::cout << ConsoleManip::MoveToBottom;
        std::cout << ConsoleManip::ScrollUp;
        std::cout << ConsoleManip::MoveCursorUp(1);
        std::cout << ConsoleManip::InsertLineAbove; 
        std::cout << str; 
        std::cout << ConsoleManip::MoveToBottom;
        return statusLines++;
    }
    /* Edits the status line based off the given index.
        Index is 0 based, with 0 being the topmost status line.
        Indexes referring to status lines yet to be created will do nothing.
    */
    void EditStatusLine(u16 index, std::string str){
        if(index+1 > statusLines) return;
        std::cout << ConsoleManip::MoveToBottom;
        std::cout << ConsoleManip::MoveCursorUp(statusLines - index);
        std::cout << ConsoleManip::ClearCurrentLine;
        std::cout << str;
        std::cout << ConsoleManip::MoveToBottom;
    }

    // Returns the number of status lines remaining
    // Fails if the status line doesn't exist (returns current statusLine)
    i16 DeleteStatusLine(u16 index){
        if(index+1 > statusLines) return statusLines;
        std::cout << ConsoleManip::MoveToBottom;
        std::cout << ConsoleManip::MoveCursorUp(statusLines - index);
        std::cout << ConsoleManip::DeleteCurrentLine;
        std::cout << ConsoleManip::ScrollDown;
        std::cout << ConsoleManip::MoveToBottom;
        return statusLines--;
    }

    // Returns the index of the next status line to be allocated
    i16 getNextStatusLine(){
        return statusLines;
    }

    void ShowOptions(){
        std::cout << ConsoleManip::MoveToBottom;
        std::cout << "\x1B[94m";
        std::cout << "()()()() Options go here!";
        std::cout << "\x1B[0m";
        std::cout << ConsoleManip::MoveToBottom;
        showOptions = true;
    }
    void HideOptions(){
        std::cout << ConsoleManip::MoveToBottom;
        std::cout << ConsoleManip::ClearCurrentLine;
        showOptions = false;
    }

    // Name must be less than 255 chars
    static void SetConsoleName(std::string name){
        std::cout << "\x1B]0;" << name << "\x1B\x5C";
    }
};

static Console cout;