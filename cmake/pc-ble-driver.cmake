# Platform specific
if(MSVC)
    include(${CMAKE_CURRENT_LIST_DIR}/msvc.cmake)
elseif(APPLE)
    include(${CMAKE_CURRENT_LIST_DIR}/apple.cmake)
else()
    include(${CMAKE_CURRENT_LIST_DIR}/gcc.cmake)
endif()

set(PC_BLE_DRIVER_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/../include ${CMAKE_CURRENT_LIST_DIR}/../include/internal/transport)

set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)

# Find the necessary boost components on system
find_package ( Boost 1.54.0 REQUIRED COMPONENTS thread system regex date_time chrono )

