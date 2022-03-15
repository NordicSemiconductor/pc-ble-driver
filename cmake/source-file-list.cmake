# This file contains the list of source files that is compiled
# Update this list instead of using glob files 
#
# From the cmake documentation (https://cmake.org/cmake/help/latest/command/file.html#glob)
# Note We do not recommend using GLOB to collect a list of source files from your source tree.
# If no CMakeLists.txt file changes when a source is added or removed then the generated build system cannot know when to ask CMake to regenerate.
# The CONFIGURE_DEPENDS flag may not work reliably on all generators, or if a new generator is added in the future that cannot support it, projects using it will be stuck. 
# Even if CONFIGURE_DEPENDS works reliably, there is still a cost to perform the check on every rebuild.

# Common source files
set(LIB_BASE_CPP_SRC_FILES 
    src/common/adapter_internal.cpp
    src/common/app_ble_gap.cpp
    src/common/ble_common.cpp
    src/common/sd_rpc_impl.cpp
)

set(LIB_TRANSPORT_CPP_SRC_FILES 
    src/common/transport/h5.cpp
    src/common/transport/h5_transport.cpp
    src/common/transport/serialization_transport.cpp
    src/common/transport/slip.cpp
    src/common/transport/transport.cpp
    src/common/transport/uart_settings.cpp
    src/common/transport/uart_settings_boost.cpp
    src/common/transport/uart_transport.cpp
)

if(WIN32)
    set(LIB_PLATFORM_C_SRC_FILES
        src/common/platform/win/disphelper.c
    )

    set(LIB_PLATFORM_CPP_SRC_FILES
        src/common/platform/win/enumser.cpp
        src/common/platform/win/jlinkid_reg_lookup.cpp
        src/common/platform/win/serial_port_enum.cpp
    )
elseif(APPLE)
    set(LIB_PLATFORM_CPP_SRC_FILES 
        src/common/platform/macos_osx/serial_port_enum.cpp
    )
else()
    # Assume Linux
    set(LIB_PLATFORM_CPP_SRC_FILES 
        src/common/platform/linux/serial_port_enum.cpp
    )
endif()

# SDK source files, different per API version

# Newer codecs from nRF5 SDK are backwards compatible to SoftDevice API v3
set(LIB_NEWER_SDK_API_COMMON_C_SRC_FILES
    src/sd_api_common/sdk/components/serialization/common/ble_serialization.c
    src/sd_api_common/sdk/components/serialization/common/cond_field_serialization.c
    src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gap_struct_serialization.c
    src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gattc_struct_serialization.c
    src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatts_struct_serialization.c
    src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_gatt_struct_serialization.c
    src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_l2cap_struct_serialization.c
    src/sd_api_common/sdk/components/serialization/common/struct_ser/ble/ble_struct_serialization.c
)

set(LIB_SDK_SD_API_V2_C_SRC_FILES
    src/sd_api_v2/sdk/components/serialization/application/codecs/common/conn_systemreset_app.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/app_ble_user_mem.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_enable.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_event.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_evt_tx_complete.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_evt_user_mem_release.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_evt_user_mem_request.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_address_get.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_address_set.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_adv_data_set.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_adv_start.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_adv_stop.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_appearance_get.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_appearance_set.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_authenticate.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_auth_key_reply.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_connect.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_connect_cancel.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_conn_param_update.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_conn_sec_get.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_device_name_get.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_device_name_set.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_disconnect.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_encrypt.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_evt_adv_report.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_evt_auth_key_request.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_evt_auth_status.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_evt_connected.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_evt_conn_param_update.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_evt_conn_param_update_request.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_evt_conn_sec_update.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_evt_disconnected.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_evt_key_pressed.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_evt_lesc_dhkey_request.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_evt_passkey_display.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_evt_rssi_changed.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_evt_scan_req_report.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_evt_sec_info_request.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_evt_sec_params_request.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_evt_sec_request.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_evt_timeout.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_keypress_notify.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_lesc_dhkey_reply.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_lesc_oob_data_get.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_lesc_oob_data_set.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_ppcp_get.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_ppcp_set.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_rssi_get.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_rssi_start.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_rssi_stop.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_scan_start.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_scan_stop.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_sec_info_reply.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_sec_params_reply.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gap_tx_power_set.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_attr_info_discover.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_characteristics_discover.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_char_values_read.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_char_value_by_uuid_read.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_descriptors_discover.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_evt_attr_info_disc_rsp.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_evt_char_disc_rsp.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_evt_char_vals_read_rsp.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_evt_char_val_by_uuid_read_rsp.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_evt_desc_disc_rsp.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_evt_hvx.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_evt_prim_srvc_disc_rsp.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_evt_read_rsp.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_evt_rel_disc_rsp.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_evt_timeout.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_evt_write_rsp.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_hv_confirm.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_primary_services_discover.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_read.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_relationships_discover.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gattc_write.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_attr_get.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_characteristic_add.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_descriptor_add.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_evt_hvc.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_evt_rw_authorize_request.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_evt_sc_confirm.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_evt_sys_attr_missing.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_evt_timeout.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_evt_write.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_hvx.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_include_add.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_initial_user_handle_get.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_rw_authorize_reply.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_service_add.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_service_changed.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_sys_attr_get.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_sys_attr_set.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_value_get.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_gatts_value_set.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_l2cap_cid_register.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_l2cap_cid_unregister.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_l2cap_evt_rx.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_l2cap_tx.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_opt_get.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_opt_set.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_tx_packet_count_get.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_user_mem_reply.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_uuid_decode.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_uuid_encode.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_uuid_vs_add.c
    src/sd_api_v2/sdk/components/serialization/application/codecs/s130/serializers/ble_version_get.c
    src/sd_api_v2/sdk/components/serialization/common/ble_serialization.c
    src/sd_api_v2/sdk/components/serialization/common/cond_field_serialization.c
    src/sd_api_v2/sdk/components/serialization/common/struct_ser/s130/ble_gap_struct_serialization.c
    src/sd_api_v2/sdk/components/serialization/common/struct_ser/s130/ble_gattc_struct_serialization.c
    src/sd_api_v2/sdk/components/serialization/common/struct_ser/s130/ble_gatts_struct_serialization.c
    src/sd_api_v2/sdk/components/serialization/common/struct_ser/s130/ble_struct_serialization.c
)

