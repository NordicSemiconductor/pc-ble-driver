#Create install target
if(NOT CMAKE_INSTALL_PREFIX)
    set(NRF_BLE_DRIVER_INSTALL_ROOT ${CMAKE_BINARY_DIR}/artifacts/pc-ble-driver-${PC_BLE_DRIVER_VERSION})
else()
    set(NRF_BLE_DRIVER_INSTALL_ROOT ${CMAKE_INSTALL_PREFIX})
endif()

set(NRF_BLE_DRIVER_INCLUDE_PREFIX "${NRF_BLE_DRIVER_INSTALL_ROOT}/include/nrf/ble/driver")

install(
    DIRECTORY include/common DESTINATION ${NRF_BLE_DRIVER_INCLUDE_PREFIX}
    PATTERN "internal" EXCLUDE
    PATTERN "sdk_compat" EXCLUDE
)

# Directory include/common/sdk_compat is included without sdk_compat
# in SoftDevice. Moving those header files to common remove an extra
# include to think about
install(
    FILES 
        include/common/sdk_compat/nrf.h
        include/common/sdk_compat/compiler_abstraction.h
        include/common/sdk_compat/nrf_svc.h
    DESTINATION ${NRF_BLE_DRIVER_INCLUDE_PREFIX}/common
)

install(FILES "LICENSE" DESTINATION ${NRF_BLE_DRIVER_INSTALL_ROOT})

foreach(SD_API_VER ${SD_API_VERS})
    string(TOLOWER ${SD_API_VER} SD_API_VER_L)

    install(
        TARGETS ${PC_BLE_DRIVER_${SD_API_VER}_SHARED_LIB}
        LIBRARY DESTINATION ${NRF_BLE_DRIVER_INSTALL_ROOT}/lib/shared
        ARCHIVE DESTINATION ${NRF_BLE_DRIVER_INSTALL_ROOT}/lib/shared
        RUNTIME DESTINATION ${NRF_BLE_DRIVER_INSTALL_ROOT}/lib/shared
        COMPONENT SDK
    )

    install(
        TARGETS ${PC_BLE_DRIVER_${SD_API_VER}_STATIC_LIB}
        ARCHIVE DESTINATION ${NRF_BLE_DRIVER_INSTALL_ROOT}/lib/static
        COMPONENT SDK
    )

    if(SD_API_VER STREQUAL "SD_API_V6")
        install(
            DIRECTORY 
                src/${SD_API_VER_L}/sdk/components/softdevice/s140/headers/
            DESTINATION ${NRF_BLE_DRIVER_INCLUDE_PREFIX}/${SD_API_VER_L}
            COMPONENT SDK
        )
    endif()

    if(NOT SD_API_VER STREQUAL "SD_API_V6")
        install(
            DIRECTORY 
                src/${SD_API_VER_L}/sdk/components/softdevice/s132/headers/
            DESTINATION ${NRF_BLE_DRIVER_INCLUDE_PREFIX}/${SD_API_VER_L}
            COMPONENT SDK
        )
    endif()
endforeach(SD_API_VER)
