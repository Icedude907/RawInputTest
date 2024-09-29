#include <string>
#include <iostream>

#include "IntTypes.hpp"
#include "Console.hpp"
#include "Util.hpp"

/* TODO: Maybe I should return "handles" that keep track of the line?
    Currently, only one thing can work with the status lines. Removing will ruin everything else.
*/

void Console::Setup(){
    std::cout << "\n\n\n" << ConsoleManip::MoveToBottom;
}
// Appends a single line status message.
void Console::AppendMessage(std::string str){
    std::cout << ConsoleManip::MoveToBottom;
    std::cout << ConsoleManip::MoveCursorUp(getMessagePadding()+1);
    std::cout << ConsoleManip::ScrollUp;
    std::cout << ConsoleManip::InsertLineAbove;
    std::cout << str;
    // Move back to bottom in case of termination being nice. Should just do this by intercepting a termination event
    std::cout << ConsoleManip::MoveToBottom;
}
// Returns the index of the statusline created.
i16 Console::CreateStatusLine(std::string str){
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
void Console::EditStatusLine(u16 index, std::string str){
    if(index+1 > statusLines) return;
    std::cout << ConsoleManip::MoveToBottom;
    std::cout << ConsoleManip::MoveCursorUp(statusLines - index);
    std::cout << ConsoleManip::ClearCurrentLine;
    std::cout << str;
    std::cout << ConsoleManip::MoveToBottom;
}

// Returns the number of status lines remaining
// Fails if the status line doesn't exist (returns current statusLine)
i16 Console::DeleteStatusLine(u16 index){
    if(index+1 > statusLines) return statusLines;
    std::cout << ConsoleManip::MoveToBottom;
    std::cout << ConsoleManip::MoveCursorUp(statusLines - index);
    std::cout << ConsoleManip::DeleteCurrentLine;
    std::cout << ConsoleManip::ScrollDown;
    std::cout << ConsoleManip::MoveToBottom;
    return statusLines--;
}

// Returns the index of the next status line to be allocated
i16 Console::getNextStatusLine(){
    return statusLines;
}

void Console::ShowOptions(){
    std::cout << ConsoleManip::MoveToBottom;
    std::cout << "\x1B[94m";
    std::cout << "()()()() Options go here!";
    std::cout << "\x1B[0m";
    std::cout << ConsoleManip::MoveToBottom;
    showOptions = true;
}
void Console::HideOptions(){
    std::cout << ConsoleManip::MoveToBottom;
    std::cout << ConsoleManip::ClearCurrentLine;
    showOptions = false;
}

// Name must be less than 255 chars
void Console::SetConsoleName(std::string name){
    std::cout << "\x1B]0;" << name << "\x1B\x5C";
}