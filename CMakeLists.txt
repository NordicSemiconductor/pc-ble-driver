cmake_minimum_required(VERSION 3.0)

include (cmake/pc-ble-driver.cmake)

set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)

# Find the necessary boost components on system
find_package ( Boost 1.54.0 REQUIRED COMPONENTS thread system regex date_time chrono )

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

file(GLOB_RECURSE LIB_C_SOURCES src/*.c)
file(GLOB_RECURSE LIB_CPP_SOURCES src/*.cpp)

if(WIN32)
    file(GLOB PLATFORM_SOURCE_FILES "src/platform/win/*.c" "src/platform/win/*.cpp")
elseif(APPLE)
    file(GLOB PLATFORM_SOURCE_FILES "src/platform/osx/*.c" "src/platform/osx/*.cpp")
else()
    # Assume Linux
    file(GLOB PLATFORM_SOURCE_FILES "src/platform/linux/*.c" "src/platform/linux/*.cpp")
endif()

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

# Add definitions used by compiler when compiling source files
add_definitions(
    -DSD_RPC_EXPORTS
    -DHCI_LINK_CONTROL # Adds support for Link Control packets according to the HCI standard
)

# Force .c files to be compiled with the C++ compiler
set_source_files_properties(
    ${LIB_C_SOURCES}
    PROPERTIES
    LANGUAGE CXX
)

add_library(pc-ble-driver STATIC ${LIB_C_SOURCES} ${LIB_CPP_SOURCES})

# Specify libraries to link serialization library with
target_link_libraries (pc-ble-driver PRIVATE ${Boost_LIBRARIES})

add_executable(test_uart test/test_uart.cpp)
target_link_libraries(test_uart PRIVATE ${Boost_LIBRARIES})
