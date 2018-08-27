enable_testing()

# Use the latest SoftDevice API version in the tests
#list(GET SD_API_VER_NUMS -1 SD_API_VER)

if(MSVC)
    add_definitions(-DPC_BLE_DRIVER_STATIC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories (
    ../3rdparty
    ../include/common/sdk_compat
    ../include/common
    ../include/common/internal/transport
)

function(setup_test source_file softdevice_api_ver)
    get_filename_component(test_name ${source_file} NAME_WE)
    set(test_name "${test_name}_v${softdevice_api_ver}")

    # Build executable
    add_executable(${test_name} ${source_file})

    target_compile_definitions(${test_name} PRIVATE -DNRF_SD_BLE_API=${softdevice_api_ver} -DNRF_LOG_FILENAME="${test_name}.txt")
    target_include_directories(${test_name} PRIVATE ../src/sd_api_v${softdevice_api_ver}/sdk/components/softdevice/s132/headers)
    target_include_directories(${test_name} PRIVATE ../src/sd_api_v${softdevice_api_ver}/sdk/components/softdevice/s140/headers)

    if(WIN32)
        target_link_libraries(${test_name} PRIVATE pc_ble_driver_static_sd_api_v${softdevice_api_ver})
    elseif(APPLE)
        target_link_libraries(${test_name} PRIVATE pc_ble_driver_static_sd_api_v${softdevice_api_ver})
    else()
        # Assume Linux
        target_link_libraries(${test_name} PRIVATE pc_ble_driver_static_sd_api_v${softdevice_api_ver} "pthread")
    endif()

    add_test(NAME ${test_name} COMMAND ${test_name})

    message(STATUS "Added test ${source_file}, with test name ${test_name}.")
endfunction(setup_test)

if(TEST_ALL)
    set(TEST_TRANSPORT true)
    set(TEST_SOFTDEVICE_API true)
endif()

if(TEST_TRANSPORT)
    file(GLOB transport_tests_src "transport/test_*.cpp")

    foreach(transport_test_src ${transport_tests_src})
        setup_test(${transport_test_src} 2)
    endforeach(transport_test_src)
endif(TEST_TRANSPORT)

if(TEST_SOFTDEVICE_API)
    message(STATUS "Using SoftDevice API version ${SD_API_VER} in tests.")

    file(GLOB tests_src "softdevice_api/test_*.cpp")

    foreach(SD_API_VER ${SD_API_VER_NUMS})
        foreach(test_src ${tests_src})
            setup_test(${test_src} ${SD_API_VER})
        endforeach(test_src)
    endforeach(SD_API_VER)
endif(TEST_SOFTDEVICE_API)
