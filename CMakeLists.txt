set(CMAKE_TOOLCHAIN_FILE /home/nerochiaro/projects/arduino/arduino-cmake/cmake/ArduinoToolchain.cmake)
cmake_minimum_required(VERSION 2.8)

project(Cor C CXX)

generate_arduino_firmware(cor
    BOARD uno
    PORT /dev/ttyACM0
    SKETCH .
)
