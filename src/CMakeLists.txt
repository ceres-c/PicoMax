# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/usr/share/pico-sdk")

set(PICO_BOARD pico CACHE STRING "Board type")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(picomax C CXX ASM)


# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Check build env
if (NOT TARGET tinyusb_device)
  message(FATAL_ERROR "not building picomax because TinyUSB submodule is not initialized in the SDK")
endif()
if(NOT PICO_ON_DEVICE)
  message(WARNING "Not building picomax for Pico target. Are you sure you want to do this?")
endif()

# Add executable. Default name is the project name, version 0.1

add_executable(picomax picomax.c glitch.c)
add_subdirectory(icsp)

pico_set_program_name(picomax "picomax")
pico_set_program_version(picomax "0.1")

pico_enable_stdio_usb(picomax 1)
pico_enable_stdio_uart(picomax 0)

pico_generate_pio_header(picomax ${CMAKE_CURRENT_LIST_DIR}/glitch.pio)

# Add the standard library to the build
target_link_libraries(picomax
  pico_stdlib
  hardware_pio
  icsp
  hardware_i2c
)

# Add the standard include files to the build
target_include_directories(picomax PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

pico_add_extra_outputs(picomax)
