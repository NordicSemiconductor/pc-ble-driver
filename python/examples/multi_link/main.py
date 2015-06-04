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

The example shows how to use the s130 library to discover and connect to multiple peripherals.
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
from time import sleep

connection_handle = None
ble_scan_params = None
ble_conn_params = None


max_peer_count = 8
peer_count = 0

target_list = ["Example"]


def log_message_handler(severity, log_message):
    unused = severity
    try:
        print "Log: {}".format(log_message)
    except Exception, ex:
        print "Exception: {}".format(str(ex))


def ble_evt_handler(ble_event):
    global peer_count

    try:
        if ble_event is None:
            print "Received empty ble_event"
            return

        evt_id = ble_event.header.evt_id

        if evt_id == ble_driver.BLE_GAP_EVT_CONNECTED:
            on_connected(ble_event.evt.gap_evt)

        elif evt_id == ble_driver.BLE_GAP_EVT_DISCONNECTED:
            on_disconnected(ble_event.evt.gap_evt)

        elif evt_id == ble_driver.BLE_GAP_EVT_ADV_REPORT:
            on_adv_report(ble_event.evt.gap_evt)

        else:
            print "Received event with ID: 0x{0:02X}".format(evt_id)

    except Exception, ex:
        print "Exception: {}".format(str(ex))
        print traceback.extract_tb(sys.exc_info()[2])


def on_connected(gap_event):
    global peer_count
    print "Connected to device"
    peer_count += 1
    print "{} devices connected".format(peer_count)
    start_scan()


def on_disconnected(gap_event):
    global peer_count
    print "Disconnected, reason: {}".format(gap_event.params.disconnected.reason)
    peer_count -= 1
    print "{} devices connected".format(peer_count)


def parse_adv_report(data_type, data, data_length):
    index = 0
    parsed_data = []

    while index < data_length:
        field_length = data[index]
        field_type = data[index+1]

        if field_type == data_type:
            parsed_data = data[index+2:index+field_length+1]
            return parsed_data
        index += field_length+1

    return None


def on_adv_report(gap_event):
    adv_report = gap_event.params.adv_report

    length = adv_report.dlen
    data = adv_report.data
    data_list = util.uint8_array_to_list(data, length)

    ret_val = parse_adv_report(ble_driver.BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, data_list, length)
    if ret_val == ble_driver.NRF_ERROR_NOT_FOUND:
        ret_val = parse_adv_report(ble_driver.BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME, data_list, length)

    if ret_val is None:
        print "Received advertisment report (no device name)"
        return

    device_name = "".join(chr(element) for element in ret_val)

    print "Received advertisment report, device_name: {}".format(device_name)

    if device_name not in target_list:
        return

    if peer_count >= max_peer_count:
        print "Max peer count reached"
        return

    peer_addr = adv_report.peer_addr
    ble_address = ble_driver.ble_gap_addr_t()
    ble_address.addr_type = peer_addr.addr_type
    ble_address.addr = peer_addr.addr

    error_code = ble_driver.sd_ble_gap_connect(ble_address, ble_scan_params, ble_conn_params)

    if error_code != ble_driver.NRF_SUCCESS:
        print "Failed to connect. Error code: 0x{0:02X}".format(error_code)
        return


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


def init_connection_params():
    global ble_conn_params
    ble_conn_params = ble_driver.ble_gap_conn_params_t()
    ble_conn_params.min_conn_interval = util.msec_to_units(30, util.UNIT_1_25_MS)
    ble_conn_params.max_conn_interval = util.msec_to_units(60, util.UNIT_1_25_MS)
    ble_conn_params.conn_sup_timeout = util.msec_to_units(4000, util.UNIT_10_MS)
    ble_conn_params.slave_latency = 0


def init_scan_params():
    global ble_scan_params
    ble_scan_params = ble_driver.ble_gap_scan_params_t()
    ble_scan_params.interval = util.msec_to_units(200, util.UNIT_0_625_MS)
    ble_scan_params.window = util.msec_to_units(150, util.UNIT_10_MS)


def start_scan():
    error_code = ble_driver.sd_ble_gap_scan_start(ble_scan_params)

    print "Started scan with return code: 0x{0:02X}".format(error_code)


def intro_message():
    print "Example for demonstration of connection to multiple devices."
    print "The example will connect to maximum {} simultaneous devices.".format(max_peer_count)
    print "The device used is the device on {}.".format(SERIAL_PORT)
    print "This example will connect to devices with the following names:"

    for target in target_list:
        print "  * {}".format(target)


def main(serial_port):
    intro_message()
    print "Serial port used: {}".format(serial_port)
    ble_driver.sd_rpc_serial_port_name_set(serial_port)
    ble_driver.sd_rpc_serial_baud_rate_set(115200)
    ble_driver.sd_rpc_evt_handler_set(ble_evt_handler)
    ble_driver.sd_rpc_log_handler_set(log_message_handler)
    ble_driver.sd_rpc_open()
    init_ble_stack()
    init_connection_params()
    init_scan_params()
    start_scan()

    while True:
        sleep(1)

    ble_driver.sd_rpc_close()


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) == 2 else SERIAL_PORT)
    quit()
