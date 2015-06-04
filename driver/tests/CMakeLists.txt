# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

# Setup for all examples -- start --
include_directories (
    SYSTEM ${Boost_INCLUDE_DIRS}
    common
)

# Must set this to be able to run Boost Test Framework on Linux. This is because the Linux build of BLE Driver is
# dynamic and not static.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_definitions(-DBOOST_TEST_DYN_LINK)
endif()

set(Boost_USE_MULTITHREADED ON)

# Find the necessary boost components on system
find_package ( Boost 1.54.0 REQUIRED COMPONENTS system filesystem thread unit_test_framework chrono date_time )

add_compile_options(
    -g
)

# Forcing CXX compilation of C-files (this is relevant for all executables below)
set_source_files_properties(
    ../src/app_log.c
    ../src/app_uart_pc.c
    ../src/uart.cpp
    ../src/app_timer_pc.c
    common/app_log.c
    PROPERTIES
    LANGUAGE CXX
)

# Setup for all examples -- end --


# --------------------
# app_log_test related
# --------------------
add_executable(
    app_log_test
    app_log_test/tst_app_log.cpp
    ../src/app_log.c
)

target_link_libraries(
    app_log_test
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)


# ----------------------
# app_timer_test related
# ----------------------

add_executable(
    app_timer_test
    app_timer_test/tst_app_timer.cpp
    mock/timer_boost.cpp
    ../src/app_log.c
    ../src/app_timer_pc.c
    ../src/timer.cpp
)

target_include_directories(
    app_timer_test
    BEFORE
    PRIVATE
    mock)

target_link_libraries(
    app_timer_test
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)


# ---------------------
# app_uart_test related
# --------------------

add_executable(
    app_uart_test
    app_uart_test/main.cpp
    app_uart_test/tst_app_uart.cpp
    mock/uart_boost.cpp
    ../src/app_log.c
    ../src/app_uart_pc.c
    ../src/uart.cpp
)

target_link_libraries(
    app_uart_test
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

target_include_directories(
    app_uart_test
    BEFORE
    PRIVATE
    mock)

# --------------------
# uart_cpp_test related
# --------------------

add_executable(
    uart_cpp_test
    uart_cpp_test/tst_uart_boost.cpp
    uart_cpp_test/tst_uart_settings.cpp
    mock/uart_test.cpp
    common/app_log.c
    ../src/uart.cpp
    ../src/uart_boost.cpp
    ../src/uart_settings.cpp
    ../src/uart_settings_boost.cpp

)

target_link_libraries(
    uart_cpp_test
    ${Boost_LIBRARIES}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

target_include_directories(
    uart_cpp_test
    PRIVATE
    ../inc
    mock
)

if(MINGW)
    set_property(TARGET uart_cpp_test APPEND PROPERTY LINK_LIBRARIES ws2_32)
endif(MINGW)


# --------------------
# timer_cpp_test related
# --------------------

add_executable(
    timer_cpp_test
    timer_cpp_test/tst_timer_boost.cpp
    ../src/timer.cpp
    ../src/timer_boost.cpp
)

target_link_libraries(
    timer_cpp_test
    ${Boost_LIBRARIES}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

target_include_directories(
    timer_cpp_test
    PRIVATE
    ../inc
)

if(MINGW)
    set_property(TARGET timer_cpp_test APPEND PROPERTY LINK_LIBRARIES ws2_32)
endif(MINGW)

# ------------------
# The ctest section
# ------------------

enable_testing()

set(test_params --log_level=all --report_level=no)

add_test(NAME app_uart_test COMMAND "app_uart_test" ${test_params})
add_test(NAME app_timer_test COMMAND "app_timer_test" ${test_params})
add_test(NAME app_log_test COMMAND "app_log_test" ${test_params})

add_test(NAME timer_cpp_test COMMAND "timer_cpp_test" ${test_params})

if (NOT APPLE)
    add_test(NAME uart_cpp_test COMMAND "uart_cpp_test" ${test_params})
endif()