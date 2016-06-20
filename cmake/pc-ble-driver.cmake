# include guard
if(PC_BLE_DRIVER_CMAKE_INCLUDED)
    return()
endif(PC_BLE_DRIVER_CMAKE_INCLUDED)
set(PC_BLE_DRIVER_CMAKE_INCLUDED true)
  
math(EXPR COMPILER_ARCH_BITS "8*${CMAKE_SIZEOF_VOID_P}")
# Default to compiler architecture
set(ARCH_BITS ${COMPILER_ARCH_BITS})

SET(ARCH not_set CACHE STRING "Architecture (x86_32 or x86_64)")
string(TOLOWER "${ARCH}" ARCH)

if(${ARCH} STREQUAL not_set)
    message(STATUS "Architecture not set, using native ${ARCH_BITS}-bit toolchain.")
else()
    if(MSVC)
        message(FATAL_ERROR "ARCH not available with MSVC. Use  -G \"Visual Studio XX <Win64>\" instead.")
    elseif(APPLE)
        message(FATAL_ERROR "ARCH not available on macOS / OS X. Universal 32 and 64-bit binaries will be built.")
    endif()
    if(${ARCH} STREQUAL x86_32)
        set(ARCH_BITS 32)
    elseif(${ARCH} STREQUAL x86_64)
        set(ARCH_BITS 64)
    else()
        message(FATAL_ERROR "Invalid architecture: ARCH=${ARCH}.")
    endif()
    message(STATUS "Building ${ARCH_BITS}-bit targets with ${COMPILER_ARCH_BITS}-bit toolchain.")
endif()

if(NOT MSVC)
    message(STATUS "Building with build type: ${CMAKE_BUILD_TYPE}.")
endif()

# Compiler specific
if(MSVC)
    include(${CMAKE_CURRENT_LIST_DIR}/msvc.cmake)
elseif(APPLE)
    include(${CMAKE_CURRENT_LIST_DIR}/apple.cmake)
else()
    # Linux
    include(${CMAKE_CURRENT_LIST_DIR}/gcc.cmake)
endif()

# Use multithreaded Boost libraries
set(Boost_USE_MULTITHREADED ON)

# Use static boost libraries so the dynamic library 
# can run anywhere
set(Boost_USE_STATIC_LIBS   ON)

# Find the necessary boost components on the system.
# Minimum version required is 1.54.0
find_package ( Boost 1.54.0 REQUIRED COMPONENTS thread system regex date_time chrono )

set(PC_BLE_DRIVER_PROJECT_NAME "pc_ble_driver")
set(PC_BLE_DRIVER_OBJ_LIB "pc_ble_driver_obj")
set(PC_BLE_DRIVER_STATIC_LIB "pc_ble_driver_static")
set(PC_BLE_DRIVER_SHARED_LIB "pc_ble_driver_shared")

# pc-ble-driver root folder
set(PC_BLE_DRIVER_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

# pc-ble-driver hex folder
set(PC_BLE_DRIVER_HEX_DIR ${PC_BLE_DRIVER_ROOT_DIR}/hex)

# pc-ble-driver include paths
set(PC_BLE_DRIVER_INCLUDE_DIR ${PC_BLE_DRIVER_ROOT_DIR}/include ${PC_BLE_DRIVER_ROOT_DIR}/../include/internal/transport)

find_package(Git REQUIRED)

function(git_repo_metadata dir commit branch remotes)

    # Get the latest abbreviated commit hash of the working branch
    execute_process(
      COMMAND ${GIT_EXECUTABLE} log -1 --format=%H
      WORKING_DIRECTORY ${dir}
      OUTPUT_VARIABLE GIT_COMMIT_HASH
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )    

    set(${commit} ${GIT_COMMIT_HASH} PARENT_SCOPE)

    # Get the current working branch
    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
      WORKING_DIRECTORY ${dir}
      OUTPUT_VARIABLE GIT_BRANCH
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(${branch} ${GIT_BRANCH} PARENT_SCOPE)

    # Get the Git Remote
    execute_process(
      COMMAND ${GIT_EXECUTABLE} remote -v
      WORKING_DIRECTORY ${dir}
      OUTPUT_VARIABLE GIT_REMOTES
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(${remotes} ${GIT_REMOTES} PARENT_SCOPE)

endfunction()

function(build_metadata dir dst)

    cmake_host_system_information(RESULT BUILD_MD_HOSTNAME QUERY HOSTNAME)
    string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S (YY-MM-DD HH:MM:SS, UTC)" UTC)

    set(str "\n")
    string(CONCAT str ${str} "* System: " ${CMAKE_SYSTEM} "\n") 
    string(CONCAT str ${str} "* Hostname: " ${BUILD_MD_HOSTNAME} "\n") 
    string(CONCAT str ${str} "* Timestamp: " ${BUILD_TIMESTAMP} "\n") 
    string(CONCAT str ${str} "* Generator: " ${CMAKE_GENERATOR} "\n") 
    string(CONCAT str ${str} "* Build type: " ${CMAKE_BUILD_TYPE} "\n") 
    string(CONCAT str ${str} "* C Compiler: " ${CMAKE_C_COMPILER_ID} "\n") 
    string(CONCAT str ${str} "* C++ Compiler: " ${CMAKE_CXX_COMPILER_ID} "\n") 
    string(CONCAT str ${str} "* CMake version: " ${CMAKE_VERSION} "\n") 
    string(CONCAT str ${str} "* Boost version: " ${Boost_MAJOR_VERSION} "." ${Boost_MINOR_VERSION} "." ${Boost_SUBMINOR_VERSION} "\n") 
    string(CONCAT str ${str} "* Boost libs: " ${Boost_LIBRARY_DIRS} "\n") 
     
    set(${dst} ${str} PARENT_SCOPE)
 

endfunction()
