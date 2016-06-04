# include guard
if(PC_BLE_DRIVER_CMAKE_INCLUDED)
    return()
endif(PC_BLE_DRIVER_CMAKE_INCLUDED)
set(PC_BLE_DRIVER_CMAKE_INCLUDED true)
  
# Find the necessary boost components on the system.
# Minimum version required is 1.54.0
find_package ( Boost 1.54.0 REQUIRED COMPONENTS thread system regex date_time chrono )
set(Boost_USE_MULTITHREADED ON)

# Compiler specific
if(MSVC)
    include(${CMAKE_CURRENT_LIST_DIR}/msvc.cmake)
    set(Boost_USE_STATIC_LIBS   ON)
elseif(APPLE)
    include(${CMAKE_CURRENT_LIST_DIR}/apple.cmake)
    set(Boost_USE_STATIC_LIBS   ON)
else()
    # Linux
    include(${CMAKE_CURRENT_LIST_DIR}/gcc.cmake)
    # Use dynamic boost libraries on Linux
    set(Boost_USE_STATIC_LIBS   OFF)
endif()

# pc-ble-driver include paths
set(PC_BLE_DRIVER_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/../include ${CMAKE_CURRENT_LIST_DIR}/../include/internal/transport)
