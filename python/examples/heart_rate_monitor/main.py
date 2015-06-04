# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

"""
Example on use of s130_nrf51_ble_driver python binding library.

The example shows how to use the s130 library to configure a heart rate monitor peripheral.
"""

# Add location of binding library to path
import sys
sys.path.append("../..")

import platform
import ctypes

SERIAL_PORT = ""

if platform.system() == "Windows":
    # Load the DLL into memory (instead of copying to current directory)
    ctypes.cdll.LoadLibrary('../../../driver/lib/s130_nrf51_ble_driver')

    SERIAL_PORT = "COM1"

if platform.system() == "Darwin":
    SERIAL_PORT = "/dev/tty.usbmodem00000"

if platform.system() == "Linux":
    # Load the DLL into memory (instead of copying to current directory)
    ctypes.cdll.LoadLibrary('../../../driver/lib/libs130_nrf51_ble_driver.so')

    SERIAL_PORT = "/dev/ttyACM0"

# Import the binding library
import s130_nrf51_ble_driver as ble_driver
import ble_driver_util as util

import traceback
import time

MAX_HRM_LEN = ble_driver.BLE_L2CAP_MTU_DEF - 1 - 2

HEART_RATE_BASE = 65
HEART_RATE_INCREASE = 3
HEART_RATE_LIMIT = 190

connection_handle = ble_driver.BLE_CONN_HANDLE_INVALID

heart_rate_service_handle = 0
heart_rate_measurement_handle = ble_driver.ble_gatts_char_handles_t()

heart_rate = HEART_RATE_BASE
send_notifications = False
advertisement_timed_out = False


def log_message_handler(severity, log_message):
    unused = severity
    try:
        print "Log: {}".format(log_message)
    except Exception, ex:
        print "Exception: {}".format(str(ex))


def ble_evt_handler(ble_event):
    global connection_handle
    global advertisement_timed_out
    try:
        if ble_event is None:
            print "Received empty ble_event"
            return

        evt_id = ble_event.header.evt_id

        if evt_id == ble_driver.BLE_GAP_EVT_CONNECTED:
            connection_handle = ble_event.evt.gap_evt.conn_handle
            print "Connected, connection handle 0x{:04X}".format(connection_handle)

        elif evt_id == ble_driver.BLE_GAP_EVT_DISCONNECTED:
            print "Disconnected"
            start_advertising()
            connection_handle = ble_driver.BLE_CONN_HANDLE_INVALID

        elif evt_id == ble_driver.BLE_GAP_EVT_TIMEOUT:
            print "Advertisement timed out"
            advertisement_timed_out = True

        elif evt_id == ble_driver.BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            ble_driver.sd_ble_gap_sec_params_reply(connection_handle,
                                                   ble_driver.BLE_GAP_SEC_STATUS_SUCCESS,
                                                   None, None)

        elif evt_id == ble_driver.BLE_GATTS_EVT_SYS_ATTR_MISSING:
            ble_driver.sd_ble_gatts_sys_attr_set(connection_handle, None, 0, 0)

        elif evt_id == ble_driver.BLE_GATTS_EVT_WRITE:
            on_write(ble_event.evt.gatts_evt)

        else:
            print "Received event with ID: {}".format(evt_id)

    except Exception, ex:
        print "Exception: {}".format(str(ex))
        print traceback.extract_tb(sys.exc_info()[2])


def on_write(gatts_event):
    global send_notifications
    write_evt = gatts_event.params.write
    length = write_evt.len
    data = write_evt.data

    data_list = util.uint8_array_to_list(data, length)

    if write_evt.context.char_uuid.uuid == 0x2A37:
        write_data = data_list[0]
        send_notifications = write_data == ble_driver.BLE_GATT_HVX_NOTIFICATION


def init_ble_stack():
    ble_enable_params = ble_driver.ble_enable_params_t()
    ble_enable_params.gatts_enable_params.attr_tab_size = ble_driver.BLE_GATTS_ATTR_TAB_SIZE_DEFAULT
    ble_enable_params.gatts_enable_params.service_changed = False

    error_code = ble_driver.sd_ble_enable(ble_enable_params)

    if error_code == ble_driver.NRF_SUCCESS:
        return error_code

    if error_code == ble_driver.NRF_ERROR_INVALID_STATE:
        print "BLE stack already enabled"
        return ble_driver.NRF_SUCCESS

    print "Failed to enable BLE stack"
    return error_code


