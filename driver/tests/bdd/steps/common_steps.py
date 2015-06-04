# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

import os
from Queue import Queue, Empty
import subprocess
from threading import Thread

from behave import given, when, then

from util import *


logger = logging.getLogger(__file__)

logging.basicConfig(format='%(asctime)s %(name)-12s %(levelname)-8s %(message)s',
                    datefmt='%m-%d %H:%M:%S ')


@given(u'target {target_id} is running {softdevice} as {ble_role}')
def step_impl(context, target_id, softdevice, ble_role):
    target = context.target_registry.find_one(target_id=int(target_id))
    assert target is not None
    assert softdevice is not None
    flash_target(target, context.firmware[softdevice])

    target['ble_role'] = ble_role


@given(u'{type_of_application} application \'{application_path}\' is running towards target {target_id}')
def step_impl(context, type_of_application, application_path, target_id):
    target = context.target_registry.find_one(target_id=int(target_id))

    app_info = {
        'path': os.path.join(context.release_dir, *application_path.split('/')),
        'queue': Queue()
    }

    target['app'] = app_info

    exec_name = os.path.basename(app_info['path'])
    exec_dir = os.path.dirname(app_info['path'])

    args = []
    cwd = None
    shell = False
    executable = None

    if type_of_application == 'native':
        args.append(os.path.join(exec_dir, exec_name))
    elif type_of_application == 'Python':
        args.append(sys.executable)
        args.append(os.path.join(exec_dir, exec_name))

        executable = sys.executable  # Set the python interpreter as executable
        cwd = exec_dir
    else:
        raise Exception("Type of application '%s' is unknown.", type_of_application)

    args.append(target['serial_port'])

    process = subprocess.Popen(args=args,
                               bufsize=1,
                               cwd=cwd,
                               executable=executable,
                               stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE,
                               close_fds=ON_POSIX,
                               universal_newlines=True,
                               shell=shell)

    if process.poll() is not None:
        raise Exception("Error starting {} application {}, return code is {}".format(
            type_of_application,
            exec_name,
            process.poll()))

    target['proc'] = process

    queue = app_info['queue']

    stdout_thread = Thread(target=process_pipe, args=(process.stdout, queue))
    stdout_thread.start()
    target['proc.stdout_thread'] = stdout_thread

    stderr_thread = Thread(target=process_pipe, args=(process.stderr, queue))
    stderr_thread.start()
    target['proc.stderr_thread'] = stderr_thread


@when(u'target {target_id} application prints \'{stdout_text}\' within {seconds} second(s)')
def step_impl(context, target_id, stdout_text, seconds):
    target = context.target_registry.find_one(target_id=int(target_id))
    assert target is not None
    logger.debug("stdout_text: %s, wait for %s seconds", stdout_text, seconds)
    queue = target['app']['queue']

    text_found = False

    seconds_remaining = float(seconds)

    while not text_found and seconds_remaining > 0:
        time_start = time.time()
        try:
            # Not entirely correct timeout, but good enough for these kinds of tests
            entry = queue.get(True, float(seconds_remaining))
            logger.debug("entry: %s", entry)

            if entry is not None:
                if entry['type'] == 'output' and entry['data'].find(stdout_text) >= 0:
                    text_found = True
                elif entry['type'] is 'proc_ended':
                    logger.debug("process %s terminated with status %s", target.name, entry['data'])
                    assert False, "Process has terminated with status code %s" % entry['data']
                elif entry['type'] is 'output_terminated':
                    assert False, "Output has terminated before we have received the required text."
                    kill_processes(context)
        except Empty:
            kill_processes(context)
            assert False, "Process did not respond or respond with required data within %s second(s)." % seconds

        seconds_remaining -= time.time() - time_start

    assert text_found


@when(u'user type \'{_input}\' in target {target_id} application')
def step_impl(context, _input, target_id):
    target = context.target_registry.find_one(target_id=int(target_id))
    assert target is not None

    stdin = target['proc'].stdin

    if _input == '<newline>':
        time.sleep(1)
        stdin.write('\n')
    else:
        stdin.write(_input)

    stdin.flush()


@when(u'target {target_id} application is terminated')
def step_impl(context, target_id):
    target = context.target_registry.find_one(target_id=int(target_id))
    assert target is not None
    kill_process(target)


@when(u'target {target_id} is running {firmware}')
def step_impl(context, target_id, firmware):
    target = context.target_registry.find_one(target_id=int(target_id))
    assert target is not None
    flash_target(target, context.firmware[firmware])


@then(u'all the developers are happy :-)')
def step_impl(context):
    assert True  # That is true
