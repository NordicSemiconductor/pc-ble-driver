if(MSVC)
    add_definitions(-DPC_BLE_DRIVER_STATIC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

find_package(Catch2 CONFIG REQUIRED)

include_directories (
    ../3rdparty
    ../include/common/sdk_compat
    ../include/common
    ../include/common/internal/transport
)

set(TESTS_SOFTDEVICE_V2 )
set(TESTS_SOFTDEVICE_V3 )
set(TESTS_SOFTDEVICE_V5 )
set(TESTS_SOFTDEVICE_V6 )

set(CONNECTIVITY_ROOT "${CMAKE_SOURCE_DIR}/hex")
find_program(NRFJPROG "nrfjprog")

if(NOT NRFJPROG)
    message(STATUS "nrfjprog not found, tests will not be ran.")
endif()

execute_process(
    COMMAND ${NRFJPROG} "--version" 
    OUTPUT_VARIABLE version_info
    RESULT_VARIABLE result
)

message(STATUS "nrfjprog reports:\nversions:${version_info}\nexit_code:${result}\n")

set(BUILD_TYPE )
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "CMAKE_BUILD_TYPE not set, probably because it is running in a multi target environment. Setting it to Debug.")
    set(BUILD_TYPE "Debug")
else()
    set(BUILD_TYPE "${CMAKE_BUILD_TYPE}")
endif()

file(TO_NATIVE_PATH "${CONNECTIVITY_ROOT}/sd_api_v2/connectivity_${CONNECTIVITY_VERSION}_1m_with_s130_2.0.1.hex" SD_API_V2_S130_HEX)
file(TO_NATIVE_PATH "${CONNECTIVITY_ROOT}/sd_api_v3/connectivity_${CONNECTIVITY_VERSION}_1m_with_s132_3.1.0.hex" SD_API_V3_S132_HEX)
file(TO_NATIVE_PATH "${CONNECTIVITY_ROOT}/sd_api_v3/connectivity_${CONNECTIVITY_VERSION}_usb_with_s132_3.1.0.hex" SD_API_V3_S132_PCA10056_USB_HEX)
file(TO_NATIVE_PATH "${CONNECTIVITY_ROOT}/sd_api_v5/connectivity_${CONNECTIVITY_VERSION}_1m_with_s132_5.1.0.hex" SD_API_V5_S132_HEX)
file(TO_NATIVE_PATH "${CONNECTIVITY_ROOT}/sd_api_v6/connectivity_${CONNECTIVITY_VERSION}_1m_with_s132_6.1.0.hex" SD_API_V6_S132_HEX)
file(TO_NATIVE_PATH "${CONNECTIVITY_ROOT}/sd_api_v6/connectivity_${CONNECTIVITY_VERSION}_1m_with_s140_6.1.0.hex" SD_API_V6_S140_HEX)
file(TO_NATIVE_PATH "${CONNECTIVITY_ROOT}/sd_api_v6/connectivity_${CONNECTIVITY_VERSION}_1m_with_s140_usb_6.1.0.hex" SD_API_V6_S140_PCA10056_USB_HEX)

function(parse_device_id device_id parse_error segger_sn serial_port)
    set(INVALID_FORMAT_MESSAGE "Device id ${device_id} is in an invalid format, must be <SEGGER_SERIAL_NUMBER>:<SERIAL_PORT>")

    string(REPLACE ":" " " DEVICE_INFO ${device_id})
    separate_arguments(DEVICE_INFO)
    list(LENGTH DEVICE_INFO DEVICE_INFO_COUNT)
    if(NOT DEVICE_INFO_COUNT EQUAL 2)
        set(${parse_error} ${INVALID_FORMAT_MESSAGE} PARENT_SCOPE)
        return()
    endif()

    list(GET DEVICE_INFO 0 DEVICE_SEGGER_SN)
    list(GET DEVICE_INFO 1 DEVICE_SERIAL_PORT)
    if(NOT DEVICE_SEGGER_SN OR NOT DEVICE_SERIAL_PORT)
        set(${parse_error} ${INVALID_FORMAT_MESSAGE} PARENT_SCOPE)
        return()
    endif()

    set(${segger_sn} ${DEVICE_SEGGER_SN} PARENT_SCOPE)
    set(${serial_port} ${DEVICE_SERIAL_PORT} PARENT_SCOPE)
endfunction()

