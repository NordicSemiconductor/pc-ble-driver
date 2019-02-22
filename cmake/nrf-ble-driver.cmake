# include guard
if(NRF_BLE_DRIVER_CMAKE_INCLUDED)
    return()
endif(NRF_BLE_DRIVER_CMAKE_INCLUDED)
set(NRF_BLE_DRIVER_CMAKE_INCLUDED true)
  
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

# Set C++ standard to use
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler specific
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    include(${CMAKE_CURRENT_LIST_DIR}/clang.cmake)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    include(${CMAKE_CURRENT_LIST_DIR}/gcc.cmake)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    include(${CMAKE_CURRENT_LIST_DIR}/msvc.cmake)
endif()

if(APPLE)
    include(${CMAKE_CURRENT_LIST_DIR}/apple.cmake)
endif()

# Add or remove SD API versions here
if(NOT SD_API_VER_NUMS)
    set(SD_API_VER_NUMS 2 3 5 6)
endif()

list(LENGTH SD_API_VER_NUMS SD_API_VER_COUNT)

set(SD_API_VER_PREFIX "SD_API_V")
set(SD_API_VERS )

foreach(SD_API_VER_NUM ${SD_API_VER_NUMS})
    # Set internal variable names based on the list of version numbers
    set(_SD_API_VER "${SD_API_VER_PREFIX}${SD_API_VER_NUM}")
    string(TOLOWER ${_SD_API_VER} _SD_API_VER_L)
    # Append it to the list
    list(APPEND SD_API_VERS ${_SD_API_VER})
    # Set project and variable names
    set(NRF_BLE_DRIVER_${_SD_API_VER}_PROJECT_NAME "nrf_ble_driver_${_SD_API_VER_L}")
    set(NRF_BLE_DRIVER_${_SD_API_VER}_OBJ_LIB "nrf_ble_driver_obj_${_SD_API_VER_L}")
    set(NRF_BLE_DRIVER_${_SD_API_VER}_STATIC_LIB "nrf_ble_driver_${_SD_API_VER_L}_static")
    set(NRF_BLE_DRIVER_${_SD_API_VER}_SHARED_LIB "nrf_ble_driver_${_SD_API_VER_L}_shared")
endforeach(SD_API_VER_NUM)

set(SD_API_VER_COMPILER_DEF "NRF_SD_BLE_API_VERSION")

# pc-ble-driver root folder
set(NRF_BLE_DRIVER_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

# pc-ble-driver hex folder
set(NRF_BLE_DRIVER_HEX_DIR ${NRF_BLE_DRIVER_ROOT_DIR}/hex)

# pc-ble-driver include path
set(NRF_BLE_DRIVER_INCLUDE_DIR ${NRF_BLE_DRIVER_ROOT_DIR}/include)

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
    string(CONCAT str ${str} "* C Compiler: " "${CMAKE_C_COMPILER_ID} (${ARCH_BITS}-bit)" "\n") 
    string(CONCAT str ${str} "* C++ Compiler: " "${CMAKE_CXX_COMPILER_ID} (${ARCH_BITS}-bit)" "\n") 
    string(CONCAT str ${str} "* CMake version: " ${CMAKE_VERSION} "\n") 
    string(CONCAT str ${str} "* SD API Versions:") 
    foreach(SD_API_VER ${SD_API_VERS})
        string(CONCAT str ${str} " <${SD_API_VER}>") 
    endforeach()
    string(CONCAT str ${str} "\n") 
     
    set(${dst} ${str} PARENT_SCOPE)
endfunction()
