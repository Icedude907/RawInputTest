// This is admittedly a weird way of splitting off into multiple files but I'm doing it so sue me.
// This actually prevents making multiple compliation units.
#pragma once

#include <memory>
#include <tuple>
#include <map>
#include <thread>
#include <map>

#include <Windows.h>
#include "WinUtil.ipp"

#include "IntTypes.hpp"
#include "Util.hpp"
#include "KeyCodes.hpp"
#include "Console.hpp"
#include "InputDisplay.ipp"

static InputDisplay inputDisplay;

// TODO: Process touchpad data
// TODO: Make keyboard and mouse displays better (as the comment in RawInputTool.cpp)
void processRawInput(const bool unfocusedInput, const RAWINPUT* input){
    // Ptr is important since the struct can actually be of arbitrary length depending on if it's raw hid input.
    auto header = input->header;
    std::stringstream ss;

    if(input->header.dwType == RIM_TYPEKEYBOARD) {
        auto keybd = input->data.keyboard;
        KeyboardKey kk(keybd.MakeCode, keybd.Flags);
        State& state = inputDisplay.getStateOrAdd(header.hDevice);
        if(keybd.Flags & 1 == RI_KEY_BREAK){ state.kbdst.remove(kk); }
        else{ state.kbdst.add(kk); }

        ss << "KBD : " << state.kbdst.to_string();
        if(keybd.ExtraInformation != 0){ ss <<" other: 0x" << std::hex << keybd.ExtraInformation << std::dec; }
    }else if(input->header.dwType == RIM_TYPEMOUSE){
        auto mouse = input->data.mouse;
        State& state = inputDisplay.getStateOrAdd(header.hDevice);
        
        state.moust.update(mouse);
        ss << "MOUS: " << state.moust.to_string();
        if(mouse.ulExtraInformation != 0){ ss <<" other: 0x" << std::hex << mouse.ulExtraInformation << std::dec; }
    }else if(input->header.dwType == RIM_TYPEHID){
        ss << "HID/";
        auto hid = input->data.hid;
        // dwSizeHid = size per object
        // dwCount = number of objects
        // bRawData = array of bytes of raw data, should cast
        //      sizeof(bRawData) = dwSizeHid * dwCount

        // Windows Precision Touchpad input

    }else{
        ss << "Other data: ";
    }
    inputDisplay.updateOrAdd(header.hDevice, ss.str());
}


LRESULT CALLBACK windowCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    // Looking at `rawinputtest2` you will see a number of extra messages handled for completedness sake
    // This test won't deal with them because we are exclusively concerned with raw input, and will use the default processors in all other cases
    if(msg == WM_INPUT_DEVICE_CHANGE){
        HANDLE deviceHandle = (HANDLE)lParam;
        std::stringstream ss;
        switch(wParam){
            case GIDC_ARRIVAL: 
                ss << "New Dev: ";
                // inputDisplay.add(deviceHandle);
                break;
            case GIDC_REMOVAL: 
                ss << "Removed: ";
                inputDisplay.remove(deviceHandle);
                break;
            default: ss << "Something happened regarding a device on the system: ";
        }
        ss << std::hex << deviceHandle << std::dec;
        if(wParam == GIDC_ARRIVAL){
            ss << ' ' << Util::Win::getDeviceInstancePath(deviceHandle);
        }
        cout.AppendMessage(ss.str());
        return 0; // 0 = good
    }else if(msg == WM_INPUT){
        HRAWINPUT rawInputPacketHandle = (HRAWINPUT)lParam;
        UINT size = 0;
        GetRawInputData(rawInputPacketHandle, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
        std::unique_ptr<RAWINPUT[]> input(new RAWINPUT[size]);
        GetRawInputData(rawInputPacketHandle, RID_INPUT, input.get(), &size, sizeof(RAWINPUTHEADER));

        processRawInput(GET_RAWINPUT_CODE_WPARAM(wParam), input.get());

        return TRUE;
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
        Util::printStringHex("Error CreateWindow(). Code:", lastword);
    }
    return windowHandle;
}

void registerDevices(const HWND windowHandle){
    const size_t deviceCount = 3;
    RAWINPUTDEVICE device[deviceCount];
    device[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    device[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
    device[0].dwFlags = RIDEV_INPUTSINK | RIDEV_NOLEGACY | RIDEV_DEVNOTIFY;
    device[0].hwndTarget = windowHandle;

    device[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
    device[1].usUsage = HID_USAGE_GENERIC_MOUSE;
    device[1].dwFlags = RIDEV_INPUTSINK | RIDEV_NOLEGACY | RIDEV_DEVNOTIFY;
    device[1].hwndTarget = windowHandle;

    // Precision touchpad. Part of other.
    // Many touchpads also send keyboard and mouse data on other device pages to simulate actions.
    // However, raw input is normally best.
    device[2].usUsagePage = 0x000D;
    device[2].usUsage = 0x0005;
    device[2].dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
    device[2].hwndTarget = windowHandle;

    if (RegisterRawInputDevices(device, deviceCount, sizeof(device[0])) == FALSE){
        auto lastword = GetLastError();
        Util::printStringHex("Error registering raw input devices. Code:", lastword);
    }
}

void RawInputThreadMain(){
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    cout.Setup();
    cout.SetConsoleName("Raw Input Tool");
    cout.AppendMessage("Messages appear here:");

    cout.ShowOptions();

    auto windowName = "RawInput\'Window\'";
    auto windowHandle = createWindow(windowCallback, windowName, windowName);
    registerDevices(windowHandle);

    cout.CreateStatusLine("----- INPUT -------------");

    MSG msg;
    BOOL ret;
    while(ret = GetMessage(&msg, NULL, 0, 0) != 0) {
        if (ret == -1){
            cout.AppendMessage("An error was recieved in the message processing loop.");
        }else{
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}