include(GNUInstallDirs)

function(setup_example example_directory)
    set(EXAMPLE_NAME_BASE "${example_directory}")
    file(GLOB EXAMPLE_SOURCE_FILES "${example_directory}/*.c")

    # Build executable
    foreach(SD_API_VER ${SD_API_VER_NUMS})
        set(EXAMPLE_NAME "${EXAMPLE_NAME_BASE}_sd_api_v${SD_API_VER}")
        add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE_FILES})

        target_compile_definitions(${EXAMPLE_NAME} PRIVATE -DNRF_SD_BLE_API=${SD_API_VER})

        target_include_directories(${EXAMPLE_NAME} PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/sd_api_v${SD_API_VER}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/common/config>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/common/sdk_compat>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/common>

            $<INSTALL_INTERFACE:include/sd_api_v${SD_API_VER}>
        )

        if(WIN32)
            target_link_libraries(${EXAMPLE_NAME} PRIVATE nrf_ble_driver_sd_api_v${SD_API_VER}_static)
        elseif(APPLE)
            target_link_libraries(${EXAMPLE_NAME} PRIVATE nrf_ble_driver_sd_api_v${SD_API_VER}_static)
        else()
            # Assume Linux
            target_link_libraries(${EXAMPLE_NAME} PRIVATE nrf_ble_driver_sd_api_v${SD_API_VER}_static "pthread")
        endif()

        install(
            TARGETS ${EXAMPLE_NAME}
            EXPORT ${PROJECT_NAME}-targets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT Examples
        )
    endforeach()
endfunction(setup_example)

link_directories (
    ../build
)

if(MSVC)
    add_definitions(-DPC_BLE_DRIVER_STATIC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

setup_example(heart_rate_monitor)
setup_example(heart_rate_collector)

message(STATUS "Compiled examples are installed in directory \"${CMAKE_INSTALL_BINDIR}\"")
