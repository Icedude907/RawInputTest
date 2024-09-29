cmake_minimum_required(VERSION 3.19)

project(BackgroundInputTest VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Mingw linking
if ((CMAKE_BUILD_TYPE NOT EQUAL "Debug") AND (MINGW))
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
else()
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif()

# Background input tests
add_executable(HotkeyAPITest "src/tests/hotkey.cpp")
add_executable(RawInputTest "src/tests/rawinput.cpp")
add_executable(RawInputTest2 "src/tests/rawinput2.cpp")
target_link_libraries(RawInputTest2 hid)

# Random extra test for console output
add_executable(VirtualTerminalTest "src/tests/virtualterminal.cpp")

# Useful program
add_executable(RawInputTool "src/RawInputTool.cpp")
target_link_libraries(RawInputTest2 hid)