def set_adv_data():
    device_name = "HRM Example"
    device_name_utf8 = [ord(character) for character in list(device_name)]

    data_type = [ble_driver.BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME]

    payload = list(data_type + device_name_utf8)
    payload_length = len(payload)

    data_list = [payload_length] + payload
    data_length = len(data_list)

    data_array = util.list_to_uint8_array(data_list)
    # To get the correct pointer type, call cast() on the array object.
    data_array_pointer = data_array.cast()

    error_code = ble_driver.sd_ble_gap_adv_data_set(data_array_pointer, data_length, None, 0)

    if error_code != ble_driver.NRF_SUCCESS:
        print "Failed to set advertisement data. Error code: 0x{0:02X}".format(error_code)
        return

    print "Advertising data set"


def start_advertising():
    adv_params = ble_driver.ble_gap_adv_params_t()

    adv_params.type = ble_driver.BLE_GAP_ADV_TYPE_ADV_IND
    adv_params.p_peer_addr = None  # Undirected advertisement.
    adv_params.fp = ble_driver.BLE_GAP_ADV_FP_ANY
    adv_params.p_whitelist = None
    adv_params.interval = util.msec_to_units(40, util.UNIT_0_625_MS)
    adv_params.timeout = 180  # Advertising timeout 180 seconds

    error_code = ble_driver.sd_ble_gap_adv_start(adv_params)

    if error_code != ble_driver.NRF_SUCCESS:
        print "Failed to start advertising. Error code: 0x{0:02X}".format(error_code)
        return

    print "Started advertising"


def encode_heart_rate_measurement(encoded_hrm, heart_rate_value):
    encoded_hrm[0] = 0
    encoded_hrm[1] = heart_rate_value

    return 2


def init_characteristics():
    char_md = ble_driver.ble_gatts_char_md_t()
    cccd_md = ble_driver.ble_gatts_attr_md_t()
    attr_char_value = ble_driver.ble_gatts_attr_t()
    ble_uuid = ble_driver.ble_uuid_t()
    attr_md = ble_driver.ble_gatts_attr_md_t()
    encoded_initial_hrm = ble_driver.uint8_array(MAX_HRM_LEN)

    cccd_md.read_perm.sm = 1
    cccd_md.read_perm.lv = 1
    cccd_md.write_perm.sm = 1
    cccd_md.write_perm.lv = 1
    cccd_md.vloc = ble_driver.BLE_GATTS_VLOC_STACK

    char_md.char_props.notify = 1
    char_md.p_char_user_desc = None
    char_md.p_char_pf = None
    char_md.p_user_desc_md = None
    char_md.p_cccd_md = cccd_md
    char_md.p_sccd_md = None

    ble_uuid.type = ble_driver.BLE_UUID_TYPE_BLE
    ble_uuid.uuid = 0x2A37

    attr_md.read_perm.sm = 1
    attr_md.read_perm.lv = 1
    attr_md.write_perm.sm = 1
    attr_md.write_perm.lv = 1
    attr_md.vloc = ble_driver.BLE_GATTS_VLOC_STACK
    attr_md.rd_auth = 0
    attr_md.wr_auth = 0
    attr_md.vlen = 1

    attr_char_value_init_len = encode_heart_rate_measurement(encoded_initial_hrm, 10)

    attr_char_value.p_uuid = ble_uuid
    attr_char_value.p_attr_md = attr_md
    attr_char_value.init_len = attr_char_value_init_len
    attr_char_value.init_offs = 0
    attr_char_value.max_len = MAX_HRM_LEN
    attr_char_value.p_value = encoded_initial_hrm.cast()

    error_code = ble_driver.sd_ble_gatts_characteristic_add(heart_rate_service_handle,
                                                            char_md,
                                                            attr_char_value,
                                                            heart_rate_measurement_handle)

    if error_code != ble_driver.NRF_SUCCESS:
        print "Failed to initialize characteristics. Error code: 0x{0:02X}".format(error_code)
        return error_code

    print "Characteristics initiated"

    return ble_driver.NRF_SUCCESS


