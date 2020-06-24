cmake_minimum_required(VERSION 3.11)

set(NRF_BLE_DRIVER_DEFAULT_VERSION "0.0.0")

if(NOT DEFINED NRF_BLE_DRIVER_VERSION)
    message(STATUS "NRF_BLE_DRIVER_VERSION not specified, setting it to ${NRF_BLE_DRIVER_DEFAULT_VERSION}")
    set(NRF_BLE_DRIVER_VERSION "${NRF_BLE_DRIVER_DEFAULT_VERSION}")
endif()

set(NRF_BLE_DRIVER_CONNECTIVITY_DEFAULT_VERSION "0.0.0")

# Add connectivity firmware building
if(NOT DEFINED CONNECTIVITY_VERSION)
    message(STATUS "CONNECTIVITY_VERSION not specified, setting it to ${NRF_BLE_DRIVER_CONNECTIVITY_DEFAULT_VERSION}")
    set(CONNECTIVITY_VERSION "${NRF_BLE_DRIVER_CONNECTIVITY_DEFAULT_VERSION}" CACHE STRING "" FORCE)
else()
    set(CONNECTIVITY_VERSION "${CONNECTIVITY_VERSION}" CACHE STRING "" FORCE)
endif()

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

project(
    nrf-ble-driver
    VERSION ${NRF_BLE_DRIVER_VERSION}
)

include(cmake/nrf-ble-driver.cmake)
include(cmake/softdevice-file-list.cmake)

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
    message(FATAL_ERROR "Protocol implementation does not support big endian platforms.")
endif()

# Common source files
set(LIB_BASE_CPP_SRC_FILES 
    src/common/adapter_internal.cpp
    src/common/app_ble_gap.cpp
    src/common/ble_common.cpp
    src/common/sd_rpc_impl.cpp
)

set(LIB_TRANSPORT_CPP_SRC_FILES 
    src/common/transport/h5.cpp
    src/common/transport/h5_transport.cpp
    src/common/transport/serialization_transport.cpp
    src/common/transport/slip.cpp
    src/common/transport/transport.cpp
    src/common/transport/uart_settings.cpp
    src/common/transport/uart_settings_boost.cpp
    src/common/transport/uart_transport.cpp
)

if(WIN32)
    set(LIB_PLATFORM_C_SRC_FILES
        src/common/platform/win/disphelper.c
    )

    set(LIB_PLATFORM_CPP_SRC_FILES
        src/common/platform/win/enumser.cpp
        src/common/platform/win/jlinkid_reg_lookup.cpp
        src/common/platform/win/serial_port_enum.cpp
    )
elseif(APPLE)
    set(LIB_PLATFORM_CPP_SRC_FILES 
        src/common/platform/macos_osx/serial_port_enum.cpp
    )
else()
    # Assume Linux
    set(LIB_PLATFORM_CPP_SRC_FILES 
        src/common/platform/linux/serial_port_enum.cpp
    )
endif()

# SDK source files, different per API version

# Newer codecs from nRF5 SDK are backwards compatible to SoftDevice API v3
set(LIB_NEWER_SDK_API_COMMON_C_SRC_FILES
    src/sd_api_common/sdk/components/serialization/common/ble_serialization.c
    src/sd_api_common/sdk/components/serialization/common/cond_field_serialization.c
    src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gap_struct_serialization.c
    src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gattc_struct_serialization.c
    src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatts_struct_serialization.c
    src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatt_struct_serialization.c
    src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_l2cap_struct_serialization.c
    src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_struct_serialization.c
)

foreach(SD_API_VER ${SD_API_VERS})
    string(TOLOWER ${SD_API_VER} SD_API_VER_L)
    file(GLOB_RECURSE LIB_SDK_${SD_API_VER}_C_SRC_FILES "src/${SD_API_VER_L}/sdk/*.c")
    file(GLOB_RECURSE LIB_SDK_${SD_API_VER}_CPP_SRC_FILES "src/${SD_API_VER_L}/*.cpp")
endforeach(SD_API_VER)

# Set C, CPP and platform source file groups and properties
foreach(SD_API_VER ${SD_API_VERS})
    set(LIB_${SD_API_VER}_C_SRC_FILES
                        ${LIB_SDK_${SD_API_VER}_C_SRC_FILES}
    )

    if(SD_API_VER STRGREATER "SD_API_V2")
        list(APPEND LIB_${SD_API_VER}_C_SRC_FILES ${LIB_NEWER_SDK_API_COMMON_C_SRC_FILES})
    endif()

    set(LIB_${SD_API_VER}_CPP_SRC_FILES ${LIB_BASE_CPP_SRC_FILES}
                          ${LIB_SDK_${SD_API_VER}_CPP_SRC_FILES}
                          ${LIB_TRANSPORT_CPP_SRC_FILES}
    )

    # Force .c files to be compiled with the C++ compiler
    if(NOT APPLE)
        set_source_files_properties(
            ${LIB_C_${SD_API_VER}_SRC_FILES}
            PROPERTIES
            LANGUAGE CXX
        )
    endif()
