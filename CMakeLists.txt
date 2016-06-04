cmake_minimum_required(VERSION 2.8.3)

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
    file(GLOB LIB_PLATFORM_C_SRC_FILES "src/platform/osx/*.c")
    file(GLOB LIB_PLATFORM_CPP_SRC_FILES "src/platform/osx/*.cpp")
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

# Determine if we must add -fPIC since it is required for x64 architectures
if(UNIX AND NOT WIN32 AND NOT APPLE)
    find_program(CMAKE_UNAME uname /bin /usr/bin /usr/local/bin)
    if(CMAKE_UNAME)
        exec_program(uname ARGS -m OUTPUT_VARIABLE CMAKE_SYSTEM_PROCESSOR)
        set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR} CACHE INTERNAL "processor type (i386 and x86_64")
        
        if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
            add_definitions(-fPIC)
        endif()
    endif()
endif()

# Link with pthreads on OS X
if (APPLE)
    add_compile_options(-pthread)
endif(APPLE)

# Add definitions used by compiler when compiling source files
add_definitions(
    -DSD_RPC_EXPORTS
    -DHCI_LINK_CONTROL # Adds support for Link Control packets according to the HCI standard
)

# Force .c files to be compiled with the C++ compiler
set_source_files_properties(
    ${LIB_C_SRC_FILES}
    PROPERTIES
    LANGUAGE CXX
)

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

add_library(pc-ble-driver STATIC ${LIB_C_SRC_FILES} ${LIB_CPP_SRC_FILES} ${LIB_PLATFORM_SRC_FILES})
add_library(pc-ble-driver-obj OBJECT ${LIB_C_SRC_FILES} ${LIB_CPP_SRC_FILES} ${LIB_PLATFORM_SRC_FILES})

# Specify libraries to link serialization library with
target_link_libraries (pc-ble-driver PRIVATE ${Boost_LIBRARIES})

add_executable(test_uart test/test_uart.cpp)
target_link_libraries(test_uart PRIVATE ${Boost_LIBRARIES})
