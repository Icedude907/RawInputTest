#pragma once

#include <stdint.h>
#include <string>
#include <tuple>
#include <map>
#include <sstream>

#include <Windows.h>

struct KeyboardKey {
    USHORT code;
    bool E0:1; // Bitfield.
    bool E1:1;

    constexpr KeyboardKey(const KeyboardKey&) = default;
    constexpr KeyboardKey(USHORT code): code(code), E0(false), E1(false){} // Also allows automagic conversion
    constexpr KeyboardKey(USHORT code, USHORT info): code(code), E0(info & RI_KEY_E0), E1(info & RI_KEY_E1) {}
    constexpr KeyboardKey(USHORT code, bool E0, bool E1): code(code), E0(E0), E1(E1){}

    // Used for sorting algorithms.
    // Lower code first. If tied then no E0 is less. If tied then no E1 is less.
    friend inline bool operator<(const KeyboardKey& lhs, const KeyboardKey& rhs){
        // I'd love to just compare the entire struct converted to an int using less than, but I have to do this instead.
        return std::tie(lhs.code, lhs.E0, lhs.E1) < std::tie(rhs.code, rhs.E0, rhs.E1);
    }
    friend inline bool operator==(const KeyboardKey& lhs, const KeyboardKey& rhs){
        return lhs.code == rhs.code 
            && lhs.E0 == rhs.E0 
            && lhs.E1 == rhs.E1;
    }

    // Returns the keycode in hex form with a suffix
    // - (E0), | (E1), + (E0 & E1)
    std::string to_string(){
        std::stringstream ss;
        ss << 'x' << std::hex << std::setw(4) << std::setfill('0') << (size_t)code;
        if(E0){
            if(E1){ ss << '+'; }
            else  { ss << '-'; }
        }else if(E1){
            ss << '|';
        }
        return ss.str();
    }
};
namespace std {
    template <> struct hash<KeyboardKey>{
        // Hashing function. Just converts to size_t
        // Not really optimal but I don't care. It does the thing.
        std::size_t operator()(const KeyboardKey& val) const noexcept  {
            size_t hash = (val.E0 << 1 | val.E1) << (sizeof(USHORT) * 8) | val.code;
            return hash;
        }
    };
}

// Translates specific key patterns into keycodes.
// Can't use constexpr cos c++ sucks.
/* I used (sorted) map and it initially absolutely exploded with an 100 line error trace
    Turns out this is because it didn't have a way to sort keys in order (missing `operator<`)
    Imagine C++ having nice error reporting
   Can't use unordered_map without defining a hashing function which would be a pain but again I didn't really get a clear error.
*/
namespace keycodeTable{
    const static std::map<KeyboardKey, std::string> keycodetable = {
        // Alphabet codes. Aligns with the QUERTY layout but I've sorted them alphabetically.
        {0x1e, "a"}, {0x30, "b"}, {0x2e, "c"}, {0x20, "d"}, {0x12, "e"},
        {0x21, "f"}, {0x22, "g"}, {0x23, "h"}, {0x17, "i"}, {0x24, "j"},
        {0x25, "k"}, {0x26, "l"}, {0x32, "m"}, {0x31, "n"}, {0x18, "o"},
        {0x19, "p"}, {0x10, "q"}, {0x13, "r"}, {0x1f, "s"}, {0x14, "t"},
        {0x16, "u"}, {0x2f, "v"}, {0x11, "w"}, {0x2d, "x"}, {0x15, "y"},
        {0x2c, "z"},

        {0x02, "1"}, {0x03, "2"}, {0x04, "3"}, {0x05, "4"}, {0x06, "5"},
        {0x07, "6"}, {0x08, "7"}, {0x09, "8"}, {0x0a, "9"}, {0x0b, "0"},
        
        {0x3b, "f1"}, {0x3c, "f2"}, {0x3d, "f3"}, {0x3e, "f4"}, {0x3f, "f5"},
        {0x40, "f6"}, {0x41, "f7"}, {0x42, "f8"}, {0x43, "f9"}, {0x44, "f10"},
        {0x57, "f11"},{0x58, "f12"},
        
        // Other querty rows
        {0x01, "esc"},
        {0x29, "`"}, {0x0c, "-"}, {0x0d, "="}, {0x0e, "bsp"},
        {0x0f, "tab"}, {0x1a, "["}, {0x1b, "]"}, {0x2b, "\\"},
        {0x3a, "caps"}, {0x27, ";"}, {0x28, "'"}, {0x1c, "entr"},
        {0x2a, "shift"}, {0x33, ","}, {0x34, "."}, {0x35, "/"}, {0x36, "rshift"},
        {0x1d, "ctrl"}, /*{0x33, "fn"},*/ {{0x5b, true, false}, "win"}, {0x38, "alt"}, {0x39, "space"},
            {{0x38, true, false}, "ralt"}, {{0x5d, true, false}, "app"}, {{0x1d, true, false}, "rctrl"},
    };

    // Gets the keycode from the table
    // If it fails, we return a keycode to string
    std::string lookup(KeyboardKey k){
        auto first = keycodetable.find(k);
        if(first == keycodetable.end()){
            return k.to_string();
        }
        return std::string("\'") + first->second + '\'';
    }
}