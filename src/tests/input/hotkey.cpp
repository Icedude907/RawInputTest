
#include <stdint.h>
#include <vector>
#include <iostream>

#include <Windows.h>
#include <hidusage.h>

// HOTKEY API:
// Register a keycode with specific flags
//  (Flags include shift, alt, etc)
// Wait for messages, once you have a hotkey message the hotkey has been triggered
// Unfortunately doesn't provide the ability to detect keyreleases, and it also eats the input.

int main(){
    const uint8_t VK_Z = 0x5a;
    if (RegisterHotKey(NULL, 19, MOD_NOREPEAT, VK_Z)) {
        std::cout << "Hotkey Z registered!" << std::endl;
    }

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0) != 0){
        if (msg.message == WM_HOTKEY){
            std::cout << "WM_HOTKEY received" << std::endl;
        }
    }

    return 0;
}