include(GNUInstallDirs)

if(MSVC)
    add_definitions(-DPC_BLE_DRIVER_STATIC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(spdlog CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)

set(TESTS_SOFTDEVICE_V2 )
set(TESTS_SOFTDEVICE_V3 )
set(TESTS_SOFTDEVICE_V5 )
set(TESTS_SOFTDEVICE_V6 )

function(setup_test_run)
    set(CONNECTIVITY_ROOT "${CMAKE_SOURCE_DIR}/hex")

    find_program(NRFC "nrfc")
    if(NOT NRFC)
        message(STATUS "nrfc not found, tests will not be ran.")
    endif()

    file(TO_NATIVE_PATH "${CONNECTIVITY_ROOT}/sd_api_v2/connectivity_${CONNECTIVITY_VERSION}_1m_with_s130_2.0.1.hex" SD_API_V2_S130_HEX)
    file(TO_NATIVE_PATH "${CONNECTIVITY_ROOT}/sd_api_v3/connectivity_${CONNECTIVITY_VERSION}_1m_with_s132_3.1.0.hex" SD_API_V3_S132_HEX)
    file(TO_NATIVE_PATH "${CONNECTIVITY_ROOT}/sd_api_v3/connectivity_${CONNECTIVITY_VERSION}_usb_with_s132_3.1.0.hex" SD_API_V3_S132_PCA10056_USB_HEX)
    file(TO_NATIVE_PATH "${CONNECTIVITY_ROOT}/sd_api_v5/connectivity_${CONNECTIVITY_VERSION}_1m_with_s132_5.1.0.hex" SD_API_V5_S132_HEX)
    file(TO_NATIVE_PATH "${CONNECTIVITY_ROOT}/sd_api_v5/connectivity_${CONNECTIVITY_VERSION}_usb_with_s132_5.1.0.hex" SD_API_V5_S132_PCA10056_USB_HEX)
    file(TO_NATIVE_PATH "${CONNECTIVITY_ROOT}/sd_api_v6/connectivity_${CONNECTIVITY_VERSION}_1m_with_s132_6.1.1.hex" SD_API_V6_S132_HEX)
    file(TO_NATIVE_PATH "${CONNECTIVITY_ROOT}/sd_api_v6/connectivity_${CONNECTIVITY_VERSION}_1m_with_s140_6.1.1.hex" SD_API_V6_S140_HEX)
    file(TO_NATIVE_PATH "${CONNECTIVITY_ROOT}/sd_api_v6/connectivity_${CONNECTIVITY_VERSION}_usb_with_s140_6.1.1.hex" SD_API_V6_S140_PCA10056_USB_HEX)

    set(SD_API_V2_S130_HEX ${SD_API_V2_S130_HEX} PARENT_SCOPE)
    set(SD_API_V3_S132_HEX ${SD_API_V3_S132_HEX} PARENT_SCOPE)
    set(SD_API_V3_S132_PCA10056_USB_HEX ${SD_API_V3_S132_PCA10056_USB_HEX} PARENT_SCOPE)
    set(SD_API_V5_S132_HEX ${SD_API_V5_S132_HEX} PARENT_SCOPE)
    set(SD_API_V5_S132_PCA10056_USB_HEX ${SD_API_V5_S132_PCA10056_USB_HEX} PARENT_SCOPE)
    set(SD_API_V6_S132_HEX ${SD_API_V6_S132_HEX} PARENT_SCOPE)
    set(SD_API_V6_S140_HEX ${SD_API_V6_S140_HEX} PARENT_SCOPE)
    set(SD_API_V6_S140_PCA10056_USB_HEX ${SD_API_V6_S140_PCA10056_USB_HEX} PARENT_SCOPE)
endfunction(setup_test_run)

function(setup_test)
    cmake_parse_arguments(
        SETUP_TEST
        ""
        "SOURCE_FILE;SOFTDEVICE_API_VER;TEST_LIST"
        SOURCE_TESTCASES
        ${ARGN}
    )

    set(SOFTDEVICE_API_VER "${SETUP_TEST_SOFTDEVICE_API_VER}")
    set(SOURCE_FILE "${SETUP_TEST_SOURCE_FILE}")
    set(SOURCE_TESTCASES "${SETUP_TEST_SOURCE_TESTCASES}")
    set(TEST_LIST "${SETUP_TEST_TEST_LIST}")

    set(TEST_UTIL_SRC
        util/src/logging.cpp
        util/src/test_environment.cpp
        util/src/test_util.cpp
        util/src/test_util_adapter_wrapper.cpp
        util/src/test_util_conversion.cpp
        util/src/test_util_role.cpp
    )

    get_filename_component(TEST_NAME ${SOURCE_FILE} NAME_WE)
    set(TEST_NAME "${TEST_NAME}_sd_api_v${SOFTDEVICE_API_VER}")

    # Build executable
    add_executable(${TEST_NAME} ${SOURCE_FILE} ${SOURCE_TESTCASES} ${TEST_UTIL_SRC})

    target_compile_definitions(${TEST_NAME} PRIVATE -DNRF_SD_BLE_API=${SOFTDEVICE_API_VER})
    target_include_directories(${TEST_NAME} 
        SYSTEM PRIVATE 
        ../include/sd_api_v${SOFTDEVICE_API_VER}
        ../include/common/sdk_compat
        ../include/common
        ../include/common/internal/transport
        ../include/common/config
    )

    target_include_directories(${TEST_NAME} SYSTEM PRIVATE util/include)

    target_link_libraries(${TEST_NAME} PRIVATE 
        nrf_ble_driver_sd_api_v${SOFTDEVICE_API_VER}_static
        Threads::Threads
        spdlog::spdlog
        Catch2::Catch2
    )

    if(NOT ${TEST_NAME} STREQUAL "test_uart_boost_v2")
        set(${TEST_LIST} ${${TEST_LIST}} ${TEST_NAME} PARENT_SCOPE)
    else()
        message(STATUS "${TEST_NAME} NOT ADDED to ${TEST_LIST} since the test is a serial port loopback test.")
    endif()

    install(
        TARGETS ${TEST_NAME}
        EXPORT ${PROJECT_NAME}-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT Tests
    )
endfunction(setup_test)

if(DISABLE_TESTS)
    set(TEST_TRANSPORT false)
    set(TEST_SOFTDEVICE_API false)
else()
    set(TEST_TRANSPORT true)
    set(TEST_SOFTDEVICE_API true)
endif()

if(TEST_TRANSPORT)
    file(GLOB transport_tests_src "transport/test_*.cpp")

    list(GET SD_API_VER_NUMS 0 ANY_SD_API_VERSION)

    foreach(transport_test_src ${transport_tests_src})
        # Use any SD API version for linking object files common between SD API versions
        setup_test(SOURCE_FILE ${transport_test_src} SOURCE_TESTCASES "" SOFTDEVICE_API_VER ${ANY_SD_API_VERSION} TEST_LIST TESTS_SOFTDEVICE_${ANY_SD_API_VERSION})
    endforeach(transport_test_src)
endif()

if(TEST_SOFTDEVICE_API)
    set(TESTS_SRC softdevice_api/test.cpp)
    set(TESTCASES_SRC 
        softdevice_api/testcase_advertising.cpp
        softdevice_api/testcase_driver_open_close.cpp
        softdevice_api/testcase_issue_128bit_uuid.cpp
        softdevice_api/testcase_issue_gh_112.cpp
        softdevice_api/testcase_issue_stuck_in_scan_mode.cpp
        softdevice_api/testcase_phy_update.cpp
        softdevice_api/testcase_rssi.cpp
        softdevice_api/testcase_security.cpp
        softdevice_api/testcase_util.cpp
    )

    foreach(SD_API_VER ${SD_API_VER_NUMS})
        foreach(TEST_SRC ${TESTS_SRC})
            setup_test(SOURCE_FILE ${TEST_SRC} SOURCE_TESTCASES ${TESTCASES_SRC} SOFTDEVICE_API_VER ${SD_API_VER} TEST_LIST TESTS_SOFTDEVICE_V${SD_API_VER})
        endforeach()
    endforeach(SD_API_VER)

    message(STATUS "Compiled tests are installed in directory \"${CMAKE_INSTALL_BINDIR}\"")
endif()


if(WIN32)
    string(REPLACE "\\" "/" VCPKG_ROOT_FWD $ENV{VCPKG_ROOT})
    if("${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" MATCHES "x64")
        set(VCPKG_TRIPLET "x64-windows")
    elseif("${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" MATCHES "X86")
        set(VCPKG_TRIPLET "x86-windows")
    else()
        message(WARNING "Could not find generator platform on Windows. Defaulting to x86. Was: ${CMAKE_GENERATOR_PLATFORM}")
        set(VCPKG_TRIPLET "x86-windows")
    endif()

    set(BINARY_SOURCE_PATH "${VCPKG_ROOT_FWD}/installed/${VCPKG_TRIPLET}")

    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set(BINARY_SOURCE_PATH "${BINARY_SOURCE_PATH}/debug/bin")
    else()
        set(BINARY_SOURCE_PATH "${BINARY_SOURCE_PATH}/bin")
    endif()

    if(CMAKE_BUILD_TYPE MATCHES Debug)
        install(FILES "${BINARY_SOURCE_PATH}/fmtd.dll" DESTINATION bin)
        install(FILES "${BINARY_SOURCE_PATH}/spdlogd.dll" DESTINATION bin)
    else()
        install(FILES "${BINARY_SOURCE_PATH}/fmt.dll" DESTINATION bin)
        install(FILES "${BINARY_SOURCE_PATH}/spdlog.dll" DESTINATION bin)
    endif()
endif()