set(LIB_SDK_SD_API_V2_CPP_SRC_FILES
    src/sd_api_v2/ble_gap_impl.cpp
    src/sd_api_v2/ble_gattc_impl.cpp
    src/sd_api_v2/ble_gatts_impl.cpp
    src/sd_api_v2/ble_impl.cpp
)

set(LIB_SDK_SD_API_V3_C_SRC_FILES
    src/sd_api_v3/sdk/components/serialization/application/codecs/common/conn_systemreset_app.c
    src/sd_api_v3/sdk/components/serialization/application/codecs/s132/serializers/app_ble_user_mem.c
    src/sd_api_v3/sdk/components/serialization/application/codecs/s132/serializers/ble_app.c
    src/sd_api_v3/sdk/components/serialization/application/codecs/s132/serializers/ble_event.c
    src/sd_api_v3/sdk/components/serialization/application/codecs/s132/serializers/ble_evt_app.c
    src/sd_api_v3/sdk/components/serialization/application/codecs/s132/serializers/ble_gap_app.c
    src/sd_api_v3/sdk/components/serialization/application/codecs/s132/serializers/ble_gap_evt_app.c
    src/sd_api_v3/sdk/components/serialization/application/codecs/s132/serializers/ble_gattc_app.c
    src/sd_api_v3/sdk/components/serialization/application/codecs/s132/serializers/ble_gattc_evt_app.c
    src/sd_api_v3/sdk/components/serialization/application/codecs/s132/serializers/ble_gatts_app.c
    src/sd_api_v3/sdk/components/serialization/application/codecs/s132/serializers/ble_gatts_evt_app.c
    src/sd_api_v3/sdk/components/serialization/application/codecs/s132/serializers/ble_l2cap_app.c
    src/sd_api_v3/sdk/components/serialization/application/codecs/s132/serializers/ble_l2cap_evt_app.c
)

set(LIB_SDK_SD_API_V3_CPP_SRC_FILES
    src/sd_api_v3/ble_gap_impl.cpp
    src/sd_api_v3/ble_gattc_impl.cpp
    src/sd_api_v3/ble_gatts_impl.cpp
    src/sd_api_v3/ble_impl.cpp
)

set(LIB_SDK_SD_API_V5_C_SRC_FILES
    src/sd_api_v5/sdk/components/serialization/application/codecs/ble/serializers/app_ble_user_mem.c
    src/sd_api_v5/sdk/components/serialization/application/codecs/ble/serializers/ble_app.c
    src/sd_api_v5/sdk/components/serialization/application/codecs/ble/serializers/ble_event.c
    src/sd_api_v5/sdk/components/serialization/application/codecs/ble/serializers/ble_evt_app.c
    src/sd_api_v5/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_app.c
    src/sd_api_v5/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_evt_app.c
    src/sd_api_v5/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_app.c
    src/sd_api_v5/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_evt_app.c
    src/sd_api_v5/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_app.c
    src/sd_api_v5/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_evt_app.c
    src/sd_api_v5/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_app.c
    src/sd_api_v5/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_evt_app.c
    src/sd_api_v5/sdk/components/serialization/application/codecs/common/conn_systemreset_app.c
)

set(LIB_SDK_SD_API_V5_CPP_SRC_FILES
    src/sd_api_v5/ble_gap_impl.cpp
    src/sd_api_v5/ble_gattc_impl.cpp
    src/sd_api_v5/ble_gatts_impl.cpp
    src/sd_api_v5/ble_impl.cpp
)

set(LIB_SDK_SD_API_V6_C_SRC_FILES
    src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/app_ble_user_mem.c
    src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_app.c
    src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_event.c
    src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_evt_app.c
    src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_app.c
    src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gap_evt_app.c
    src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_app.c
    src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gattc_evt_app.c
    src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_app.c
    src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_gatts_evt_app.c
    src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_app.c
    src/sd_api_v6/sdk/components/serialization/application/codecs/ble/serializers/ble_l2cap_evt_app.c
    src/sd_api_v6/sdk/components/serialization/application/codecs/common/conn_systemreset_app.c
)

set(LIB_SDK_SD_API_V6_CPP_SRC_FILES
    src/sd_api_v6/ble_gap_impl.cpp
    src/sd_api_v6/ble_gattc_impl.cpp
    src/sd_api_v6/ble_gatts_impl.cpp
    src/sd_api_v6/ble_impl.cpp
)
