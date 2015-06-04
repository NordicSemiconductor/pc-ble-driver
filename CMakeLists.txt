# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

cmake_minimum_required (VERSION 2.6)

# Do not allow in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "Prevented in-source build. Build out-of-source instead.")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

# Environment settings (nrf51_sdk location)
if("${NRF51_SDK_PATH}" STREQUAL "")
    message(FATAL_ERROR "NRF51_SDK_PATH must be set to a valid SDK path.")
endif()

set (nrf51_sdk ${NRF51_SDK_PATH})

# Project root path
set (repo_root_path ${CMAKE_SOURCE_DIR})

# Check if necessary parameters is passed to cmake
if("${SERIALIZATION_VERSION}" STREQUAL "")
    message(FATAL_ERROR "SERIALIZATION_VERSION not set. Not able to continue build. Please set with -DSERIALIZATION_VERSION=X.Y.Z invoking cmake.")
endif()

if("${SERIALIZATION_REVISION}" STREQUAL "")
    message(FATAL_ERROR "SERIALIZATION_REVISION not set. Not able to continue build. Please set with -DSERIALIZATION_REVISION=X when invoking cmake.")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
        -g -O0
    )

    add_definitions(
        -DDEBUG
    )
endif()


# Remove nRF51 hardware specific files from the SDK that prevents compilation.
# Since the C++ preprocessor will try to search the local directory first before searching
# other directories the we have to remove the files below. That way the preprocessor will use
# nrf_svc.h from override folder instead.
file(REMOVE ${nrf51_sdk}/components/softdevice/s130/headers/nrf_svc.h) # included by ${nrf51_sdk_components}/softdevice/s130/headers/ble_gap.h

# S130 specific include paths
set (nrf51_sdk_components ${nrf51_sdk}/components)
set (nrf51_sdk_codecs ${nrf51_sdk_components}/serialization/application/codecs)
set (nrf51_sdk_codecs_s130 ${nrf51_sdk_codecs}/s130)

set (s130_includes
    ${repo_root_path}/driver/inc
    ${repo_root_path}/driver/inc_override
    ${nrf51_sdk}/framework/cunit/include_override
    ${nrf51_sdk_components}/drivers_nrf/hal
    ${nrf51_sdk_components}/drivers_nrf/uart
    ${nrf51_sdk_components}/libraries/crc16
    ${nrf51_sdk_components}/libraries/scheduler
    ${nrf51_sdk_components}/libraries/timer
    ${nrf51_sdk_components}/libraries/util
    ${nrf51_sdk_components}/serialization/application/hal
    ${nrf51_sdk_components}/serialization/application/transport
    ${nrf51_sdk_components}/serialization/common
    ${nrf51_sdk_components}/serialization/common/transport
    ${nrf51_sdk_components}/serialization/common/transport/ser_phy
    ${nrf51_sdk_components}/softdevice/common/softdevice_handler
    ${nrf51_sdk_components}/toolchain/gcc
    ${nrf51_sdk_components}/toolchain
    ${nrf51_sdk_components}/device
    ${nrf51_sdk_components}/drivers_nrf/hal
    ${nrf51_sdk_codecs_s130}/serializers
    ${nrf51_sdk_components}/serialization/common/struct_ser/s130
    ${nrf51_sdk_components}/softdevice/s130 #needed for '#include "headers/nrf_soc.h"'
    ${nrf51_sdk_components}/softdevice/s130/headers
)

# Add subdirectories
if("${ARTIFACT}" STREQUAL "driver")
    add_subdirectory(driver)
elseif("${ARTIFACT}" STREQUAL "binding")
    add_subdirectory(python)
else()
    message(FATAL_ERROR "ARTIFACT variable has to be specified, it can be driver or binding")
endif()
