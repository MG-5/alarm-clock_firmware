cmake_minimum_required(VERSION 3.22)
set(CMAKE_PROJECT_NAME alarm-clock_firmware)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)
set(BUILD_TESTING OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Add custom compiler flags, will also add flags to C++ compiler
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter")

# Include toolchain file
include(cubemx/cmake/gcc-arm-none-eabi.cmake)

# Disable warnings for rwx segments, as they are normal for embedded systems
add_link_options(-Wl,--no-warn-rwx-segments)

set(core_BUILDCONFIGURATION "embedded")
add_subdirectory(core)

# Enable CMake support for languages
enable_language(C CXX ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cubemx/cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

add_subdirectory(src/rtc/Time)
add_subdirectory(src/util)

# Add sources to executable
target_sources(
    ${CMAKE_PROJECT_NAME} PRIVATE
    src/buttons/Buttons.cxx

    src/display/font/Font.cxx
    src/display/Display.cxx

    src/rtc/DS3231.cxx
    src/rtc/RealTimeClock.cxx

    src/state_machine/ButtonCallbacks.cxx
    src/state_machine/StateMachine.cxx

    src/Application.cxx
)

# Add include paths
target_include_directories(
    ${CMAKE_PROJECT_NAME} PRIVATE
    src
)

# Add linked libraries
target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    stm32cubemx
    core
    util
    gcem
    Time
)
