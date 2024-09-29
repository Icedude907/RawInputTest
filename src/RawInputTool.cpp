#include <stdint.h>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <unordered_set>
#include <map>
#include <thread>

#include <Windows.h>
extern "C" { // Weird mingw issue Easily fixed.
    #include <hidsdi.h>
}

#include "KeyCodes.hpp"
#include "RawInputThread.ipp"

////////////////////////////////////////////////
// General layout of the program
// Threads:
// Main: General odds and ends.
// 2: Window setup and processing
////////////////////////////////////////////////

/* Console Layout
...
Device 0x100 was added
Device was removed
Unknown message 0x555 recieved ***SAVE POSITION HERE***
----------------------------------------------
Input:      + press, * repeating, - release
Keybd 1)  't'+  'o'-  'w'*
Keybd 2)  0x55+ 0x33+
Mouse 1 update 255) L+  R-  dx: +1, dy: -5

(1) opt1, (2) opt2, (3) opt3
*//* Render Behaviour
    Cursor position is saved just after the end of the message queue
    Adding a new message just involves moving to the next line, inserting the message, saving cursor position, and rendering the bottom again.
    Rendering the bottom involves loading the cursor position navigating to the row we want, and going from there.
*//* Program behaviour
(1) Prevents keypresses from triggering other options (cancel with escape)
(2) Enumerate rawinput devices
(Q) Quits the program (case sensitive)
// Unfortunately gonna have to use ReadConsoleInput to get these inputs because I don't want to quit out of focus and virtual terminal sequences can't help me
*/


// https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
void EnableVirtualTerminalProcessing(HANDLE hOut){
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

int main(){
    // Setup. May not all be required but I want to have it incase it prevents jank later on in development.
    SetProcessDPIAware();
    AllocConsole();
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    EnableVirtualTerminalProcessing(consoleHandle);

    // Start the threads
    std::thread rawInputThread(RawInputThreadMain);

    // Need to find a way to stop the program.
    rawInputThread.join();
    std::cout << std::flush;
}