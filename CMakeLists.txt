# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(picomk C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
file(GLOB SRC_FILES CONFIGURE_DEPENDS *.c ble/*.c pinnacle/*.c keyboard/*.c led/*.c settings/*.c hid/*.c state/*.c peripheral/*.c)
add_executable(picomk ${SRC_FILES})

pico_generate_pio_header(picomk ${CMAKE_CURRENT_LIST_DIR}/led/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/led)

# pull in common dependencies
target_link_libraries(picomk
    pico_stdlib
    pico_btstack_ble
    pico_btstack_cyw43
    pico_cyw43_arch_none
    hardware_i2c
)

pico_enable_stdio_usb(picomk 1)
pico_enable_stdio_uart(picomk 0)

# btstack
target_include_directories(picomk PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    )
pico_btstack_make_gatt_header(picomk PRIVATE "${CMAKE_CURRENT_LIST_DIR}/ble/picomk.gatt")

# create map/bin/hex file etc.
pico_add_extra_outputs(picomk)

target_compile_definitions(picomk PRIVATE
    #WANT_HCI_DUMP=1 # This enables btstack debug
    DEBUG_MAIN=0
    DEBUG_BLE=0
    DEBUG_EVENT=0
    DEBUG_MATRIX_SCAN=0
    DEBUG_MATRIX_SCAN_DEEP=0
    DEBUG_PINNACLE=0
    DEBUG_PERIPHERAL=0
    )

# ROM and RAM utilization set to display at build time
target_link_options(picomk PRIVATE -Xlinker --print-memory-usage)

# Set warnings to display at build time
# target_compile_options(picomk PRIVATE -Wall -Wextra)
