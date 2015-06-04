# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

project(ble_driver CXX)

# Boost specific settings (link statically (on MinGW) with the multithreaded libraries)
if(MINGW OR APPLE)
    set(Boost_USE_STATIC_LIBS ON)
endif()

if(APPLE)
    message(STATUS "Remember to build boost with the following command line when using brew:")
    message(STATUS "CFLAGS=-fPIC brew install --verbose  --env=std --build-from-source boost")
endif(APPLE)

set(Boost_USE_MULTITHREADED ON)

# Find the necessary boost components on system
find_package ( Boost 1.54.0 REQUIRED COMPONENTS thread system regex date_time chrono )

# Filename for .def for S130
set(s130_target_def_file s130_nrf51_ble_driver.def)

# Add WIN32 specific settings
if(WIN32)
    # Values used when generating version.rc
    string(REPLACE "." "," SERIALIZATION_VERSION_WIN32 ${SERIALIZATION_VERSION})

    enable_language(RC) # Enable compilation of WIN32 resource files
    add_definitions(
        -D_WIN32_WINNT=0x0502
    )

    # Generate dll information for win32 build.
    # Values specified above are used in the version.rc.in file
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/version.rc
        @ONLY
    )

    # Generate batch file for creating .lib file
    get_filename_component(VS_DIR [HKEY_CURRENT_USER\\Software\\Microsoft\\DevDiv\\VCForPython\\9.0;InstallDir] REALPATH CACHE)
    string(REPLACE "/" "\\" VS_DIR ${VS_DIR})
    set(VS_DIR "${VS_DIR}\\VC\\bin")
    set(VS_LIB_PATH "${VS_DIR}\\lib")

    string(REPLACE ".def" ".lib" s130_target_lib_file ${s130_target_def_file})

    configure_file(
        ${CMAKE_SOURCE_DIR}/scripts/msvc_def_to_lib.bat.in
        ${CMAKE_CURRENT_BINARY_DIR}/msvc_def_to_lib.bat
        @ONLY)
endif()

# Include boost as a system include so that the compiler do not nag about issues in boost header files
include_directories (
    ${s130_includes}
    SYSTEM ${Boost_INCLUDE_DIRS}
)

set (s130_src
    ${nrf51_sdk_codecs_s130}/middleware/app_mw_ble.c
    ${nrf51_sdk_codecs_s130}/middleware/app_mw_ble_gap.c
    ${nrf51_sdk_codecs_s130}/middleware/app_mw_ble_gap.c
    ${nrf51_sdk_codecs_s130}/middleware/app_mw_ble_gattc.c
    ${nrf51_sdk_codecs_s130}/middleware/app_mw_ble_l2cap.c
    ${nrf51_sdk_codecs_s130}/middleware/app_mw_ble_gatts.c

    ${nrf51_sdk_codecs_s130}/serializers/app_ble_gap_sec_keys.c
    ${nrf51_sdk_codecs_s130}/serializers/app_ble_user_mem.c

    ${nrf51_sdk_codecs_s130}/serializers/ble_enable.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_event.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_evt_tx_complete.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_evt_user_mem_release.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_evt_user_mem_request.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_opt_get.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_opt_set.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_tx_buffer_count_get.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_user_mem_reply.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_uuid_decode.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_uuid_encode.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_uuid_vs_add.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_version_get.c

    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_address_get.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_address_set.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_adv_data_set.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_adv_start.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_adv_stop.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_appearance_get.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_appearance_set.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_auth_key_reply.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_authenticate.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_conn_param_update.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_conn_sec_get.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_connect.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_connect_cancel.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_device_name_get.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_device_name_set.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_disconnect.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_encrypt.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_evt_adv_report.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_evt_auth_key_request.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_evt_auth_status.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_evt_conn_param_update.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_evt_conn_param_update_request.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_evt_conn_sec_update.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_evt_connected.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_evt_disconnected.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_evt_passkey_display.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_evt_rssi_changed.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_evt_scan_req_report.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_evt_sec_info_request.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_evt_sec_params_request.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_evt_sec_request.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_evt_timeout.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_ppcp_get.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_ppcp_set.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_rssi_get.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_rssi_start.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_rssi_stop.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_scan_start.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_scan_stop.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_sec_info_reply.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_sec_params_reply.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gap_tx_power_set.c

    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_char_value_by_uuid_read.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_char_values_read.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_characteristics_discover.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_descriptors_discover.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_evt_char_disc_rsp.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_evt_char_val_by_uuid_read_rsp.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_evt_char_vals_read_rsp.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_evt_desc_disc_rsp.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_evt_hvx.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_evt_prim_srvc_disc_rsp.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_evt_read_rsp.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_evt_rel_disc_rsp.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_evt_timeout.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_evt_write_rsp.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_hv_confirm.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_primary_services_discover.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_read.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_relationships_discover.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gattc_write.c

    ${nrf51_sdk_codecs_s130}/serializers/ble_gatts_characteristic_add.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gatts_descriptor_add.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gatts_evt_hvc.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gatts_evt_rw_authorize_request.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gatts_evt_sc_confirm.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gatts_evt_sys_attr_missing.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gatts_evt_timeout.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gatts_evt_write.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gatts_hvx.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gatts_include_add.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gatts_rw_authorize_reply.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gatts_service_add.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gatts_service_changed.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gatts_sys_attr_get.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gatts_sys_attr_set.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gatts_value_get.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_gatts_value_set.c

    ${nrf51_sdk_codecs_s130}/serializers/ble_l2cap_cid_register.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_l2cap_cid_unregister.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_l2cap_evt_rx.c
    ${nrf51_sdk_codecs_s130}/serializers/ble_l2cap_tx.c

    ${nrf51_sdk_codecs_s130}/serializers/power_system_off.c

    ${nrf51_sdk_components}/serialization/common/struct_ser/s130/ble_struct_serialization.c
    ${nrf51_sdk_components}/serialization/common/struct_ser/s130/ble_gattc_struct_serialization.c
    ${nrf51_sdk_components}/serialization/common/struct_ser/s130/ble_gatts_struct_serialization.c
    ${nrf51_sdk_components}/serialization/common/struct_ser/s130/ble_gap_struct_serialization.c
)