function(add_softdevice_test target_name pca device_a device_b connfw tests_reporter tests)
    # Basic sanity checks
    if(NOT NRFJPROG)
        return()
    endif()

    if(NOT device_a OR NOT device_b)
        message(STATUS "Devices(s) missing for running tests in target ${target_name}")
        return()
    endif()

    set(COMMAND_NAME "${target_name}_CMD")

    set(DEVICE_A_SEGGER_SN )
    set(DEVICE_A_SERIAL_PORT )
    set(PARSE_ERROR )

    parse_device_id(${device_a} PARSE_ERROR DEVICE_A_SEGGER_SN DEVICE_A_SERIAL_PORT)

    if(PARSE_ERROR)
        message(WARNING "${PARSE_ERROR}")
        return()
    endif()

    parse_device_id(${device_b} PARSE_ERROR DEVICE_B_SEGGER_SN DEVICE_B_SERIAL_PORT)
    if(PARSE_ERROR)
        message(STATUS "${PARSE_ERROR}")
        return()
    endif()

    set(TEST_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")

    message(STATUS "TARGET: ${target_name} uses:")
    message(STATUS "DEVICE_A: SN:${DEVICE_A_SEGGER_SN} PORT:${DEVICE_A_SERIAL_PORT}")
    message(STATUS "DEVICE_B: SN:${DEVICE_B_SEGGER_SN} PORT:${DEVICE_B_SERIAL_PORT}")
    message(STATUS "TESTS: ${tests}")
    message(STATUS "REPORTER: ${tests_reporter}")

    add_custom_command(
        OUTPUT ${COMMAND_NAME}

        COMMAND ${NRFJPROG} --version
        COMMAND ${NRFJPROG} -s ${DEVICE_A_SEGGER_SN} --eraseall
        COMMAND ${NRFJPROG} -s ${DEVICE_A_SEGGER_SN} --program ${connfw}
        COMMAND ${NRFJPROG} -s ${DEVICE_A_SEGGER_SN} --reset

        COMMAND ${NRFJPROG} -s ${DEVICE_B_SEGGER_SN} --eraseall
        COMMAND ${NRFJPROG} -s ${DEVICE_B_SEGGER_SN} --program ${connfw}
        COMMAND ${NRFJPROG} -s ${DEVICE_B_SEGGER_SN} --reset
        COMMAND ${CMAKE_COMMAND} -E sleep 2

        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/test-reports" 
    )

    if(BLE_DRIVER_TEST_OPENCLOSE_ITERATIONS)
        set(OPENCLOSE_ITERATIONS ${BLE_DRIVER_TEST_OPENCLOSE_ITERATIONS})
    else()
        set(OPENCLOSE_ITERATIONS 10)
    endif()

    if(APPLE)
        set(BAUD_RATE 115200)
    else()
        set(BAUD_RATE 1000000)
    endif()

    # Extract hex filename only
    get_filename_component(connfw_filename "${connfw}" NAME)

    # Link tests together so they are ran in sequence
    foreach(integration_test IN LISTS tests)
        add_custom_command(
            OUTPUT ${COMMAND_NAME}
            APPEND COMMAND ${CMAKE_COMMAND} -E echo "Running tests from $<TARGET_FILE:${integration_test}> with connectivity ${connfw}."
        )

        add_custom_command(
            OUTPUT ${COMMAND_NAME}
            APPEND COMMAND ${CMAKE_COMMAND} -E env ${define_env}
            "$<TARGET_FILE:${integration_test}>"
            --port-a ${DEVICE_A_SERIAL_PORT}
            --port-b ${DEVICE_B_SERIAL_PORT}
            --iterations ${OPENCLOSE_ITERATIONS}
            --baud-rate ${BAUD_RATE}
            --log-level trace
            --hardware-info "hex:${connfw_filename},pca:${pca},segger_sn:${DEVICE_A_SEGGER_SN},${DEVICE_B_SEGGER_SN}"
            --reporter ${tests_reporter}
            --order lex
            --out "$<TARGET_FILE_DIR:${integration_test}>/test-reports/${target_name}-${integration_test}.xml" || (exit 0)
            DEPENDS ${integration_test}
        )
    endforeach()

    add_custom_target("${target_name}" DEPENDS ${COMMAND_NAME})
endfunction()

function(add_run_test_targets)
    # SoftDevice API version -> SoftDevice version -> PCBA
    #
    # SDv6:
    #    S132: PCA10040
    #    S140: PCA10056

    # SDv5:
    #    S132: PCA10040
    #    S132: PCA10056

    # SDv3:
    #    S132: PCA10040
    #    S132: PCA10056

    # SDv2:
    #    S130: PCA10028
    #    S130: PCA10031
    set(TEST_REPORTER "junit")

    set(RUN_TEST_TARGETS )

    if(BLE_DRIVER_TEST_PCA10028_A AND BLE_DRIVER_TEST_PCA10028_B)
        # SDv2
        if(2 IN_LIST SD_API_VER_NUMS)
            add_softdevice_test(
                run_test_sdv2_s130_pca10028
                pca10028
                "${BLE_DRIVER_TEST_PCA10028_A}"
                "${BLE_DRIVER_TEST_PCA10028_B}"
                "${SD_API_V2_S130_HEX}"
                "${TEST_REPORTER}"
                "${TESTS_SOFTDEVICE_V2}"
            )
            list(APPEND RUN_TEST_TARGETS run_test_sdv2_s130_pca10028)
        endif()
    endif()

    if(BLE_DRIVER_TEST_PCA10031_A AND BLE_DRIVER_TEST_PCA10031_B)
        # SDv2
        if(2 IN_LIST SD_API_VER_NUMS)
            add_softdevice_test(
                run_test_sdv2_s130_pca10031
                pca10031
                "${BLE_DRIVER_TEST_PCA10031_A}"
                "${BLE_DRIVER_TEST_PCA10031_B}"
                "${SD_API_V2_S130_HEX}"
                "${TEST_REPORTER}"
                "${TESTS_SOFTDEVICE_V2}"
            )
            list(APPEND RUN_TEST_TARGETS run_test_sdv2_s130_pca10031)
        endif()
    endif()

    if(BLE_DRIVER_TEST_PCA10040_A AND BLE_DRIVER_TEST_PCA10040_B)
        # SDv3
        if(3 IN_LIST SD_API_VER_NUMS)
            add_softdevice_test(
                run_test_sdv3_s132_pca10040
                pca10040
                "${BLE_DRIVER_TEST_PCA10040_A}"
                "${BLE_DRIVER_TEST_PCA10040_B}"
                "${SD_API_V3_S132_HEX}"
                "${TEST_REPORTER}"
                "${TESTS_SOFTDEVICE_V3}"
            )
            list(APPEND RUN_TEST_TARGETS run_test_sdv3_s132_pca10040)
        endif()

        # SDv5
        if(5 IN_LIST SD_API_VER_NUMS)
            add_softdevice_test(
                run_test_sdv5_s132_pca10040
                pca10040
                "${BLE_DRIVER_TEST_PCA10040_A}"
                "${BLE_DRIVER_TEST_PCA10040_B}"
                "${SD_API_V5_S132_HEX}"
                "${TEST_REPORTER}"
                "${TESTS_SOFTDEVICE_V5}"
            )
            list(APPEND RUN_TEST_TARGETS run_test_sdv5_s132_pca10040)
        endif()

        # SDv6
        if(6 IN_LIST SD_API_VER_NUMS)
            add_softdevice_test(
                run_test_sdv6_s132_pca10040
                pca10040
                "${BLE_DRIVER_TEST_PCA10040_A}"
                "${BLE_DRIVER_TEST_PCA10040_B}"
                "${SD_API_V6_S132_HEX}"
                "${TEST_REPORTER}"
                "${TESTS_SOFTDEVICE_V6}"
            )
            list(APPEND RUN_TEST_TARGETS run_test_sdv6_s132_pca10040)
        endif()
    endif()

    if(BLE_DRIVER_TEST_PCA10056_USB_A AND BLE_DRIVER_TEST_PCA10056_USB_B)
        # SDv3
        if(3 IN_LIST SD_API_VER_NUMS)
            add_softdevice_test(
                run_test_sdv3_s132_pca10056_usb
                pca10056
                "${BLE_DRIVER_TEST_PCA10056_USB_A}"
                "${BLE_DRIVER_TEST_PCA10056_USB_B}"
                "${SD_API_V3_S132_PCA10056_USB_HEX}"
                "${TEST_REPORTER}"
                "${TESTS_SOFTDEVICE_V3}"
            )
            list(APPEND RUN_TEST_TARGETS run_test_sdv3_s132_pca10056_usb)
        endif()

        # SDv6
        if(6 IN_LIST SD_API_VER_NUMS)
            add_softdevice_test(
                run_test_sdv6_s140_pca10056_usb
                pca10056
                "${BLE_DRIVER_TEST_PCA10056_USB_A}"
                "${BLE_DRIVER_TEST_PCA10056_USB_B}"
                "${SD_API_V6_S140_PCA10056_USB_HEX}"
                "${TEST_REPORTER}"
                "${TESTS_SOFTDEVICE_V6}"
            )
            list(APPEND RUN_TEST_TARGETS run_test_sdv6_s140_pca10056_usb)
        endif()
    endif()

    if(BLE_DRIVER_TEST_PCA10056_A AND BLE_DRIVER_TEST_PCA10056_B)
        # SDv3
        if(3 IN_LIST SD_API_VER_NUMS)
            add_softdevice_test(
                run_test_sdv3_s132_pca10056
                pca10056
                "${BLE_DRIVER_TEST_PCA10056_A}"
                "${BLE_DRIVER_TEST_PCA10056_B}"
                "${SD_API_V3_S132_HEX}"
                "${TEST_REPORTER}"
                "${TESTS_SOFTDEVICE_V3}"
            )
            list(APPEND RUN_TEST_TARGETS run_test_sdv3_s132_pca10056)
        endif()

        # SDv5
        if(5 IN_LIST SD_API_VER_NUMS)
            add_softdevice_test(
                run_test_sdv5_s132_pca10056
                pca10056
                "${BLE_DRIVER_TEST_PCA10056_A}"
                "${BLE_DRIVER_TEST_PCA10056_B}"
                "${SD_API_V5_S132_HEX}"
                "${TEST_REPORTER}"
                "${TESTS_SOFTDEVICE_V5}"
            )
            list(APPEND RUN_TEST_TARGETS run_test_sdv5_s132_pca10056)
        endif()

        # SDv6
        if(6 IN_LIST SD_API_VER_NUMS)
            add_softdevice_test(
                run_test_sdv6_s140_pca10056
                pca10056
                "${BLE_DRIVER_TEST_PCA10056_A}"
                "${BLE_DRIVER_TEST_PCA10056_B}"
                "${SD_API_V6_S140_HEX}"
                "${TEST_REPORTER}"
                "${TESTS_SOFTDEVICE_V6}"
            )
            list(APPEND RUN_TEST_TARGETS run_test_sdv6_s140_pca10056)
        endif()
    endif()

    add_custom_target(run_tests DEPENDS ${RUN_TEST_TARGETS})
endfunction(add_run_test_targets)

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

    message(STATUS "softdevice_api_ver:${softdevice_api_ver} source_file:${source_file} source_testcases:${source_testcases} test_list:${test_list}")

    get_filename_component(test_name ${source_file} NAME_WE)
    set(test_name "${test_name}_v${softdevice_api_ver}")

    # Build executable
    add_executable(${test_name} ${source_file} ${source_testcases} ${test_util_src})

    target_compile_definitions(${test_name} PRIVATE -DNRF_SD_BLE_API=${softdevice_api_ver})
    target_include_directories(${test_name} SYSTEM PRIVATE ../src/sd_api_v${softdevice_api_ver}/sdk/components/softdevice/s132/headers)
    target_include_directories(${test_name} SYSTEM PRIVATE ../src/sd_api_v${softdevice_api_ver}/sdk/components/softdevice/s140/headers)
    target_include_directories(${test_name} SYSTEM PRIVATE util/include)

    if(WIN32)
        target_link_libraries(${test_name} PRIVATE pc_ble_driver_static_sd_api_v${softdevice_api_ver} Catch2::Catch2)
    elseif(APPLE)
        target_link_libraries(${test_name} PRIVATE pc_ble_driver_static_sd_api_v${softdevice_api_ver} Catch2::Catch2)
    else()
        # Assume Linux
        target_link_libraries(${test_name} PRIVATE pc_ble_driver_static_sd_api_v${softdevice_api_ver} "pthread" Catch2::Catch2)
    endif()

    if(NOT ${test_name} STREQUAL "test_uart_boost_v2")
        set(${test_list} ${${test_list}} ${test_name} PARENT_SCOPE)
    else()
        message(STATUS "${test_name} NOT ADDED to ${test_list} since the test is a serial port loopback test.")
    endif()
endfunction(setup_test)

if(TEST_ALL)
    set(TEST_TRANSPORT true)
    set(TEST_SOFTDEVICE_API true)
endif()

if(TEST_TRANSPORT)
    file(GLOB transport_tests_src "transport/test_*.cpp")

    list(GET SD_API_VER_NUMS 0 ANY_SD_API_VERSION)
    message(STATUS "Linking common code with SoftDevice API version ${ANY_SD_API_VERSION}")

    foreach(transport_test_src ${transport_tests_src})
        # Use any SD API version for linking object files common between SD API versions
        setup_test(SOURCE_FILE ${transport_test_src} SOURCE_TESTCASES "" SOFTDEVICE_API_VER ${ANY_SD_API_VERSION} TEST_LIST TESTS_SOFTDEVICE_${ANY_SD_API_VERSION})
    endforeach(transport_test_src)
endif(TEST_TRANSPORT)

if(TEST_SOFTDEVICE_API)
    file(GLOB tests_src "softdevice_api/test_*.cpp")
    file(GLOB testcases_src "softdevice_api/testcase_*.cpp")

    foreach(SD_API_VER ${SD_API_VER_NUMS})
        foreach(test_src ${tests_src})
            setup_test(SOURCE_FILE ${test_src} SOURCE_TESTCASES ${testcases_src} SOFTDEVICE_API_VER ${SD_API_VER} TEST_LIST TESTS_SOFTDEVICE_V${SD_API_VER})
        endforeach(test_src)
    endforeach(SD_API_VER)

    add_run_test_targets()
endif(TEST_SOFTDEVICE_API)
