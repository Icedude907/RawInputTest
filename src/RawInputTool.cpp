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
#include "Console.hpp"

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
Unknown message 0x555 recieved
----------------------------------------------
Input:      + press, * repeating, - release
Keybd 1)  't'+  'o'-  'w'*
Keybd 2)  0x55+ 0x33+
Mouse 1 update 255) L+  R-  dx: +1, dy: -5

(1) opt1, (2) opt2, (3) opt3
*//* Program behaviour
(1) Prevents keypresses from triggering other options (cancel with escape)
(2) Enumerate rawinput devices
(Q) Quits the program (case sensitive)
// Unfortunately gonna have to use ReadConsoleInput to get these inputs because I don't want to quit out of focus and virtual terminal sequences can't help me
// Or use the rawinput api and somehow clear all inputs the person is making to the terminal
*/

// TODO: Make more performant and easier to understand.
int main(){
    // Setup. May not all be required but I want to have it incase it prevents jank later on in development.
    SetProcessDPIAware();
    AllocConsole();
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    Util::Win::EnableVirtualTerminalProcessing(consoleHandle);

    // Start the threads
    std::thread rawInputThread(RawInputThreadMain);

    // Need to find a way to stop the program.
    rawInputThread.join();
    std::cout << std::flush;
}