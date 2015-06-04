# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

project(python_swig_bindings C)

if(MINGW)
    set(SWIG_EXECUTABLE "$ENV{SWIG_ROOT}/swig.exe")
endif()

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(PythonLibs REQUIRED)

set(module_name s130_nrf51_ble_driver)
set(driver_name s130_nrf51_ble_driver)

# Add WIN32 specific settings
if(WIN32)
    add_definitions(
        -D_WIN32_WINNT=0x0502
    )
endif()

include_directories(
    ${PYTHON_INCLUDE_PATH}
    ${s130_includes}
)

set(CMAKE_SWIG_FLAGS "-threads")

# Add definitions used by compiler when compiling source files
add_definitions(
    -DSD_RPC_EXPORTS
)

set(src_file swig/ble_driver_py.i)

find_library(s130_ble_driver NAMES ${driver_name} PATHS ${CMAKE_BINARY_DIR}/driver)

# Create a directory to put the generated output from SWIG
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/swig")

set_property(SOURCE  ${src_file} PROPERTY SWIG_FLAGS "-Wall")

swig_add_module(${module_name} python ${src_file})

set_source_files_properties(${src_file} PROPERTIES CPLUSCPLUS ON)


swig_link_libraries(${module_name} ${PYTHON_LIBRARIES} ${s130_ble_driver})