set (common_src
    src/sd_rpc.c
    src/app_log.c
    src/app_log_utility.c
    src/uart.cpp
    src/uart_boost.cpp
    src/uart_settings.cpp
    src/uart_settings_boost.cpp
    src/timer.cpp
    src/timer_boost.cpp

    src/app_timer_pc.c
    src/app_uart_pc.c
    src/nrf_soc_pc.c
    src/ser_app_hal_pc.c
    src/ser_app_power_system_off_pc.c

    ${nrf51_sdk_components}/libraries/crc16/crc16.c
    ${nrf51_sdk_components}/serialization/application/transport/app_mailbox.c
    ${nrf51_sdk_components}/serialization/application/transport/ser_sd_transport.c
    ${nrf51_sdk_components}/serialization/application/transport/ser_softdevice_handler.c
    ${nrf51_sdk_components}/serialization/common/ble_serialization.c
    ${nrf51_sdk_components}/serialization/common/cond_field_serialization.c
    ${nrf51_sdk_components}/serialization/common/transport/ser_hal_transport.c
    ${nrf51_sdk_components}/serialization/common/transport/ser_phy/ser_phy_hci.c
    ${nrf51_sdk_components}/serialization/common/transport/ser_phy/ser_phy_hci_slip.c
    ${nrf51_sdk_components}/softdevice/common/softdevice_handler/softdevice_handler.c
)

# Add definitions used by compiler when compiling source files
add_definitions(
    -DSD_RPC_EXPORTS
    -DNRF51
    -DBOARD_NRF6310
    -DSVCALL_AS_NORMAL_FUNCTION
    -DBLE_STACK_SUPPORT_REQD
    -DHCI_LINK_CONTROL # Adds support for Link Control packets according to the HCI standard
    -DS130
)

# Compiler options use when compiling
add_compile_options(
    -Wall
    -Wno-effc++
    -Wno-unknown-pragmas
    -Wno-undef
    -Wstrict-overflow=4
    -Wno-long-long
    -Wfloat-equal
    -Wundef
    -Wshadow
    -Wpointer-arith
    #-H # Used for debugging header dependencies. See https://docs.freebsd.org/info/gcc/gcc.info.Preprocessor_Options.html
)

if (APPLE)
    add_compile_options(-pthread)
endif(APPLE)

# Add warning that is only used on GCC (ant not LLVM)
if (NOT APPLE)
    add_compile_options(
        -Wlogical-op
    )
endif(NOT APPLE)

# Force .c files to be compiled with the C++ compiler
set_source_files_properties(
    ${common_src}
    ${s130_src}
    PROPERTIES
    LANGUAGE CXX
)

# S130 target --start--
set(s130_src_list ${common_src} ${s130_src})

if(MINGW)
    set(s130_src_list ${s130_src_list} ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
endif(MINGW)

add_library(s130_nrf51_ble_driver SHARED ${s130_src_list})

# Specify libraries to link serialization library with
target_link_libraries (s130_nrf51_ble_driver PRIVATE ${Boost_LIBRARIES})

if(MINGW)
    set_property(TARGET s130_nrf51_ble_driver APPEND PROPERTY LINK_LIBRARIES ws2_32)
    set_property(TARGET s130_nrf51_ble_driver APPEND PROPERTY LINK_FLAGS "-static-libgcc -static-libstdc++ -static -Wl,--output-def,${s130_target_def_file}")
    set_property(TARGET s130_nrf51_ble_driver PROPERTY PREFIX "")
endif(MINGW)

if(APPLE)
    set_property(TARGET s130_nrf51_ble_driver APPEND PROPERTY LINK_FLAGS "-static -lc -lc++")
endif(APPLE)

if(WIN32)
    add_custom_command(
        TARGET s130_nrf51_ble_driver
        POST_BUILD
        COMMAND "${CMAKE_CURRENT_BINARY_DIR}\\msvc_def_to_lib.bat")
endif(WIN32)

# S130 target --end--

add_subdirectory(tests)
