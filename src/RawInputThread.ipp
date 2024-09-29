// This is admittedly a weird way of splitting off into multiple files but I'm doing it so sue me.
// This actually prevents making multiple compliation units.
#pragma once

#include <unordered_set>
#include <sstream>

#include "Util.ipp"
#include "WinUtil.ipp"
#include "KeyCodes.hpp"
#include "Console.ipp"

// std::unordered_set<KeyboardKey> pressedButtons;

LRESULT CALLBACK windowCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    // Looking at `rawinputtest2` you will see a number of extra messages handled for completedness sake
    // This test won't deal with them because we are exclusively concerned with raw input, and will use the default processors in all other cases
    if(msg == WM_INPUT_DEVICE_CHANGE){
        std::stringstream ss;
        switch(wParam){
            case GIDC_ARRIVAL: ss << "New Dev: "; break;
            case GIDC_REMOVAL: ss << "Removed: "; break;
            default: ss << "Something happened regarding a device on the system: ";
        }
        HANDLE deviceHandle = (HANDLE)lParam;
        auto deviceInstancePath = getDeviceInstancePath(deviceHandle);
        ss << std::hex << deviceHandle << std::dec << ' ' << deviceInstancePath;
        AppendMessage(ss.str());
        return 0; // 0 = good
    }else if(msg == WM_INPUT){

    }
    
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND createWindow(const WNDPROC callback, const LPCSTR className, const LPCSTR windowName){
    WNDCLASS wc = {
        .style = CS_VREDRAW | CS_HREDRAW,
        .lpfnWndProc = callback,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = GetModuleHandle(0),
        .hIcon = LoadIcon(0, IDI_APPLICATION),
        .hCursor = LoadCursor(0, IDC_ARROW),
        .hbrBackground = (HBRUSH)COLOR_WINDOW,
        .lpszMenuName = nullptr,
        .lpszClassName = className,
    };
    RegisterClass(&wc);

    HWND windowHandle = CreateWindow(className, windowName, WS_POPUP, 0, 0, 64, 64, 0, 0, GetModuleHandle(NULL), NULL);
    if (windowHandle == NULL){
        auto lastword = GetLastError();
        printStringHex("Error CreateWindow(). Code:", lastword);
    }
    return windowHandle;
}

void registerDevices(const HWND windowHandle){
    const size_t deviceCount = 2;
    RAWINPUTDEVICE device[deviceCount];
    device[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    device[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
    device[0].dwFlags = RIDEV_INPUTSINK | RIDEV_NOLEGACY | RIDEV_DEVNOTIFY;
    device[0].hwndTarget = windowHandle;

    device[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
    device[1].usUsage = HID_USAGE_GENERIC_MOUSE;
    device[1].dwFlags = RIDEV_INPUTSINK | RIDEV_NOLEGACY | RIDEV_DEVNOTIFY;
    device[1].hwndTarget = windowHandle;

    if (RegisterRawInputDevices(device, deviceCount, sizeof(device[0])) == FALSE){
        auto lastword = GetLastError();
        printStringHex("Error registering raw input devices. Code:", lastword);
    }
}

void RawInputThreadMain(){
    std::cout << ConsoleManip::SetCursorBottomLine << ConsoleManip::SaveCursorPos << std::flush; // Do this to prevent messiness.
    AppendMessage("Messages appear here:");

    auto windowName = "RawInput\'Window\'";
    auto windowHandle = createWindow(windowCallback, windowName, windowName);
    registerDevices(windowHandle);

    // SavedPOS   |here
    CreateStatusLine("-------------------------");
    CreateStatusLine("This is the input display");
    CreateStatusLine("Example text.");

    // Inserting a message
    AppendMessage("Here is a message!");

    {
        return;
    }

    MSG msg;
    BOOL ret;
    while(ret = GetMessage(&msg, NULL, 0, 0) != 0) {
        if (ret == -1){
            AppendMessage("An error was recieved in the message processing loop.");
        }else{
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}