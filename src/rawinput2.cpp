/* General notes:
    Capital types tend to be windows types.
*/

#include <stdint.h>
#include <vector>
#include <iostream>
#include <iomanip>

#include <Windows.h>
// Weird mingw issue Easily fixed.
extern "C" {
    #include <hidsdi.h>
}

//
// Utility functions
//
void printStringHex(std::string str, size_t hex){
    std::cout << str << " 0x" << std::hex << hex << std::dec << std::endl;
}

// Not optimal but I don't care. C++ sucks. Rust isn't perfect but better.
std::string getDeviceInstancePath(HANDLE handle){
    UINT deviceInstancePathLen; // One of: RIDI_PREPARSEDDATA  RIDI_DEVICENAME  RIDI_DEVICEINFO
    GetRawInputDeviceInfoA(handle, RIDI_DEVICENAME, NULL, &deviceInstancePathLen); // A = ascii, W = wide char (unicode)
    LPSTR deviceInstancePath = new CHAR[deviceInstancePathLen + 1]; // +1 for terminator? (Idk if its required.)
    GetRawInputDeviceInfoA(handle, RIDI_DEVICENAME, deviceInstancePath, &deviceInstancePathLen);
    /* I just want to highlight how obtuse the below line is. 
      Does it make a copy of the string? Does it take ownership of the buffer by setting a pointer?
      You can't tell unless you read the docs. More self documenting code is good!
      `let str = std::string::copyCStr(deviceInstancePath) is much less obtuse.`
      In this scenario copyCStr is a static function returning a std::string
    */
    std::string str = std::string(deviceInstancePath);

    // Oh boy I love manually cleaning up arrays to prevent memory leaks. _C++_...
    delete[] deviceInstancePath;
    return str;
}

// Lists all the devices detected by the raw input api.
// This isn't actually required for any reason (I don't think), but included for completedness.
// In addition, the deviceInstancePath might be used to get extra details about a device.
void enumerateRawInputDeviceList(){
    UINT numdevices = 0;
    GetRawInputDeviceList(NULL, &numdevices, sizeof(RAWINPUTDEVICELIST));   // Called to get array size
    auto devices = new RAWINPUTDEVICELIST[numdevices];
    GetRawInputDeviceList(devices, &numdevices, sizeof(RAWINPUTDEVICELIST));// Get the devices

    std::cout << "Raw input devices:\n";
    for(size_t i = 0; i < numdevices; i++){
        switch(devices[i].dwType){
            case RIM_TYPEMOUSE:     std::cout << "Mouse: "; break;
            case RIM_TYPEKEYBOARD:  std::cout << "Keybd: "; break;
            case RIM_TYPEHID:       std::cout << "H.I.D: "; break;
            default:                std::cout << "Other: ";
        }
        std::cout << devices[i].hDevice << ' ';
        std::cout << '\"' << getDeviceInstancePath(devices[i].hDevice) << "\",\n";
    }
    delete[] devices;
}

// To get background input you must have a window set up to recieve messages
// Why this is still a requirement remains a mystery to this day
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
    // - - x, y, w,h(any), parentless, menuless, GetModuleHandle(NULL), no extra data needed to be passed to the callback on creation
    // GetModuleHandle(NULL) returns a handle to the exe file that runs the program. Thus we associate the window with the program.
    // A window is hidden by default, so we don't need to deal with it actually annoying us tho.
    HWND windowHandle = CreateWindow(className, windowName, WS_POPUP, 0, 0, 64, 64, 0, 0, GetModuleHandle(NULL), NULL);
    if (windowHandle == NULL){
        auto lastword = GetLastError();
        printStringHex("Error CreateWindow(). Code:", lastword);
    }
    return windowHandle;
}

