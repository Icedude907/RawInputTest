#include <stdint.h>
#include <vector>
#include <iostream>
#include <iomanip>

#include <Windows.h>

int main(){
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode); // https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences

    std::cout << "This is some text" << std::endl;
    std::cout << "\x1b""7""Saved position" << std::endl;
    std::string str;
    std::cin >> str;
    std::cout << "\x1b""8""Loaded position" << std::endl;
}