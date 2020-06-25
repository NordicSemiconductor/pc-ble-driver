find_program(NRFC "nrfc" HINTS "C:/opt/nrf-device-lib/bin" "/opt/nrf-device-lib/bin")
if(NOT NRFC)
    message(FATAL_ERROR "nrfc not found, tests will not be ran.")
endif()

if(NOT BOARD)
    message(FATAL_ERROR "BOARD not specifided")
endif()

if(NOT INSTALL_DIRECTORY)
    message(FATAL_ERROR "INSTALL_DIRECTORY not specified")
endif()

file(TO_NATIVE_PATH "${INSTALL_DIRECTORY}" INSTALL_DIRECTORY)

set(ITERATIONS_ 10)

if(ITERATIONS)
    set(ITERATIONS_ ${ITERATIONS})
endif()

if(NOT SD_TYPE)
    message(FATAL_ERROR "SD_TYPE not specified")
endif()

if(NOT SD_VERSION)
    message(FATAL_ERROR "SD_VERSION not specifided")
endif()

string(REPLACE "." ";" VERSION_LIST ${SD_VERSION})
list(GET VERSION_LIST 0 SD_VERSION_MAJOR)
list(GET VERSION_LIST 1 SD_VERSION_MINOR)
list(GET VERSION_LIST 2 SD_VERSION_PATCH)

if(NOT CONNECTIVITY_VERSION)
    message(FATAL_ERROR "CONNECTIVITY_VERSION not specified")
endif()

set(SD_TRANSPORT_ "1m")

if(SD_TRANSPORT)
    set(SD_TRANSPORT_ "${SD_TRANSPORT}")
endif()

if(NOT TEST_OUTPUT_DIRECTORY)
    message(FATAL_ERROR "TEST_OUTPUT_DIRECTORY is not set")
endif()

# Determine variables based on above input
set(CONNECTIVITY_FIRMWARE "${INSTALL_DIRECTORY}/share/nrf-ble-driver/hex/sd_api_v${SD_VERSION_MAJOR}/connectivity_${CONNECTIVITY_VERSION}_${SD_TRANSPORT_}_with_${SD_TYPE}_${SD_VERSION}.hex")
file(TO_NATIVE_PATH "${CONNECTIVITY_FIRMWARE}" CONNECTIVITY_FIRMWARE)

set(TEST_EXECUTABLE_ "test_sd_api_v${SD_VERSION_MAJOR}")
set(TEST_EXECUTABLE_DIRECTORY "${INSTALL_DIRECTORY}/bin")
file(TO_NATIVE_PATH "${TEST_EXECUTABLE_DIRECTORY}" TEST_EXECUTABLE_DIRECTORY)
find_program(TEST_EXECUTABLE NAMES ${TEST_EXECUTABLE_} PATHS ${TEST_EXECUTABLE_DIRECTORY})

if(NOT TEST_EXECUTABLE)
    message(FATAL_ERROR "Test ${TEST_EXECUTABLE_} not found")
endif()

set(TEST_OUTPUT_FILE "${BOARD}_${SD_TYPE}_${SD_VERSION}_${SD_TRANSPORT_}-result.xml")
set(TEST_OUTPUT_FILE "${TEST_OUTPUT_DIRECTORY}/${TEST_OUTPUT_FILE}")
file(TO_NATIVE_PATH "${TEST_OUTPUT_FILE}" TEST_OUTPUT_FILE)

execute_process(
    COMMAND ${NRFC} --find-by-board ${BOARD} --count 2 --tasks erase,program="${CONNECTIVITY_FIRMWARE}",reset --output csv-wo-header
    RESULT_VARIABLE RESULT
    OUTPUT_VARIABLE DEVICE_LIST_CONTENT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT RESULT EQUAL 0)
    message(FATAL_ERROR "Result of running nrfc is ${RESULT}")
endif()

message(STATUS "Output from nrfc:\"${DEVICE_LIST_CONTENT}\"")

string(REGEX REPLACE ";" "\\\\;" DEVICE_LIST_CONTENT "${DEVICE_LIST_CONTENT}")
string(REGEX REPLACE "\n" ";" DEVICE_LIST_CONTENT "${DEVICE_LIST_CONTENT}")

set(DEVICE_A_COM_PORT)
set(DEVICE_A_SERIAL_NUMBER)
set(DEVICE_B_COM_PORT)
set(DEVICE_B_SERIAL_NUMBER)
set(CURRENT_DEVICE "A")

set(BAUDRATE 1000000)
set(LOG_LEVEL "trace")
set(TEST_REPORTER "junit")


foreach(LINE ${DEVICE_LIST_CONTENT})
    string(REGEX REPLACE "\\;" ";" LINE "${LINE}")

    list(GET LINE 0 SERIALNUMBER)
    list(GET LINE 1 COM_PORTS)

    string(REGEX REPLACE "\\," ";" COM_PORTS "${COM_PORTS}")

    list(LENGTH COM_PORTS COM_PORTS_LENGTH)

    if(COM_PORTS_LENGTH EQUAL 1)
        if(CURRENT_DEVICE STREQUAL "A")
            set(DEVICE_A_COM_PORT "${COM_PORTS}")
            set(DEVICE_A_SERIAL_NUMBER "${SERIALNUMBER}")
            set(CURRENT_DEVICE "B")
        elseif(CURRENT_DEVICE STREQUAL "B")
            set(DEVICE_B_COM_PORT "${COM_PORTS}")
            set(DEVICE_B_SERIAL_NUMBER "${SERIALNUMBER}")
            set(CURRENT_DEVICE "UNKNOWN")
        elseif(CURRENT_DEVICE STREQUAL "UNKNOWN")
            message(FATAL_ERROR "Device is UNKNOWN")
        endif()
    else()
        message(FATAL_ERROR "Do not know how to handle more than one COM port yet, got ${COM_PORTS_LENGTH}")
    endif()
endforeach()

message("DEVICE_A: ${DEVICE_A_SERIAL_NUMBER}/${DEVICE_A_COM_PORT}")
message("DEVICE_B: ${DEVICE_B_SERIAL_NUMBER}/${DEVICE_B_COM_PORT}")

execute_process(
    COMMAND "${TEST_EXECUTABLE}" --port-a ${DEVICE_A_COM_PORT} --port-b ${DEVICE_B_COM_PORT} --iterations ${ITERATIONS_} --baud-rate ${BAUDRATE} --log-level ${LOG_LEVEL} --hardware-info "hex:${CONNECTIVITY_FIRMWARE},pca:${BOARD},segger_sn:${DEVICE_A_SERIAL_NUMBER},${DEVICE_B_SERIAL_NUMBER}" --reporter ${TEST_REPORTER} --order lex --out "${TEST_OUTPUT_FILE}"
    RESULT_VARIABLE RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT RESULT EQUAL 0)
    message(FATAL_ERROR "Result of running ${TEST_EXECUTABLE} is ${RESULT}")
endif()
