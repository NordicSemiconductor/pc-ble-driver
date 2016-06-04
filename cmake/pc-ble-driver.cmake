# include guard
if(PC_BLE_DRIVER_CMAKE_INCLUDED)
    return()
endif(PC_BLE_DRIVER_CMAKE_INCLUDED)
set(PC_BLE_DRIVER_CMAKE_INCLUDED true)
  

# Compiler specific
if(MSVC)
    include(${CMAKE_CURRENT_LIST_DIR}/msvc.cmake)
elseif(APPLE)
    include(${CMAKE_CURRENT_LIST_DIR}/apple.cmake)
else()
    # Linux
    include(${CMAKE_CURRENT_LIST_DIR}/gcc.cmake)
endif()

set(Boost_USE_MULTITHREADED ON)

if(UNIX AND NOT WIN32 AND NOT APPLE)
    # Use dynamic boost libraries on Linux
    set(Boost_USE_STATIC_LIBS   OFF)
else()
    # Use static boost libraries on Windows and OS X
    set(Boost_USE_STATIC_LIBS   ON)
endif()

# Find the necessary boost components on the system.
# Minimum version required is 1.54.0
find_package ( Boost 1.54.0 REQUIRED COMPONENTS thread system regex date_time chrono )

# pc-ble-driver include paths
set(PC_BLE_DRIVER_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/../include ${CMAKE_CURRENT_LIST_DIR}/../include/internal/transport)
