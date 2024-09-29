#pragma once

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

    // Not optimal but I don't care.
    std::string getDeviceInstancePath(HANDLE handle){
        UINT deviceInstancePathLen; // One of: RIDI_PREPARSEDDATA  RIDI_DEVICENAME  RIDI_DEVICEINFO
        GetRawInputDeviceInfoA(handle, RIDI_DEVICENAME, NULL, &deviceInstancePathLen);
        LPSTR deviceInstancePath = new CHAR[deviceInstancePathLen + 1]; // +1 for terminator? (Idk if its required.)
        GetRawInputDeviceInfoA(handle, RIDI_DEVICENAME, deviceInstancePath, &deviceInstancePathLen);

        std::string str = std::string(deviceInstancePath);

        delete[] deviceInstancePath;
        return str;
    }

}