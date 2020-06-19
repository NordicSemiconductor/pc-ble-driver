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

    set(softdevice_api_ver "${SETUP_TEST_SOFTDEVICE_API_VER}")
    set(source_file "${SETUP_TEST_SOURCE_FILE}")
    set(source_testcases "${SETUP_TEST_SOURCE_TESTCASES}")
    set(test_list "${SETUP_TEST_TEST_LIST}")

    file(GLOB test_util_src "util/src/*.cpp")

    #message(STATUS "softdevice_api_ver:${softdevice_api_ver} source_file:${source_file} source_testcases:${source_testcases} test_list:${test_list}")

    get_filename_component(test_name ${source_file} NAME_WE)
    set(test_name "${test_name}_sd_api_v${softdevice_api_ver}")

    # Build executable
    add_executable(${test_name} ${source_file} ${source_testcases} ${test_util_src})

    target_compile_definitions(${test_name} PRIVATE -DNRF_SD_BLE_API=${softdevice_api_ver})
    target_include_directories(${test_name} 
        SYSTEM PRIVATE 
        ../include/sd_api_v${softdevice_api_ver}
        ../include/common/sdk_compat
        ../include/common
        ../include/common/internal/transport
        ../include/common/config
    )

    target_include_directories(${test_name} SYSTEM PRIVATE util/include)

    target_link_libraries(${test_name} PRIVATE 
        nrf_ble_driver_sd_api_v${softdevice_api_ver}_static
        Threads::Threads
        spdlog::spdlog
        Catch2::Catch2
    )

    if(NOT ${test_name} STREQUAL "test_uart_boost_v2")
        set(${test_list} ${${test_list}} ${test_name} PARENT_SCOPE)
    else()
        message(STATUS "${test_name} NOT ADDED to ${test_list} since the test is a serial port loopback test.")
    endif()

    install(
        TARGETS ${test_name}
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
    file(GLOB tests_src "softdevice_api/test.cpp")
    file(GLOB testcases_src "softdevice_api/testcase_*.cpp")

    foreach(SD_API_VER ${SD_API_VER_NUMS})
        foreach(test_src ${tests_src})
            setup_test(SOURCE_FILE ${test_src} SOURCE_TESTCASES ${testcases_src} SOFTDEVICE_API_VER ${SD_API_VER} TEST_LIST TESTS_SOFTDEVICE_V${SD_API_VER})
        endforeach(test_src)
    endforeach(SD_API_VER)

    message(STATUS "Compiled tests are installed in directory \"${CMAKE_INSTALL_BINDIR}\"")
endif()
