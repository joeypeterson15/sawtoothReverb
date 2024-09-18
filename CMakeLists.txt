cmake_minimum_required(VERSION 3.10)
project(SawtoothRev)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_NAME} "/Users/joeypeterson/Desktop/AudioFile")

# Add your executable
add_executable(sawtooth_with_basic_reverb index.cpp)