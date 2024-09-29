#pragma once

#include <stdint.h>
#include <string>
#include <tuple>
#include <map>

#include <Windows.h>

struct KeyboardKey {
    USHORT code;
    bool E0:1; // Bitfield.
    bool E1:1;

    constexpr KeyboardKey(const KeyboardKey&) = default;
    constexpr KeyboardKey(USHORT code): code(code), E0(false), E1(false){} // Also allows automagic conversion
    constexpr KeyboardKey(USHORT code, USHORT info): code(code), E0(info & RI_KEY_E0), E1(info & RI_KEY_E1) {}
    constexpr KeyboardKey(USHORT code, bool E0, bool E1): code(code), E0(E0), E1(E1){}

    // Used for sorting algorithm.
    // Lower code first. If tied then no E0 is less. If tied then no E1 is less.
    friend inline bool operator<(const KeyboardKey& lhs, const KeyboardKey& rhs){
        // I'd love to just compare the entire struct converted to an int using less than, but I have to do this instead.
        return std::tie(lhs.code, lhs.E0, lhs.E1) < std::tie(rhs.code, rhs.E0, rhs.E1);
    }
};

// Translates specific key patterns into keycodes.
// Can't use constexpr cos c++ sucks.
/* I used (sorted) map and it absolutely exploded with an 100 line error trace
    Turns out this is because it didn't have a way to sort keys in order (missing `operator<`)
    Imagine C++ having nice error reporting
   Can't use unordered_map without defining a hashing function which would be a pain but again I didn't really get a clear error.
*/
const static std::map<KeyboardKey, std::string> keycodetable = {
    {0x1e, "a"},
};