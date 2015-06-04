/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

%module s130_nrf51_ble_driver

%include "stdint.i"
%include "carrays.i"
%include "cpointer.i"


// Includes used in this transformation
%{
#include "sd_rpc.h"
#include "stdio.h"

#ifdef DEBUG
#include <pthread.h>
#endif // DEBUG

%}

// Requires special handling
%ignore sd_rpc_evt_handler_set;
%ignore sd_rpc_log_handler_set;

// Grab the definitions
%include "nrf_svc.h"
%include "sd_rpc.h"
%include "ble.h"
%include "nrf_error.h"
%include "ble_err.h"
%include "ble_gap.h"
%include "ble_gatt.h"
%include "ble_gatts.h"
%include "ble_gattc.h"
%include "ble_l2cap.h"
%include "ble_ranges.h"
%include "ble_types.h"
%include "ble_hci.h"

%pointer_functions(uint8_t, uint8)
%pointer_functions(uint16_t, uint16)

%array_class(uint8_t, uint8_array);
%array_class(uint16_t, uint16_array);
%array_class(ble_gattc_service_t, ble_gattc_service_array);
%array_class(ble_gattc_include_t, ble_gattc_include_array);
%array_class(ble_gattc_char_t, ble_gattc_char_array);
%array_class(ble_gattc_desc_t, ble_gattc_desc_array);
%array_class(ble_gattc_handle_value_t, ble_gattc_handle_value_array);

// Grab a Python function object as a Python object.
%typemap(in) PyObject *pyfunc {
    if (!PyCallable_Check($input)) {
        PyErr_SetString(PyExc_TypeError, "Need a callable object!");
        return NULL;
    }
    $1 = $input;
}

%rename(sd_rpc_log_handler_set) sd_rpc_log_handler_set_py;
extern void sd_rpc_log_handler_set_py(PyObject *pyfunc);

%rename(sd_rpc_evt_handler_set) sd_rpc_evt_handler_set_py;
extern void sd_rpc_evt_handler_set_py(PyObject *pyfunc);

/* Event callback handling */
%{
static PyObject *my_pyevtcallback = NULL;

static void PythonEvtCallBack(ble_evt_t *ble_event)
{
    PyObject *func;
    PyObject *arglist;
    PyObject *resultobj;
    PyGILState_STATE gstate;
    ble_evt_t* copied_ble_event;

#if DEBUG
    unsigned long int tr;
    tr = (unsigned long int)pthread_self();
    printf("XXXX-XX-XX XX:XX:XX,XXX BIND tid:0x%lX\n", tr);
#endif // DEBUG

    if(my_pyevtcallback == NULL) {
        printf("Callback not set, returning\n");
        return;
    }

    func = my_pyevtcallback;

#if DEBUG
    printf("sizeof(ble_evt_t): %ld\n", sizeof(ble_evt_t));
    printf("ble_event->header.evt_len: %d\n", ble_event->header.evt_len);
    printf("ble_event->header.evt_id: %X\n", ble_event->header.evt_id);
    printf("sizeof(ble_evt_hdr_t): %ld\n", sizeof(ble_evt_hdr_t));
#endif // DEBUG

    // Do a copy of the event so that the Python developer is able to access the event after
    // this callback is complete. The event that is received in this function is allocated
    // on the stack of the function calling this function.
    copied_ble_event = (ble_evt_t*)malloc(sizeof(ble_evt_t));
    memcpy(copied_ble_event, ble_event, sizeof(ble_evt_t));

    // Handling of Python Global Interpretor Lock (GIL)
    gstate = PyGILState_Ensure();

    // Create a Python object that points to the copied event, let the interpreter take care of
    // memory management of the copied event by setting the SWIG_POINTER_OWN flag.
    resultobj = SWIG_NewPointerObj(SWIG_as_voidptr(copied_ble_event), SWIGTYPE_p_ble_evt_t, SWIG_POINTER_OWN);
    arglist = Py_BuildValue("(O)", resultobj);

    PyEval_CallObject(func, arglist);

    Py_XDECREF(resultobj);
    Py_DECREF(arglist);

    PyGILState_Release(gstate);
}
%}

%{
// Set a Python function object as a callback function
void sd_rpc_evt_handler_set_py(PyObject *pyfunc)
{
    Py_XDECREF(my_pyevtcallback);  /* Remove any existing callback object */
    Py_XINCREF(pyfunc);
    my_pyevtcallback = pyfunc;
    sd_rpc_evt_handler_set(PythonEvtCallBack);
}
%}

/* Log callback handling */

%{
static PyObject *my_pylogcallback = NULL;

static void PythonLogCallBack(sd_rpc_log_severity_t severity, const char * log_message)
{
    PyObject *func;
    PyObject *arglist;
    PyObject *result;
    PyObject *message_obj;
    PyObject *severity_obj;
    PyGILState_STATE gstate;

    func = my_pylogcallback;

    // For information regarding GIL and copying of data, please look at
    // function PythonEvtCallBack.

#if DEBUG
    unsigned long int tr;
    tr = (unsigned long int)pthread_self();
    printf("XXXX-XX-XX XX:XX:XX,XXX BIND tid:0x%lX\n", tr);
#endif // DEBUG

    gstate = PyGILState_Ensure();

    severity_obj = SWIG_From_int((int)(severity));

    // SWIG_Python_str_FromChar boils down to PyString_FromString which does a copy of log_message string
    message_obj = SWIG_Python_str_FromChar((const char *)log_message);
    arglist = Py_BuildValue("(OO)", severity_obj, message_obj);

    result = PyEval_CallObject(func, arglist);

    Py_XDECREF(message_obj);
    Py_XDECREF(severity_obj);
    Py_DECREF(arglist);
    Py_XDECREF(result);

    PyGILState_Release(gstate);
}
%}

%{
void sd_rpc_log_handler_set_py(PyObject *pyfunc)
{
    Py_XDECREF(my_pylogcallback); /* Remove any existing callback object */
    Py_XINCREF(pyfunc);
    my_pylogcallback = pyfunc;
    sd_rpc_log_handler_set(PythonLogCallBack);
}
%}