void registerDevices(const HWND windowHandle){
    /* RegisterRawInputDevices takes an array of RAWINPUTDEVICE structs.
       All the types of device will be subsequently captured by the api with the options given.
       This demo only does kbd and mouse.
       Magic numbers are here: https://docs.microsoft.com/en-us/windows-hardware/drivers/hid/hid-architecture#hid-clients-supported-in-windows
    */
    const size_t deviceCount = 2;
    RAWINPUTDEVICE device[deviceCount];
    //capture all keyboard input
    device[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    device[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
    // SINK = Background input. NOLEGACY = Good yes pls. DEVNOTIFY = Notify when device is added or removed.
    device[0].dwFlags = RIDEV_INPUTSINK | RIDEV_NOLEGACY | RIDEV_DEVNOTIFY;
    device[0].hwndTarget = windowHandle;
    //capture all mouse input
    device[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
    device[1].usUsage = HID_USAGE_GENERIC_MOUSE;
    device[1].dwFlags = RIDEV_INPUTSINK | RIDEV_NOLEGACY | RIDEV_DEVNOTIFY;
    device[1].hwndTarget = windowHandle;

    // I don't know on what planet it would fail, but I put this here for completedness sake.
    // It's good to have this in case you have a malformed argument or something.
    if (RegisterRawInputDevices(device, deviceCount, sizeof(device[0])) == FALSE){
        auto lastword = GetLastError();
        printStringHex("Error registering raw input devices. Code:", lastword);
    }
}

// Forward declare cos cpp sucks.
#define windowCallback windowCallbackFN
LRESULT CALLBACK windowCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int main(){
    // Setup
    // -------------
    SetProcessDPIAware();   // Stops windows adjusting some stuff.
    AllocConsole();         // Makes sure we have a console
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode); // https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences

    enumerateRawInputDeviceList();
    const char windowName[] = "RawInput\'Window\'";
    auto windowHandle = createWindow(windowCallback, windowName, windowName);
    registerDevices(windowHandle);

    std::cout << "-------------------------------------\n"
                 " Starting up the message event loop.\n"
                 "-------------------------------------" << std::endl;

    // Message Loop
    // -------------
    MSG msg;
    BOOL ret;
    /* Freezes until we get a message.
       If hWnd is NULL, GetMessage retrieves messages for any window that belongs to the current thread.
       The 0's mean no message is filtered / all messages are processed.
       If the function returns 0 then the window has recieved a WM_QUIT message. (Must use != 0. -1 and below are error states).
    */
    while(ret = GetMessage(&msg, NULL, 0, 0) != 0) {
        if (ret == -1){
            std::cout << "An error was recieved in the message processing loop. " << ret << std::endl;
        }else{
            TranslateMessage(&msg); // TranslateMessage translates virtual-key messages into character messages.
            DispatchMessage(&msg);  // Windows magic calls the handler since we linked it earlier.
                                    // Runs on the same thread.
        }
    }

    // Clean up. 
    // Not really essential cos windows frees everything but its good to do.
    // -------------
    DestroyWindow(windowHandle);

    // We are done!
    std::cout << "Process completed" << std::endl;
    return EXIT_SUCCESS;
}

void processRawInput(const bool unfocusedInput, const RAWINPUT input[]){
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
}

/* Windows get a lot of messages. https://wiki.winehq.org/List_Of_Windows_Messages

*/
LRESULT CALLBACK windowCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    // Lol switch case is bad.
    switch (msg) {
        // You could just use DefWindowProc but I include for completedness.
        // Window creation process
        case WM_CREATE:     return 0;    // 0 = good
        case WM_NCCREATE:   return TRUE; // TRUE = good
        case WM_NCCALCSIZE: return 0;    // 0 = valid value
        case WM_SIZE:       return 0;    // 0 = good
        case WM_MOVE:       return 0;    // 0 = good
        // Devices
        case WM_DEVICECHANGE:
            // std::cout << "A change has occurred to some device on the system." << std::endl;
            // This will also get pinged for WM_INPUT_DEVICE_CHANGE if applicable
            return TRUE;
        // Finally, raw input
        case WM_INPUT_DEVICE_CHANGE: {
            // Called numerous times at the start for each individual device. Essentially enumerates for you.
            switch(wParam){
                case GIDC_ARRIVAL: std::cout << "New Dev: "; break;
                case GIDC_REMOVAL: std::cout << "Removed: "; break;
                default: std::cout << "Something happened regarding a device on the system: ";
            }
            HANDLE deviceHandle = (HANDLE)lParam;
            auto str = getDeviceInstancePath(deviceHandle);
            std::cout << std::hex << deviceHandle << std::dec << ' ' << str << std::endl;
            return 0; // 0 = good
        }
        case WM_INPUT:
            // // Most complex packet.
            // std::cout << "Input recieved." << std::endl;

            HRAWINPUT rawInputPacketHandle = (HRAWINPUT)lParam;
            UINT size = 0;
            GetRawInputData(rawInputPacketHandle, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
            RAWINPUT* input = new RAWINPUT[size];
            GetRawInputData(rawInputPacketHandle, RID_INPUT, input, &size, sizeof(RAWINPUTHEADER));

            processRawInput(GET_RAWINPUT_CODE_WPARAM(wParam), input);

            delete[] input;
            return TRUE;
    }
    // Anything we miss we use the default processor for.
    printStringHex("Unknown window message. Using DefWindowProc.", msg);
	return DefWindowProc(hwnd, msg, wParam, lParam);
}