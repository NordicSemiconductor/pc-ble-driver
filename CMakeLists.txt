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

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
    message(FATAL_ERROR "Protocol implementation does not support big endian platforms.")
endif()

# Common source files
file(GLOB LIB_BASE_C_SRC_FILES "src/common/*.c")
file(GLOB LIB_BASE_CPP_SRC_FILES "src/common/*.cpp")

file(GLOB LIB_TRANSPORT_C_SRC_FILES "src/common/transport/*.c")
file(GLOB LIB_TRANSPORT_CPP_SRC_FILES "src/common/transport/*.cpp")

if(WIN32)
    file(GLOB LIB_PLATFORM_C_SRC_FILES "src/common/platform/win/*.c")
    file(GLOB LIB_PLATFORM_CPP_SRC_FILES "src/common/platform/win/*.cpp")
elseif(APPLE)
    file(GLOB LIB_PLATFORM_C_SRC_FILES "src/common/platform/macos_osx/*.c")
    file(GLOB LIB_PLATFORM_CPP_SRC_FILES "src/common/platform/macos_osx/*.cpp")
else()
    # Assume Linux
    file(GLOB LIB_PLATFORM_C_SRC_FILES "src/common/platform/linux/*.c")
    file(GLOB LIB_PLATFORM_CPP_SRC_FILES "src/common/platform/linux/*.cpp")
endif()

# SDK source files, different per API version

# Newer codecs from nRF5 SDK are backwards compatible to SoftDevice API v3
file(GLOB_RECURSE LIB_NEWER_SDK_API_COMMON_C_SRC_FILES "src/sd_api_common/sdk/*.c")

foreach(SD_API_VER ${SD_API_VERS})
    string(TOLOWER ${SD_API_VER} SD_API_VER_L)
    file(GLOB_RECURSE LIB_SDK_${SD_API_VER}_C_SRC_FILES "src/${SD_API_VER_L}/sdk/*.c")
    file(GLOB_RECURSE LIB_SDK_${SD_API_VER}_CPP_SRC_FILES "src/${SD_API_VER_L}/*.cpp")
endforeach(SD_API_VER)

# Set C, CPP and platform source file groups and properties
foreach(SD_API_VER ${SD_API_VERS})
    set(LIB_${SD_API_VER}_C_SRC_FILES ${LIB_BASE_C_SRC_FILES}
                        ${LIB_SDK_${SD_API_VER}_C_SRC_FILES}
                        ${LIB_TRANSPORT_C_SRC_FILES}
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
    set(CONNECTIVITY_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/hex/${SD_API_VER_L}")

    set(HEX_FILE_GLOB 
        "${CONNECTIVITY_DIRECTORY}/connectivity_${CONNECTIVITY_VERSION}*.hex"
        "${CONNECTIVITY_DIRECTORY}/*_softdevice.hex"
    )
    set(DFU_FILE_GLOB "${CONNECTIVITY_DIRECTORY}/connectivity_${CONNECTIVITY_VERSION}*.zip")

    set(LICENSE_FILE_GLOB 
        "${CONNECTIVITY_DIRECTORY}/*licence-agreement.txt"
        "${CONNECTIVITY_DIRECTORY}/*license-agreement.txt"
    )

    file(GLOB_RECURSE ${SD_API_VER}_HEX_FILES ${HEX_FILE_GLOB})
    file(GLOB_RECURSE ${SD_API_VER}_DFU_FILES ${DFU_FILE_GLOB})
    file(GLOB_RECURSE ${SD_API_VER}_LICENSE_FILES ${LICENSE_FILE_GLOB})

    set(${SD_API_VER}_CONNECTIVITY_FILES )
    list(APPEND ${SD_API_VER}_CONNECTIVITY_FILES ${${SD_API_VER}_HEX_FILES})
    list(APPEND ${SD_API_VER}_CONNECTIVITY_FILES ${${SD_API_VER}_DFU_FILES})
    list(APPEND ${SD_API_VER}_CONNECTIVITY_FILES ${${SD_API_VER}_LICENSE_FILES})

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

if(COMPILE_CONNECTIVITY)
    add_subdirectory(hex EXCLUDE_FROM_ALL)
endif()

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
