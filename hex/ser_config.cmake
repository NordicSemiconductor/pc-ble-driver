#[[
    This script is invoked from CMakeLists in this directory during the build steps 
    for the connectivity firmware.
]]

if(NOT DEFINED SDK_VERSION)
    message(FATAL_ERROR "SDK_VERSION not provided.")
endif()

if(NOT DEFINED SER_CONFIG_IN_PATH)
    message(FATAL_ERROR "SER_CONFIG_IN_PATH not provided.")
endif()

if(NOT DEFINED BAUD_RATE)
    message(FATAL_ERROR "BAUD_RATE not provided.")
endif()

if(SDK_VERSION STREQUAL "11")
    if(BAUD_RATE STREQUAL "115k2")
        set(SER_PHY_UART_BAUDRATE "UART_BAUDRATE_BAUDRATE_Baud115200")
    elseif(BAUD_RATE STREQUAL "1m" OR BAUD_RATE STREQUAL "usb")
        set(SER_PHY_UART_BAUDRATE "UART_BAUDRATE_BAUDRATE_Baud1M")
    endif()
elseif(SDK_VERSION STREQUAL "15")
    if(BAUD_RATE STREQUAL "115k2")
        set(SER_PHY_UART_BAUDRATE_VAL "115200")
    elseif(BAUD_RATE STREQUAL "1m" OR BAUD_RATE STREQUAL "usb")
        set(SER_PHY_UART_BAUDRATE_VAL "1000000")
    endif()
endif()

if(NOT DEFINED SER_PHY_UART_BAUDRATE AND NOT DEFINED SER_PHY_UART_BAUDRATE_VAL)
    message(FATAL_ERROR "Not able to find baud rate for SDK ${SDK_VERSION}.")
endif()

get_filename_component(SER_CONFIG_DIR "${SER_CONFIG_IN_PATH}" DIRECTORY)
set(SER_CONFIG_PATH "${SER_CONFIG_DIR}/ser_config.h")

message(STATUS "Configuring ${SER_CONFIG_IN_PATH} with baud rate ${SER_PHY_UART_BAUDRATE}${SER_PHY_UART_BAUDRATE_VAL}, output file ${SER_CONFIG_PATH}")
configure_file(${SER_CONFIG_IN_PATH} ${SER_CONFIG_PATH} @ONLY)
