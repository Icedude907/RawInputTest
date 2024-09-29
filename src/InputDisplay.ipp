#pragma once

#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <tuple>
#include <map>

#include <Windows.h>

#include "IntTypes.hpp"
#include "Console.hpp"
#include "KeyCodes.hpp"

class KeyboardState{
    public:
    // Bool is (isRepeating)
    std::unordered_map<KeyboardKey, bool> pressedButtons;

    KeyboardState(){}

    // Add repeat
    void add(KeyboardKey k){
        auto [location, wasInserted] = pressedButtons.insert({k, false});
        if(!wasInserted){
            // Failed to insert, set the isRepeating flag. Location is set to the existing entry.
            location->second = true;
        }
    }
    void remove(KeyboardKey k){
        pressedButtons.erase(k);
    }

    // Converts the current keyboard state into a string.
    std::string to_string(){
        std::stringstream ss;
        for (const auto& entry: pressedButtons) {
            ss << keycodeTable::lookup(entry.first);
            if(entry.second == true){ ss << '*';}
            ss << ' ';
        }
        return ss.str();
    }
};
class MouseState{
    public: // Mousebuttons
    bool Lb, Rb, Mb, B4, B5;
    SHORT wheel; SHORT hwheel;
    LONG totalx, totaly;
    LONG relx, rely;
    LONG absx, absy;

    MouseState(): Lb(false), Rb(false), Mb(false), B4(false), B5(false),
        wheel(0), hwheel(0), totalx(0), totaly(0),
        relx(0), rely(0), absx(0), absy(0) {

    }
    
    void update(RAWMOUSE mouse){
        USHORT flags = mouse.usButtonFlags;
        USHORT data = mouse.usButtonData;
        if(flags & RI_MOUSE_LEFT_BUTTON_DOWN  ){ Lb = true;  }
        if(flags & RI_MOUSE_LEFT_BUTTON_UP    ){ Lb = false; }
        if(flags & RI_MOUSE_RIGHT_BUTTON_DOWN ){ Rb = true;  }
        if(flags & RI_MOUSE_RIGHT_BUTTON_UP   ){ Rb = false; }
        if(flags & RI_MOUSE_MIDDLE_BUTTON_DOWN){ Mb = true;  }
        if(flags & RI_MOUSE_MIDDLE_BUTTON_UP  ){ Mb = false; }
        if(flags & RI_MOUSE_BUTTON_4_DOWN     ){ B4 = true;  }
        if(flags & RI_MOUSE_BUTTON_4_UP       ){ B4 = false; }
        if(flags & RI_MOUSE_BUTTON_5_DOWN     ){ B5 = true;  }
        if(flags & RI_MOUSE_BUTTON_5_UP       ){ B5 = false; }
        if(flags & RI_MOUSE_WHEEL){ wheel += (SHORT)data; }
        if(flags & RI_MOUSE_HWHEEL){ hwheel += (SHORT)data; }

        if(mouse.lLastX != 0 || mouse.lLastY != 0){
            if(mouse.usFlags == MOUSE_MOVE_RELATIVE) {
                relx = mouse.lLastX;
                rely = mouse.lLastY;
                totalx += mouse.lLastX;
                totaly += mouse.lLastY;
            }
            if(mouse.usFlags == MOUSE_MOVE_ABSOLUTE) {
                absx = mouse.lLastX;
                absy = mouse.lLastY;
                totalx = absx;
                totaly = absy;
            }
        }
    }

    // Prints changes and resets state.
    std::string to_string(){
        std::stringstream ss;
        if(Lb) { ss << "Lb "; }
        if(Rb) { ss << "Rb "; }
        if(Mb) { ss << "Mb "; }
        if(B4) { ss << "4b "; }
        if(B5) { ss << "5b "; }
             if( wheel){ ss << "Wheel "  << wheel; }
        else if(hwheel){ ss << "HWheel " << wheel; }
        if(relx || rely){ ss << "Rel:" << relx << ", " << rely << ' '; }
        if(absx || absy){ ss << "Abs:" << absx << ", " << absy << ' '; }
        ss << "Total: " << totalx << ", " << totaly;

        wheel = 0; hwheel = 0;
        relx = 0; rely = 0; absx = 0; absy = 0;
        return ss.str();
    }
};
// Kept for future use
// TODO: State shouldn't logically be associated with a renderer. Separate using shared_ptr for more flexible use?
struct State{
    KeyboardState kbdst;
    MouseState moust;
    State(){

    }
};

// Handles rendering of inputdisplays to the console.
struct InputDisplay{
    // 
    std::map<HANDLE, std::pair<i16, State>> displayedDevices;

    // Tries to add the handle to the console. If it is already registered, nothing happens.
    void add(HANDLE handle){
        if(displayedDevices.find(handle) != displayedDevices.end()) return;
        std::stringstream ss;
        ss << 'x' << std::setfill('0') << std::setw(8) << std::hex << (size_t)handle;
        displayedDevices.insert({handle, {cout.CreateStatusLine(ss.str()), {}}});
    }
    void update(HANDLE handle, std::string str){
        auto entry = displayedDevices.find(handle);
        auto entryLine = entry->second.first;
        if(entry == displayedDevices.end()) return;
        std::stringstream ss;
        ss << 'x' << std::setfill('0') << std::setw(8) << std::hex << (size_t)handle << ' ';
        auto out = ss.str() + str;
        cout.EditStatusLine(entryLine,out);
    }
    // Not optimal but fine
    void updateOrAdd(HANDLE handle, std::string str){
        add(handle);
        update(handle, str);
    }

    State& getStateOrAdd(HANDLE handle){
        add(handle);
        return displayedDevices.find(handle)->second.second;
    }
    // Removes the given device.
    // TODO: Reorder the lines when removing.
    void remove(HANDLE handle){
        auto entry = displayedDevices.find(handle);
        auto entryLine = entry->second.first;
        if(entry == displayedDevices.end()) return;
        cout.DeleteStatusLine(entryLine);
        // Shifts all the lines below up by one.
        for (auto &[key, val] : displayedDevices){
            if(val.first > entryLine) val.first--;
        }
        displayedDevices.erase(handle);
    }
};