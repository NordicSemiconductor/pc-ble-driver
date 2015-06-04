# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

import shutil
import time
import serial
import logging
import sys

ON_POSIX = 'posix' in sys.builtin_module_names


def process_pipe(pipe, queue):
    for line in iter(pipe.readline, b''):
        queue.put({'type': 'output', 'data': line})

    pipe.close()
    queue.put({'type': 'output_terminated'})


def kill_process(target):
    if 'proc' in target:
        target['proc'].kill()

        # Close file descriptors
        target['proc'].stdin.close()
        time.sleep(1)  # Let the application terminate before proceeding


def kill_processes(context):
    targets = context.target_registry.get_all()

    for target in targets:
        kill_process(target)


def add_fflush_to_c_examples(application_path):
    with open(application_path, 'r+') as app_file:
        app_file_list = app_file.readlines()
        app_file.seek(0)
        print_started = False
        for app_file_line in app_file_list:
            app_file.write(app_file_line)
            if 'printf' in app_file_line:
                print_started = True
            if print_started and app_file_line.endswith(');\n'):
                app_file.write('fflush();\n')
                print_started = False


def segger_hack(target):
    # Now we try to circumvent the issue with the Segger firmware and the automatic flow control detection.
    # This hack only opens and closes the serial port.
    retries = 0
    is_open = False

    while retries < 3 and is_open is False:
        try:
            port = serial.Serial(port=target['serial_port'], baudrate=1000000, rtscts=1)
            is_open = True
            port.write(' ')
            port.close()
        except:
            logging.debug("Retrying to open serial port %s", target['serial_port'])
            retries += 1

    if not is_open:
        assert False, "Not able to open serial port {}.".format(target['serial_port'])


def flash_target(target, firmware):
    segger_hack(target)
    logging.debug("Copying: %s to target attached to: %s", firmware, target['drive'])
    shutil.copy(firmware, target['drive'])
    logging.debug("Copying done: now sleeping so that device is reenumerated before it is used.")
    # Let the target reboot and get enumerated again. This time varies from PC to PC.
    time.sleep(30)  # Linux require 30 seconds wait time for reenumerating a device