def init_services():
    global heart_rate_service_handle
    ble_uuid = ble_driver.ble_uuid_t()

    ble_uuid.type = ble_driver.BLE_UUID_TYPE_BLE
    ble_uuid.uuid = 0x180D

    heart_rate_service_handle_send = ble_driver.new_uint16()

    error_code = ble_driver.sd_ble_gatts_service_add(ble_driver.BLE_GATTS_SRVC_TYPE_PRIMARY,
                                                     ble_uuid,
                                                     heart_rate_service_handle_send)

    heart_rate_service_handle = ble_driver.uint16_value(heart_rate_service_handle_send)
    ble_driver.delete_uint16(heart_rate_service_handle_send)

    if error_code != ble_driver.NRF_SUCCESS:
        print "Could not initialize service. Error code: 0x{0:02X}".format(error_code)
        return error_code

    print "Services initiated"

    error_code = init_characteristics()

    if error_code != ble_driver.NRF_SUCCESS:
        return error_code

    return ble_driver.NRF_SUCCESS


def generate_heart_rate():
    global heart_rate
    heart_rate += HEART_RATE_INCREASE

    if heart_rate > HEART_RATE_LIMIT:
        heart_rate = HEART_RATE_BASE


def send_heart_rate_measurement():
    hvx_params = ble_driver.ble_gatts_hvx_params_t()
    encoded_hrm = ble_driver.uint8_array(MAX_HRM_LEN)

    generate_heart_rate()

    length = encode_heart_rate_measurement(encoded_hrm, heart_rate)

    hvx_length = ble_driver.new_uint16()
    ble_driver.uint16_assign(hvx_length, length)

    hvx_params.handle = heart_rate_measurement_handle.value_handle
    hvx_params.type = ble_driver.BLE_GATT_HVX_NOTIFICATION
    hvx_params.offset = 0
    hvx_params.p_len = hvx_length
    hvx_params.p_data = encoded_hrm.cast()

    error_code = ble_driver.sd_ble_gatts_hvx(connection_handle, hvx_params)

    actual_hvx_length = ble_driver.uint16_value(hvx_length)
    ble_driver.delete_uint16(hvx_length)

    if error_code == ble_driver.NRF_SUCCESS and length != actual_hvx_length:
        error_code = ble_driver.NRF_ERROR_DATA_SIZE
        print "Failed to send heart rate measurement. Error code: 0x{0:02X}".format(error_code)
        return error_code

    if error_code != ble_driver.NRF_SUCCESS:
        print "Failed to send heart rate measurement. Error code: 0x{0:02X}".format(error_code)
        return error_code

    return ble_driver.NRF_SUCCESS


def main(serial_port):
    print "Serial port used: {}".format(serial_port)
    ble_driver.sd_rpc_serial_port_name_set(serial_port)
    ble_driver.sd_rpc_serial_baud_rate_set(115200)
    ble_driver.sd_rpc_evt_handler_set(ble_evt_handler)
    ble_driver.sd_rpc_log_handler_set(log_message_handler)

    error_code = ble_driver.sd_rpc_open()

    if error_code != ble_driver.NRF_SUCCESS:
        print "Failed to open the nRF51 BLE Driver. Error code: 0x{0:02X}".format(error_code)
        return error_code

    error_code = init_ble_stack()

    if error_code != ble_driver.NRF_SUCCESS:
        return

    error_code = init_services()

    if error_code != ble_driver.NRF_SUCCESS:
        return error_code

    set_adv_data()
    start_advertising()

    while not advertisement_timed_out:
        time.sleep(1)
        if connection_handle != ble_driver.BLE_CONN_HANDLE_INVALID and send_notifications:
            error_code = send_heart_rate_measurement()

            if error_code != ble_driver.NRF_SUCCESS:
                break

    error_code = ble_driver.sd_rpc_close()

    if error_code != ble_driver.NRF_SUCCESS:
        print "Failed to close the nRF51 BLE Driver. Error code: 0x{0:02X}".format(error_code)
        return error_code

    print "Closed"

if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) == 2 else SERIAL_PORT)
    quit()
