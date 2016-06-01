# Platform specific
if(MSVC)
    include(${CMAKE_CURRENT_LIST_DIR}/msvc.cmake)
elseif(APPLE)
    include(${CMAKE_CURRENT_LIST_DIR}/apple.cmake)
else()
    include(${CMAKE_CURRENT_LIST_DIR}/gcc.cmake)
endif()

set(PC_BLE_DRIVER_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/../include)
