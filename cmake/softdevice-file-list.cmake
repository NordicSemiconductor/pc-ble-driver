set(LIB_SDK_V2_C_SRC_FILES
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
    src/sd_api_v2/common/ble_serialization.c
    src/sd_api_v2/common/cond_field_serialization.c
    src/sd_api_v2/common/struct_ser/s130/ble_gap_struct_serialization.c
    src/sd_api_v2/common/struct_ser/s130/ble_gattc_struct_serialization.c
    src/sd_api_v2/common/struct_ser/s130/ble_gatts_struct_serialization.c
    src/sd_api_v2/common/struct_ser/s130/ble_struct_serialization.c
)

set(LIB_SDK_V3_C_SRC_FILES
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


set(LIB_SDK_V5_C_SRC_FILES
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

set(LIB_SDK_V6_C_SRC_FILES
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
