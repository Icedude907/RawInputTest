#pragma once

#include <memory>
#include <string>

#include <Windows.h>

namespace Util::Win{
    // https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
    static void EnableVirtualTerminalProcessing(HANDLE hOut){
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }

    // FIXME: Not optimal.=
    std::string getDeviceInstancePath(HANDLE handle){
        UINT deviceInstancePathLen; // One of: RIDI_PREPARSEDDATA  RIDI_DEVICENAME  RIDI_DEVICEINFO. TODO: Implement other functions
        GetRawInputDeviceInfoA(handle, RIDI_DEVICENAME, NULL, &deviceInstancePathLen);
        // heap CHAR[] == LPSTR but can't use LPSTR because new[] requires a different template override (since it uses delete[]). Epic...
        std::unique_ptr<CHAR[]> deviceInstancePath(new CHAR[deviceInstancePathLen + 1]); // +1 for terminator? (Idk if its required.)
        GetRawInputDeviceInfoA(handle, RIDI_DEVICENAME, deviceInstancePath.get(), &deviceInstancePathLen);

        // TODO: Can I get away without this?
        std::string str = std::string(deviceInstancePath.get());
        return str;
    }

    
}