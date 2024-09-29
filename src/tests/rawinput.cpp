
#include <stdint.h>
#include <vector>
#include <iostream>
#include <iomanip>

#include <Windows.h>
#include <hidusage.h>

LRESULT CALLBACK OnWindowEvent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){

    if(msg == WM_INPUT){ /*std::cout << "WM_INPUT MESSAGE RECEIVED!" << std::endl;*/ }
    else if(msg == WM_DEVICECHANGE){ std::cout << "A device was added or removed." << std::endl; return TRUE;}
    else { std::cout << "Unknown message received. " << msg << " Probs part of window creation. Returning true and hope it works..." << std::endl; return TRUE;}

    // WParam == 0 (In focus), 1 (Not in focus)
    bool isInFocus = !GET_RAWINPUT_CODE_WPARAM(wParam);
    HRAWINPUT rawInputPacket = (HRAWINPUT)lParam;
    // Read the rawinputdata into a packet. First call gets size, second gets data.
    UINT size = 0;
    GetRawInputData(rawInputPacket, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
    RAWINPUT* input = new RAWINPUT[size];
    GetRawInputData(rawInputPacket, RID_INPUT, input, &size, sizeof(RAWINPUTHEADER));

    auto header = input->header;
    std::cout << "Device: " << header.hDevice << ": ";

    if(input->header.dwType == RIM_TYPEKEYBOARD) {
        auto keybd = input->data.keyboard;
        std::cout << "KBD : 0x" << std::hex << std::setw(4) << std::setfill('-') <<
            (size_t)keybd.MakeCode <<
            // Bitfield: 0 = down, 1 = up, 2 = E0 Prefix, 4 = E1 Prefix
            ((keybd.Flags & 0b001) ? " rel" : " prs") <<
            ((keybd.Flags & 0b010) ? " E0" : "   ") <<
            ((keybd.Flags & 0b100) ? " E1" : "   ") <<
            // Other fields: `Message` and `VKey` which are both legacy codes.
            " other: 0x" << keybd.ExtraInformation   // Device specific long field
        << std::endl;
    }else if(input->header.dwType == RIM_TYPEMOUSE){
        auto mouse = input->data.mouse;
        std::cout << "MOUS: " << std::dec;
        // To get screen coords use this
        // However, you don't want that if you want to look at each device independently.
        // POINT coords; GetPhysicalCursorPos(&coords);
        // std::cout << "Abs: " << coords.x << ", " << coords.y;
        
        if(mouse.usFlags == MOUSE_MOVE_RELATIVE) {
            std::cout << "Rel: dx " << mouse.lLastX << ", dy " << mouse.lLastY << " other: 0x" << std::hex << mouse.ulExtraInformation << std::dec;
        }
        // With the 3 devices I've checked, none use this packet. It would be a pain to handle these together but hey.
        if(mouse.usFlags == MOUSE_MOVE_ABSOLUTE) {
            std::cout << "Abs:  x " << mouse.lLastX << ",  y " << mouse.lLastY << " other: 0x" << std::hex << mouse.ulExtraInformation << std::dec;
        }
        if(mouse.usButtonFlags != 0){
            std::cout << ", Attrib: ";
            if(mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN  ){ std::cout << "Lp "; }
            if(mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP    ){ std::cout << "L^ "; }
            if(mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN ){ std::cout << "Rp "; }
            if(mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP   ){ std::cout << "R^ "; }
            if(mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN){ std::cout << "Mp "; }
            if(mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP  ){ std::cout << "M^ "; }
            if(mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN     ){ std::cout << "4p "; }
            if(mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP       ){ std::cout << "4^ "; }
            if(mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN     ){ std::cout << "5p "; }
            if(mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP       ){ std::cout << "5^ "; }
            if(mouse.usButtonFlags & RI_MOUSE_WHEEL){
                std::cout << "Wheel: " << (SHORT)mouse.usButtonData;
            }
            if(mouse.usButtonFlags & RI_MOUSE_HWHEEL){ // Horizontal mouse wheel
                std::cout << "HWhel: " << (SHORT)mouse.usButtonData;
            }
        }
        std::cout << std::endl;
    }else{
        std::cout << "Other data: " << std::endl;
    }
    delete[] input;
    return 0; 
}

int main(){
    SetProcessDPIAware(); // For mouse co-ords.
    // Must define a window to use background input. 
    // Why this is required remains a mystery to this day. 
    // We don't have to show it or render it or anything
    WNDCLASS wc = {
        .style = CS_VREDRAW | CS_HREDRAW,
        .lpfnWndProc = OnWindowEvent,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = GetModuleHandle(0),
        .hIcon = LoadIcon(0, IDI_APPLICATION),
        .hCursor = LoadCursor(0, IDC_ARROW),
        .hbrBackground = (HBRUSH)COLOR_WINDOW,
        .lpszMenuName = nullptr,
        .lpszClassName = "RawInputClass",
    };
    RegisterClass(&wc);
    HWND hwnd = CreateWindow("RawInputClass", "RawInputClass", WS_POPUP, 0, 0, 64, 64, 0, 0, GetModuleHandle(0), 0);
    
    if (hwnd == nullptr){
        auto lastword = GetLastError();
        std::cout << "Error CreateWindow(). Code: 0x" << std::hex << lastword << std::endl;
        return 0;
    }

    const size_t deviceCount = 2;
    RAWINPUTDEVICE device[deviceCount];
    //capture all keyboard input
    device[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    device[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
    device[0].dwFlags = RIDEV_INPUTSINK | RIDEV_NOLEGACY; // SINK = BG in
    device[0].hwndTarget = hwnd;
                                
    //capture all mouse input
    device[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
    device[1].usUsage = HID_USAGE_GENERIC_MOUSE;
    device[1].dwFlags = RIDEV_INPUTSINK | RIDEV_NOLEGACY;
    device[1].hwndTarget = hwnd;

    if (RegisterRawInputDevices(device, deviceCount, sizeof(device[0])) == FALSE){
        auto lastword = GetLastError();
        std::cout << "Error registering raw input devices. Code: 0x" << std::hex << lastword << std::endl;
        return 0;
    }

    // Saw this is bsnes, idk why its good. Must check
    // std::cout << "Waiting for input: ";
    // HANDLE mutex = nullptr;
    // WaitForSingleObject(mutex, INFINITE);
    // ReleaseMutex(mutex);
    // std::cout << "got." << std::endl;

    // GetMessage pauses the thread until an update is given.
    MSG msg = {0};
    while(GetMessage(&msg, NULL, 0, 0) != 0) {
        // std::cout << "Got a input packet!" << std::endl;
        TranslateMessage(&msg);
        DispatchMessage(&msg); // Sends the message to the callback function.
    }
}