cmake_minimum_required(VERSION 3.3)

include (cmake/pc-ble-driver.cmake)

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
foreach(SD_API_VER ${SD_API_VERS})
    string(TOLOWER ${SD_API_VER} SD_API_VER_L)
    file(GLOB_RECURSE LIB_SDK_${SD_API_VER}_C_SRC_FILES "src/${SD_API_VER_L}/sdk/*.c")
    file(GLOB_RECURSE LIB_SDK_${SD_API_VER}_CPP_SRC_FILES "src/${SD_API_VER_L}/sdk/*.cpp")
endforeach(SD_API_VER)

# Set C, CPP and platform source file groups and properties
foreach(SD_API_VER ${SD_API_VERS})
    set(LIB_${SD_API_VER}_C_SRC_FILES ${LIB_BASE_C_SRC_FILES}
                        ${LIB_SDK_${SD_API_VER}_C_SRC_FILES}
                        ${LIB_TRANSPORT_C_SRC_FILES}
    )

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

if(WIN32)
    # Disable CRT Warnings with Visual Studio
    set_source_files_properties(
        ${LIB_PLATFORM_C_SRC_FILES}
        PROPERTIES
        COMPILE_FLAGS "${CMAKE_C_FLAGS} -D_CRT_SECURE_NO_WARNINGS"
    )
    set_source_files_properties(
        ${LIB_PLATFORM_CPP_SRC_FILES}
        PROPERTIES
        COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS"
    )
endif()
# Add common compiler definitions
add_definitions(
    -DSD_RPC_EXPORTS
    -DHCI_LINK_CONTROL # Adds support for Link Control packets according to the HCI standard
)

# Add libraries
foreach(SD_API_VER ${SD_API_VERS})
    # Object library, from which both shared and static will be built
    add_library(${PC_BLE_DRIVER_${SD_API_VER}_OBJ_LIB} OBJECT ${LIB_${SD_API_VER}_C_SRC_FILES} ${LIB_${SD_API_VER}_CPP_SRC_FILES} ${LIB_PLATFORM_SRC_FILES})

    # shared libraries need PIC
    set_property(TARGET ${PC_BLE_DRIVER_${SD_API_VER}_OBJ_LIB} PROPERTY POSITION_INDEPENDENT_CODE 1)

    # actual shared and static libraries built from the same object files
    add_library(${PC_BLE_DRIVER_${SD_API_VER}_SHARED_LIB} SHARED $<TARGET_OBJECTS:${PC_BLE_DRIVER_${SD_API_VER}_OBJ_LIB}>)
    add_library(${PC_BLE_DRIVER_${SD_API_VER}_STATIC_LIB} STATIC $<TARGET_OBJECTS:${PC_BLE_DRIVER_${SD_API_VER}_OBJ_LIB}>)
endforeach(SD_API_VER)

# build test_uart executable
add_executable(test_uart test/test_uart.cpp)

# Set common include directories
include_directories(
        include/common
        include/common/sdk_compat
        include/common/internal
        include/common/internal/transport
        # Include boost as a system include so that the compiler doesn't nag about issues in boost header files
        SYSTEM ${Boost_INCLUDE_DIRS}
)

# Set per-SD API version include directories and compiler definitions
foreach(SD_API_VER ${SD_API_VERS})
    string(TOLOWER ${SD_API_VER} SD_API_VER_L)
    target_include_directories (${PC_BLE_DRIVER_${SD_API_VER}_OBJ_LIB} PRIVATE
        src/${SD_API_VER_L}/sdk/components/libraries/util
        src/${SD_API_VER_L}/sdk/components/serialization/application/codecs/common
        src/${SD_API_VER_L}/sdk/components/serialization/application/codecs/s130/serializers
        src/${SD_API_VER_L}/sdk/components/serialization/application/codecs/s132/serializers
        src/${SD_API_VER_L}/sdk/components/serialization/common
        src/${SD_API_VER_L}/sdk/components/serialization/common/struct_ser/s130
        src/${SD_API_VER_L}/sdk/components/serialization/common/struct_ser/s132
        src/${SD_API_VER_L}/sdk/components/softdevice/s132/headers
    )
    # Provide the NRF_SD_BLE_API_VERSION macro to each variant
    string(REGEX MATCH "[0-9]+$" _SD_API_VER_NUM "${SD_API_VER}")
    set(SD_API_VER_COMPILER_DEF_NUM "-D${SD_API_VER_COMPILER_DEF}=${_SD_API_VER_NUM}")
    #MESSAGE( STATUS "compiler def: " "${SD_API_VER_COMPILER_DEF_NUM}" )
    target_compile_definitions(${PC_BLE_DRIVER_${SD_API_VER}_OBJ_LIB} PRIVATE "${SD_API_VER_COMPILER_DEF_NUM}")
endforeach(SD_API_VER)

# Additional special linkage libraries
foreach(SD_API_VER ${SD_API_VERS})
    if(WIN32)
    elseif(APPLE)
        target_link_libraries(${PC_BLE_DRIVER_${SD_API_VER}_STATIC_LIB} PRIVATE "-framework CoreFoundation" "-framework IOKit")
        target_link_libraries(${PC_BLE_DRIVER_${SD_API_VER}_SHARED_LIB} PRIVATE "-framework CoreFoundation" "-framework IOKit")
        set_property(TARGET ${PC_BLE_DRIVER_${SD_API_VER}_STATIC_LIB} PROPERTY MACOSX_RPATH ON)
        set_property(TARGET ${PC_BLE_DRIVER_${SD_API_VER}_SHARED_LIB} PROPERTY MACOSX_RPATH ON)
    else()
        # Assume Linux
        target_link_libraries(${PC_BLE_DRIVER_${SD_API_VER}_STATIC_LIB} PRIVATE "udev" "pthread")
        target_link_libraries(${PC_BLE_DRIVER_${SD_API_VER}_SHARED_LIB} PRIVATE "udev" "pthread")
        target_link_libraries(test_uart PRIVATE "pthread")
    endif()

    # Specify libraries to link serialization library with
    target_link_libraries (${PC_BLE_DRIVER_${SD_API_VER}_SHARED_LIB} PRIVATE ${Boost_LIBRARIES})
    target_link_libraries (${PC_BLE_DRIVER_${SD_API_VER}_STATIC_LIB} PRIVATE ${Boost_LIBRARIES})
endforeach(SD_API_VER)


target_link_libraries(test_uart PRIVATE ${Boost_LIBRARIES})