endforeach(SD_API_VER)

set(LIB_PLATFORM_SRC_FILES ${LIB_PLATFORM_C_SRC_FILES} ${LIB_PLATFORM_CPP_SRC_FILES})

# Add common compiler definitions
add_definitions(
    -DASIO_STANDALONE
    -DSD_RPC_EXPORTS
    -DHCI_LINK_CONTROL # Adds support for Link Control packets according to the HCI standard
)

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(COMPILE_CONNECTIVITY)
    set(SDK_HEX_FILES)
    add_subdirectory(hex EXCLUDE_FROM_ALL)
    nrf_create_compile_targets(SDK_HEX_FILES)
endif()

# Add libraries
foreach(SD_API_VER ${SD_API_VERS})
    # Object library, from which both shared and static will be built
    add_library(${NRF_BLE_DRIVER_${SD_API_VER}_OBJ_LIB} OBJECT ${LIB_${SD_API_VER}_C_SRC_FILES} ${LIB_${SD_API_VER}_CPP_SRC_FILES} ${LIB_PLATFORM_SRC_FILES})

    # shared libraries need PIC
    set_property(TARGET ${NRF_BLE_DRIVER_${SD_API_VER}_OBJ_LIB} PROPERTY POSITION_INDEPENDENT_CODE 1)

    # actual shared and static libraries built from the same object files
    add_library(${NRF_BLE_DRIVER_${SD_API_VER}_SHARED_LIB} SHARED $<TARGET_OBJECTS:${NRF_BLE_DRIVER_${SD_API_VER}_OBJ_LIB}>)
    add_library(${NRF_BLE_DRIVER_${SD_API_VER}_STATIC_LIB} STATIC $<TARGET_OBJECTS:${NRF_BLE_DRIVER_${SD_API_VER}_OBJ_LIB}>)

    string(TOLOWER ${SD_API_VER} SD_API_VER_L)

    target_include_directories(
        ${NRF_BLE_DRIVER_${SD_API_VER}_SHARED_LIB} INTERFACE
        $<INSTALL_INTERFACE:include/${SD_API_VER_L}>
    )

    target_include_directories(
        ${NRF_BLE_DRIVER_${SD_API_VER}_STATIC_LIB} INTERFACE
        $<INSTALL_INTERFACE:include/${SD_API_VER_L}>
    )

    set(${SD_API_VER}_PUBLIC_HEADERS
        include/common/sd_rpc.h
        include/common/sd_rpc_types.h
        include/common/adapter.h
        include/common/sdk_compat/nrf.h
        include/common/sdk_compat/nrf_svc.h
        include/common/config/platform.h
        include/${SD_API_VER_L}/ble.h
        include/${SD_API_VER_L}/ble_err.h
        include/${SD_API_VER_L}/ble_gap.h
        include/${SD_API_VER_L}/ble_gatt.h
        include/${SD_API_VER_L}/ble_gattc.h
        include/${SD_API_VER_L}/ble_gatts.h
        include/${SD_API_VER_L}/ble_hci.h
        include/${SD_API_VER_L}/ble_l2cap.h
        include/${SD_API_VER_L}/ble_ranges.h
        include/${SD_API_VER_L}/ble_types.h
        include/${SD_API_VER_L}/nrf_error.h
    )

    string(TOLOWER ${SD_API_VER} SD_API_VER_L)

    if(COMPILE_CONNECTIVITY)
        set(CONNECTIVITY_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/hex/${SD_API_VER_L}")

        set(SOFTDEVICE_FILE_GLOB
            "${CONNECTIVITY_DIRECTORY}/*licence-agreement.txt"
            "${CONNECTIVITY_DIRECTORY}/*license-agreement.txt"
            "${CONNECTIVITY_DIRECTORY}/*_softdevice.hex"
        )

        file(GLOB_RECURSE ${SD_API_VER}_SOFTDEVICE_FILES ${SOFTDEVICE_FILE_GLOB})

        set(${SD_API_VER}_CONNECTIVITY_FILES "")
        set(CURRENT_SD_FOUND FALSE)

        foreach(ver ${SDK_HEX_FILES})
            if(ver MATCHES "^_")
                string(REGEX MATCH "_CONNECTIVITY_ARTIFACTS_SDv([0-9])" SDK_HEX_FILE_SD_VERSION "${ver}")

                if(NOT ${MATCHES})
                    message(FATAL_ERROR "Was not able to match SD_API version ${ver}")
                endif()

                set(SD_API_EQUAL "SD_API_V${CMAKE_MATCH_1}")

                if("${SD_API_EQUAL}" STREQUAL "${SD_API_VER}")
                    set(CURRENT_SD_FOUND TRUE)
                else()
                    set(CURRENT_SD_FOUND FALSE)
                endif()
            endif()

            if(CURRENT_SD_FOUND)
                if (ver MATCHES "^\\|")
                    string(SUBSTRING "${ver}" 1 -1 HEX)
                    list(APPEND ${SD_API_VER}_CONNECTIVITY_FILES ${HEX})
                endif()
            endif()
        endforeach()

        list(APPEND ${SD_API_VER}_CONNECTIVITY_FILES ${${SD_API_VER}_SOFTDEVICE_FILES})
    endif(COMPILE_CONNECTIVITY)

    if(WIN32)
        set_target_properties(
            ${NRF_BLE_DRIVER_${SD_API_VER}_SHARED_LIB} PROPERTIES
            PUBLIC_HEADER "${${SD_API_VER}_PUBLIC_HEADERS}"
            OUTPUT_NAME "nrf-ble-driver-${SD_API_VER_L}"
            RELEASE_POSTFIX "${MSVC_TOOLSET}-mt-${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR}_${PROJECT_VERSION_PATCH}"
            RELWITHDEBINFO_POSTFIX "${MSVC_TOOLSET}-mt-${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR}_${PROJECT_VERSION_PATCH}"
            MINSIZEREL_POSTFIX "${MSVC_TOOLSET}-mt-${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR}_${PROJECT_VERSION_PATCH}"
            DEBUG_POSTFIX "${MSVC_TOOLSET}-mt-gd-${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR}_${PROJECT_VERSION_PATCH}"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
            RESOURCE "${${SD_API_VER}_CONNECTIVITY_FILES}"
        )

        set_target_properties(
            ${NRF_BLE_DRIVER_${SD_API_VER}_STATIC_LIB} PROPERTIES
            PUBLIC_HEADER "${${SD_API_VER}_PUBLIC_HEADERS}"
            OUTPUT_NAME "nrf-ble-driver-${SD_API_VER_L}"
            RELEASE_POSTFIX "${MSVC_TOOLSET}-mt-static-${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR}_${PROJECT_VERSION_PATCH}"
            RELWITHDEBINFO_POSTFIX "${MSVC_TOOLSET}-mt-static-${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR}_${PROJECT_VERSION_PATCH}"
            MINSIZEREL_POSTFIX "${MSVC_TOOLSET}-mt-static-${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR}_${PROJECT_VERSION_PATCH}"
            DEBUG_POSTFIX "${MSVC_TOOLSET}-mt-static-gd-${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR}_${PROJECT_VERSION_PATCH}"
            RESOURCE "${${SD_API_VER}_CONNECTIVITY_FILES}"
        )
    else()
        set_target_properties(
            ${NRF_BLE_DRIVER_${SD_API_VER}_SHARED_LIB} PROPERTIES
            COMPILE_DEFINITIONS "DLL_EXPORT"
            PUBLIC_HEADER "${${SD_API_VER}_PUBLIC_HEADERS}"
            OUTPUT_NAME "nrf-ble-driver-${SD_API_VER_L}"
            VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
            SOVERSION "${PROJECT_VERSION_MAJOR}"
            PREFIX "lib"
            RESOURCE "${${SD_API_VER}_CONNECTIVITY_FILES}"
        )

        set_target_properties(
            ${NRF_BLE_DRIVER_${SD_API_VER}_STATIC_LIB} PROPERTIES
            PUBLIC_HEADER "${${SD_API_VER}_PUBLIC_HEADERS}"
            OUTPUT_NAME "nrf-ble-driver-${SD_API_VER_L}"
            PREFIX "lib"
            RESOURCE "${${SD_API_VER}_CONNECTIVITY_FILES}"
        )
    endif()
endforeach(SD_API_VER)

# Set common include directories
find_path(ASIO_INCLUDE_DIR asio.hpp)

# Set per-SD API version include directories and compiler definitions
foreach(SD_API_VER ${SD_API_VERS})
    string(TOLOWER ${SD_API_VER} SD_API_VER_L)

    target_include_directories(${NRF_BLE_DRIVER_${SD_API_VER}_OBJ_LIB} SYSTEM PRIVATE
        src/sd_api_common/sdk/components/libraries/util
        src/${SD_API_VER_L}/sdk/components/serialization/application/codecs/common
    )

    # Newer codecs from nRF5 SDK are backwards compatible to SoftDevice API v3
    if(SD_API_VER STRGREATER "SD_API_V2")
        target_include_directories(${NRF_BLE_DRIVER_${SD_API_VER}_OBJ_LIB} SYSTEM PRIVATE
            src/${SD_API_VER_L}/sdk/components/serialization/application/codecs/s132/serializers
            src/${SD_API_VER_L}/sdk/components/serialization/application/codecs/ble/serializers
            src/sd_api_common/sdk/components/serialization/common
            src/sd_api_common/sdk/components/serialization/common/struct_ser/ble
        )
    else()
        target_include_directories(${NRF_BLE_DRIVER_${SD_API_VER}_OBJ_LIB} SYSTEM PRIVATE
            src/${SD_API_VER_L}/sdk/components/serialization/application/codecs/s130/serializers
            src/${SD_API_VER_L}/sdk/components/serialization/common
            src/${SD_API_VER_L}/sdk/components/serialization/common/struct_ser/s130
        )
    endif()

    target_include_directories(${NRF_BLE_DRIVER_${SD_API_VER}_OBJ_LIB} SYSTEM PRIVATE
        include/common
        include/common/internal
        include/common/internal/transport
        ${ASIO_INCLUDE_DIR}
    )

    target_include_directories(${NRF_BLE_DRIVER_${SD_API_VER}_OBJ_LIB} SYSTEM PRIVATE
            include/common/config
            include/common/sdk_compat
            include/${SD_API_VER_L}
    )

    # Provide the NRF_SD_BLE_API_VERSION macro to each variant
    string(REGEX MATCH "[0-9]+$" _SD_API_VER_NUM "${SD_API_VER}")
    set(SD_API_VER_COMPILER_DEF_NUM "-D${SD_API_VER_COMPILER_DEF}=${_SD_API_VER_NUM}")
    #MESSAGE( STATUS "compiler def: " "${SD_API_VER_COMPILER_DEF_NUM}" )
    target_compile_definitions(${NRF_BLE_DRIVER_${SD_API_VER}_OBJ_LIB} PRIVATE "${SD_API_VER_COMPILER_DEF_NUM}")
endforeach(SD_API_VER)

# Additional special linkage libraries
foreach(SD_API_VER ${SD_API_VERS})
    if(WIN32)
    elseif(APPLE)
        target_link_libraries(${NRF_BLE_DRIVER_${SD_API_VER}_STATIC_LIB} PRIVATE "-framework CoreFoundation" "-framework IOKit")
        target_link_libraries(${NRF_BLE_DRIVER_${SD_API_VER}_SHARED_LIB} PRIVATE "-framework CoreFoundation" "-framework IOKit")
        set_property(TARGET ${NRF_BLE_DRIVER_${SD_API_VER}_STATIC_LIB} PROPERTY MACOSX_RPATH ON)
        set_property(TARGET ${NRF_BLE_DRIVER_${SD_API_VER}_SHARED_LIB} PROPERTY MACOSX_RPATH ON)
    else()
        # Assume Linux
        target_link_libraries(${NRF_BLE_DRIVER_${SD_API_VER}_STATIC_LIB} PRIVATE "udev" "pthread")
        target_link_libraries(${NRF_BLE_DRIVER_${SD_API_VER}_SHARED_LIB} PRIVATE "udev" "pthread")
    endif()
endforeach(SD_API_VER)

# Add tests
if(NOT DEFINED DISABLE_TESTS)
    add_subdirectory(test)
else()
    message(STATUS "Disabling tests")
endif()

# Add examples
if(NOT DEFINED DISABLE_EXAMPLES)
    add_subdirectory(examples)
else()
    message(STATUS "Disabling examples")
endif()

# Add install target
include(cmake/install-target.cmake)

include (cmake/clang-dev-tools.cmake)

# Create package
set(ARCH_SUFFIX "unknown")

if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    set(ARCH_SUFFIX "x86_64")
else()
    set(ARCH_SUFFIX "x86_32")
endif()

if(APPLE)
    set(OS_SUFFIX "macos")
elseif(WIN32)
    set(OS_SUFFIX "win")
else()
    #Assume Linux
    set(OS_SUFFIX "linux")
endif()

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${NRF_BLE_DRIVER_VERSION}-${OS_SUFFIX}_${ARCH_SUFFIX}")

include(CPack)
