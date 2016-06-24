cmake_minimum_required(VERSION 3.3)

include (cmake/pc-ble-driver.cmake)

# Include boost as a system include so that the compiler do not nag about issues in boost header files
include_directories (
    src/serialization
    include
    include/internal
    include/internal/transport
    include/internal/sdk/codec
    include/internal/sdk/misc
    SYSTEM ${Boost_INCLUDE_DIRS}
)

file(GLOB LIB_BASE_C_SRC_FILES "src/*.c")
file(GLOB LIB_BASE_CPP_SRC_FILES "src/*.cpp")

file(GLOB_RECURSE LIB_SDK_C_SRC_FILES "src/sdk/*.c")
file(GLOB_RECURSE LIB_SDK_CPP_SRC_FILES "src/sdk/*.cpp")

file(GLOB LIB_TRANSPORT_C_SRC_FILES "src/transport/*.c")
file(GLOB LIB_TRANSPORT_CPP_SRC_FILES "src/transport/*.cpp")

if(WIN32)
    file(GLOB LIB_PLATFORM_C_SRC_FILES "src/platform/win/*.c")
    file(GLOB LIB_PLATFORM_CPP_SRC_FILES "src/platform/win/*.cpp")
elseif(APPLE)
    file(GLOB LIB_PLATFORM_C_SRC_FILES "src/platform/macos_osx/*.c")
    file(GLOB LIB_PLATFORM_CPP_SRC_FILES "src/platform/macos_osx/*.cpp")
else()
    # Assume Linux
    file(GLOB LIB_PLATFORM_C_SRC_FILES "src/platform/linux/*.c")
    file(GLOB LIB_PLATFORM_CPP_SRC_FILES "src/platform/linux/*.cpp")
endif()

set(LIB_C_SRC_FILES ${LIB_BASE_C_SRC_FILES}
                    ${LIB_SDK_C_SRC_FILES}
                    ${LIB_TRANSPORT_C_SRC_FILES}
)

set(LIB_CPP_SRC_FILES ${LIB_BASE_CPP_SRC_FILES}
                      ${LIB_SDK_CPP_SRC_FILES}
                      ${LIB_TRANSPORT_CPP_SRC_FILES}
)


set(LIB_PLATFORM_SRC_FILES ${LIB_PLATFORM_C_SRC_FILES} ${LIB_PLATFORM_CPP_SRC_FILES})

# Add definitions used by compiler when compiling source files
add_definitions(
    -DSD_RPC_EXPORTS
    -DHCI_LINK_CONTROL # Adds support for Link Control packets according to the HCI standard
)

# Force .c files to be compiled with the C++ compiler
if(NOT APPLE)
    set_source_files_properties(
        ${LIB_C_SRC_FILES}
        PROPERTIES
        LANGUAGE CXX
    )
endif()

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

# Object library, from which both shared and static will be built
add_library(${PC_BLE_DRIVER_OBJ_LIB} OBJECT ${LIB_C_SRC_FILES} ${LIB_CPP_SRC_FILES} ${LIB_PLATFORM_SRC_FILES})

# shared libraries need PIC
set_property(TARGET ${PC_BLE_DRIVER_OBJ_LIB} PROPERTY POSITION_INDEPENDENT_CODE 1)

# actual shared and static libraries built from the same object files
add_library(${PC_BLE_DRIVER_SHARED_LIB} SHARED $<TARGET_OBJECTS:${PC_BLE_DRIVER_OBJ_LIB}>)
add_library(${PC_BLE_DRIVER_STATIC_LIB} STATIC $<TARGET_OBJECTS:${PC_BLE_DRIVER_OBJ_LIB}>)

# build test_uart executable
add_executable(test_uart test/test_uart.cpp)

# Additional special linkage libraries
if(WIN32)
elseif(APPLE)
    target_link_libraries(${PC_BLE_DRIVER_STATIC_LIB} PRIVATE "-framework CoreFoundation" "-framework IOKit")
    target_link_libraries(${PC_BLE_DRIVER_SHARED_LIB} PRIVATE "-framework CoreFoundation" "-framework IOKit")
    set_property(TARGET ${PC_BLE_DRIVER_STATIC_LIB} PROPERTY MACOSX_RPATH ON)
    set_property(TARGET ${PC_BLE_DRIVER_SHARED_LIB} PROPERTY MACOSX_RPATH ON)
else()
    # Assume Linux
    target_link_libraries(${PC_BLE_DRIVER_STATIC_LIB} PRIVATE "udev" "pthread")
    target_link_libraries(${PC_BLE_DRIVER_SHARED_LIB} PRIVATE "udev" "pthread")
    target_link_libraries(test_uart PRIVATE "pthread")
endif()


# Specify libraries to link serialization library with
target_link_libraries (${PC_BLE_DRIVER_SHARED_LIB} PRIVATE ${Boost_LIBRARIES})
target_link_libraries (${PC_BLE_DRIVER_STATIC_LIB} PRIVATE ${Boost_LIBRARIES})


target_link_libraries(test_uart PRIVATE ${Boost_LIBRARIES})
