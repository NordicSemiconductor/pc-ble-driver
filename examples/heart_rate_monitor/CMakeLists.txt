cmake_minimum_required(VERSION 3.3)

# Use multithreaded Boost libraries
set(Boost_USE_MULTITHREADED ON)

# Use static boost libraries so the dynamic library 
# can run anywhere
set(Boost_USE_STATIC_LIBS   ON)

# Find the necessary boost components on the system.
# Minimum version required is 1.54.0
find_package ( Boost 1.54.0 REQUIRED COMPONENTS thread system regex date_time chrono )

link_directories (
    ../../build
)

include_directories (
    ../../include/common/sdk_compat
    ../../include/common
    ../../include/common/internal/transport
)

if(MSVC)
    add_definitions(-DPC_BLE_DRIVER_STATIC)
endif()

# Build executable
add_executable(hrm_v2 main.c)
add_executable(hrm_v3 main.c)

target_compile_definitions(hrm_v2 PRIVATE -DNRF_SD_BLE_API=2)
target_compile_definitions(hrm_v3 PRIVATE -DNRF_SD_BLE_API=3)

target_include_directories(hrm_v2 PRIVATE ../../src/sd_api_v2/sdk/components/softdevice/s132/headers)
target_include_directories(hrm_v3 PRIVATE ../../src/sd_api_v3/sdk/components/softdevice/s132/headers)

# Specify libraries to link with
if (MSVC)
    target_link_libraries(hrm_v3 PRIVATE pc_ble_driver_static_sd_api_v3)
    target_link_libraries(hrm_v2 PRIVATE pc_ble_driver_static_sd_api_v2)
else()
    target_link_libraries(hrm_v2 PRIVATE pc_ble_driver_shared_sd_api_v2)
    target_link_libraries(hrm_v3 PRIVATE pc_ble_driver_shared_sd_api_v3)
endif()

target_link_libraries(hrm_v2 PRIVATE ${Boost_LIBRARIES})
target_link_libraries(hrm_v3 PRIVATE ${Boost_LIBRARIES})
