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

# pc-ble-driver include paths
set(PC_BLE_DRIVER_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/../include ${CMAKE_CURRENT_LIST_DIR}/../include/internal/transport)
