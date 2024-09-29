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

// Kept for future use
// TODO: State shouldn't logically be associated with a renderer. Separate using shared_ptr for more flexible use?
struct State{
    KeyboardState kbdst;

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