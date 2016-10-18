cmake_minimum_required(VERSION 3.3)

# Use multithreaded Boost libraries
set(Boost_USE_MULTITHREADED ON)

# Use static boost libraries so the dynamic library 
# can run anywhere
set(Boost_USE_STATIC_LIBS   ON)

# Find the necessary boost components on the system.
# Minimum version required is 1.54.0
find_package ( Boost 1.54.0 REQUIRED COMPONENTS thread system regex date_time chrono )

include_directories (
    ../../include/common/sdk_compat
    ../../include/common
    ../../include/common/internal/transport
    ../../src/sd_api_v2/sdk/components/softdevice/s132/headers
)

link_directories (
    ../../build
)

# build test_uart executable
add_executable(hrm main.c)

# Specify libraries to link with
target_link_libraries(hrm PRIVATE pc_ble_driver_shared_sd_api_v2)
target_link_libraries(hrm PRIVATE ${Boost_LIBRARIES})
