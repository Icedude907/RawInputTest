cmake_minimum_required(VERSION 3.19)

project(BackgroundInputTest VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(HotkeyAPITest "src/hotkey.cpp")
add_executable(RawInputTest "src/rawinput.cpp")
add_executable(RawInputTest2 "src/rawinput2.cpp")
target_link_libraries(RawInputTest2 hid)

add_executable(VirtualTerminalTest "src/virtualterminal.cpp")