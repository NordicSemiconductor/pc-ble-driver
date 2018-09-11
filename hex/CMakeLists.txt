cmake_minimum_required(VERSION 3.11)
project(connectivity)

#[[
    This CMake file uses directory structure conventions in the SDKs 
    supported (v11 and v15) to supply information to target and create them
]]

include(../cmake/nrf-download-distfile.cmake)
include(../cmake/nrf-apply-patches.cmake)

find_program(KEIL_UV4 "UV4" HINTS "C:/Keil_v5/UV4")
find_program(MERGEHEX "mergehex" HINTS "c:/Program Files (x86)/Nordic Semiconductor/nrf5x/bin")
find_program(NRFUTIL "nrfutil")

#message(STATUS "KEILUV4: ${KEIL_UV4}")
#message(STATUS "MERGEHEX: ${MERGEHEX}")
#message(STATUS "NRFUTIL: ${NRFUTIL}")

if(NOT KEIL_UV4)
    message(FATAL_ERROR "Keil UV4 not found, not able to compile connectivity firmware.")
endif()

if(NOT MERGEHEX)
    message(FATAL_ERROR "mergehex not found, not able to merge connectivity firmware with SoftDevice.")
endif()

if(NOT NRFUTIL)
    message(FATAL_ERROR "nrfutil not found, not able to create DFU packages of connectivity firmware.")
endif()

function(nrf_configure_sdk_values SDK_VERSION SDK_DIRECTORY)
    set(SER_CONFIG_PATH "${SDK_DIRECTORY}/components/serialization/common/ser_config.h")

    if(NOT EXISTS ${SER_CONFIG_PATH})
        message("Not able to find ser_config.h in path ${SER_CONFIG_PATH}")
        return()
    endif()

    if(SDK_VERSION STREQUAL "11")
        set(MAIN_PATH "${SDK_DIRECTORY}/examples/ble_central_and_peripheral/ble_connectivity/main.c")
    elseif(SDK_VERSION STREQUAL "15")
        set(MAIN_PATH "${SDK_DIRECTORY}/examples/connectivity/ble_connectivity/main.c")
    else()
        message(STATUS "Not able to prepare SDK with configuration values because SDK v${SDK_VERSION} is unknown to me.")
        return()
    endif()

    if(NOT EXISTS ${MAIN_PATH})
        message("Not able to find main.c in path ${MAIN_PATH}")
        return()
    endif()

    file(READ ${SER_CONFIG_PATH} SER_CONFIG)
    file(READ ${MAIN_PATH} MAIN)

    if(SDK_VERSION STREQUAL "11")
        string(REGEX REPLACE "#define (SER_PHY_UART_BAUDRATE)(.+)UART_BAUDRATE_BAUDRATE_Baud1M\n" "#cmakedefine \\1\\2@SER_PHY_UART_BAUDRATE@\n" SER_CONFIG_IN ${SER_CONFIG})
    elseif(SDK_VERSION STREQUAL "15")
        string(REGEX REPLACE "#define (SER_PHY_UART_BAUDRATE_VAL)(.+)1000000\n" "#cmakedefine \\1\\2@SER_PHY_UART_BAUDRATE_VAL@\n" SER_CONFIG_IN ${SER_CONFIG})
    endif()

    # Version number is defined the same way in supported SDKS
    # string(REGEX REPLACE "(\\.version_major += +)0xf1(,\n)" "\\1@VERSION_MAJOR@\\2" MAIN_IN ${MAIN})
    # string(REGEX REPLACE "(\\.version_minor += +)0xf2(,\n)" "\\1@VERSION_MINOR@\\2" MAIN_IN ${MAIN_IN})
    # string(REGEX REPLACE "(\\.version_patch += +)0xf3(,\n)" "\\1@VERSION_PATCH@\\2" MAIN_IN ${MAIN_IN})
    # message(STATUS "MAIN/SDKv${SDK_VERSION}\n\n${MAIN_IN}\n\n\n")
    # file(WRITE "${MAIN_PATH}.in" "${MAIN_IN}")

    file(WRITE "${SER_CONFIG_PATH}.in" "${SER_CONFIG_IN}")
endfunction()

function(nrf_get_sdk_dir_root SDK_DIRECTORY)
    # Detect if it is an old or new directory structure in the SDK
    if(NOT EXISTS "${SDK_DIRECTORY}/components")
        # New structure
        string(REGEX REPLACE "\\.[^.]*$" "" EXTRACTED_SDK_DIR ${nrf_prepare_sdk_FILENAME})
        set(SDK_DIRECTORY "${SDK_DIRECTORY}/${EXTRACTED_SDK_DIR}" PARENT_SCOPE)
    endif()
endfunction(nrf_get_sdk_dir_root)

function(nrf_transport_baud_rate_to_numeric BAUD_RATE BAUD_RATE_NUMERIC)
    if(BAUD_RATE STREQUAL "115k2")
        set(${BAUD_RATE_NUMERIC} "115200" PARENT_SCOPE)
    elseif(BAUD_RATE STREQUAL "1m" OR BAUD_RATE STREQUAL "usb")
        set(${BAUD_RATE_NUMERIC} "1000000" PARENT_SCOPE)
    else()
        set(${BAUD_RATE_NUMERIC} "-1" PARENT_SCOPE)
    endif()
endfunction()

function(nrf_prepare_sdk)
    set(oneValueArgs SHA512 FILENAME SDK_VERSION)
    set(multipleValuesArgs URLS PATCH_FILES)
    cmake_parse_arguments(nrf_prepare_sdk "" "${oneValueArgs}" "${multipleValuesArgs}" ${ARGN})

    if(NOT DEFINED nrf_prepare_sdk_SDK_VERSION)
        message(FATAL_ERROR "nrf_prepare_sdk requires a SDK_VERSION argument.")
    endif()
    if(NOT DEFINED nrf_prepare_sdk_URLS)
        message(FATAL_ERROR "nrf_prepare_sdk requires a URLS argument.")
    endif()
    if(NOT DEFINED nrf_prepare_sdk_FILENAME)
        message(FATAL_ERROR "nrf_prepare_sdk requires a FILENAME argument.")
    endif()
    if(NOT DEFINED nrf_prepare_sdk_SHA512)
        message(FATAL_ERROR "nrf_prepare_sdk requires a SHA512 argument.")
    endif()

    #message(STATUS "URLS: ${nrf_prepare_sdk_URLS}")
    #message(STATUS "FILENAME: ${nrf_prepare_sdk_FILENAME}")
    #message(STATUS "SHA512: ${nrf_prepare_sdk_SHA512}")

    set(SDK_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/sdks/v${nrf_prepare_sdk_SDK_VERSION}")
    SET(SDK_VERSION "${nrf_prepare_sdk_SDK_VERSION}")

    set(SDK_SETUP_SUCCESS_FILE "${SDK_DIRECTORY}/.sdk-setup-success")

    if(EXISTS "${SDK_SETUP_SUCCESS_FILE}")
        message(STATUS "SDK directory already exists, reusing that.")
    else()
        nrf_download_distfile(
            SDK
            URLS ${nrf_prepare_sdk_URLS}
            DOWNLOAD_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/sdks"
            FILENAME "${nrf_prepare_sdk_FILENAME}"
            SHA512 ${nrf_prepare_sdk_SHA512}
        )

        execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${SDK_DIRECTORY}")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xf ${SDK}
            WORKING_DIRECTORY "${SDK_DIRECTORY}"
        )

        nrf_get_sdk_dir_root(${SDK_DIRECTORY})

        message(STATUS "Patching files in directory: ${SDK_DIRECTORY}")
        if(DEFINED nrf_prepare_sdk_PATCH_FILES)
            nrf_apply_patches(
                SOURCE_PATH "${SDK_DIRECTORY}"
                PATCHES ${nrf_prepare_sdk_PATCH_FILES}
            )

            nrf_configure_sdk_values(
                "${SDK_VERSION}"
                "${SDK_DIRECTORY}"
            )
        endif()

        file(WRITE "${SDK_SETUP_SUCCESS_FILE}" "Successfully setup SDK.")
    endif()

    nrf_get_sdk_dir_root(${SDK_DIRECTORY})
    set(SDK_PATH "${SDK_DIRECTORY}" PARENT_SCOPE)
endfunction()

if(KEIL_UV4)
    set(TEMP_FILE_COUNTER 0)
endif()

function(nrf_extract_softdevice_info SOFTDEVICE_FILENAME SD_VERSION SOC_FAMILY SD_API_VERSION SD_ID)
    #message("SOFTDEVICE_FILENAME: ${SOFTDEVICE_FILENAME}")
    string(REGEX MATCH "(s[0-9]+)_nrf(5[0-9]+)_([0-9].[0-9].[0-9])" MATCHES ${SOFTDEVICE_FILENAME})

    if(MATCHES)
        #set(${SD_VERSION} "${CMAKE_MATCH_1}" PARENT_SCOPE)
        #set(${SOC_FAMILY} "${CMAKE_MATCH_2}" PARENT_SCOPE)
        #set(${SD_API_VERSION} "${CMAKE_MATCH_3}" PARENT_SCOPE)

        set(_SD_VERSION "${CMAKE_MATCH_1}")
        set(_SOC_FAMILY "${CMAKE_MATCH_2}")
        set(_SD_API_VERSION "${CMAKE_MATCH_3}")

        set(${SD_VERSION} "${_SD_VERSION}" PARENT_SCOPE)
        set(${SOC_FAMILY} "${_SOC_FAMILY}" PARENT_SCOPE)
        set(${SD_API_VERSION} "${_SD_API_VERSION}" PARENT_SCOPE)

        if(_SD_VERSION STREQUAL "s130")
            if(_SOC_FAMILY EQUAL 51)
                if(_SD_API_VERSION STREQUAL "1.0.0")
                    set(_SD_ID "0x0067")
                elseif(_SD_API_VERSION STREQUAL "2.0.0")
                    set(_SD_ID "0x0080")
                elseif(_SD_API_VERSION STREQUAL "2.0.1")
                    set(_SD_ID "0x0087")
                endif()
            endif()
        elseif(_SD_VERSION STREQUAL "s132")
            if(_SOC_FAMILY EQUAL 52)
                if(_SD_API_VERSION STREQUAL "2.0.1")
                    set(_SD_ID "0x0088")
                elseif(_SD_API_VERSION STREQUAL "3.0.0")
                    set(_SD_ID "0x008C")
                elseif(_SD_API_VERSION STREQUAL "3.1.0")
                    set(_SD_ID "0x0091")
                elseif(_SD_API_VERSION STREQUAL "4.0.0")
                    set(_SD_ID "0x0095")
                elseif(_SD_API_VERSION STREQUAL "4.0.2")
                    set(_SD_ID "0x0098")
                elseif(_SD_API_VERSION STREQUAL "4.0.3")
                    set(_SD_ID "0x0099")
                elseif(_SD_API_VERSION STREQUAL "4.0.4")
                    set(_SD_ID "0x009E")
                elseif(_SD_API_VERSION STREQUAL "4.0.5")
                    set(_SD_ID "0x009F")
                elseif(_SD_API_VERSION STREQUAL "5.0.0")
                    set(_SD_ID "0x009D")
                elseif(_SD_API_VERSION STREQUAL "5.1.0")
                    set(_SD_ID "0x00A5")
                elseif(_SD_API_VERSION STREQUAL "6.0.0")
                    set(_SD_ID "0x00A8")
                endif()
            endif()
        elseif(_SD_VERSION STREQUAL "s140")
            if(_SOC_FAMILY EQUAL 52)
                if(_SD_API_VERSION STREQUAL "6.0.0")
                    set(_SD_ID "0x00A9")
                endif()
            endif()
        endif()

        #message(STATUS "SD_VERSION:${_SD_VERSION} SOC_FAMILY:${_SOC_FAMILY} SD_API_VERSION:${_SD_API_VERSION} SD_ID:${_SD_ID}")

        set(${SD_ID} "${_SD_ID}" PARENT_SCOPE)
    else()
        message(STATUS "No SoftDevice information available in filename.")
    endif()

endfunction(nrf_extract_softdevice_info)

function(nrf_get_hex_paths SDK_ROOT SDK_VERSION PROJECT_PATH APP_HEX_PATH SOFTDEVICE_HEX_PATH)
    set(PROJECT_NAME)
    set(PCA_TYPE)
    set(SOFTDEVICE_TYPE_VERSION)
    set(TRANSPORT)

    nrf_extract_info_from_path(${PROJECT_PATH} PROJECT_NAME PCA_TYPE SOFTDEVICE_TYPE_VERSION TRANSPORT)
    set(SOFTDEVICE_SEARCH_PATH "${SDK_ROOT}/components/softdevice/${SOFTDEVICE_TYPE_VERSION}/hex/*.hex")
    file(GLOB FOUND_SOFTDEVICE_HEX "${SOFTDEVICE_SEARCH_PATH}")
    set(SOFTDEVICE_HEX_PATH "${FOUND_SOFTDEVICE_HEX}" PARENT_SCOPE)

    if(${PCA_TYPE} STREQUAL "pca10028" OR ${PCA_TYPE} STREQUAL "pca10031")
        if(KEIL_UV4)
            if("${SDK_VERSION}" STREQUAL "11")
                set(APP_HEX_PATH "${PROJECT_PATH}/_build/nrf51422_xxac_s130.hex" PARENT_SCOPE)
            else()
                message(STATUS "${PCA_TYPE} is not supported in SDK ${SDK_VERSION}")
            endif()
        else()
            message(STATUS "Not able to determine app hex path for PCA10027 or PCA10031.")
        endif()
    elseif(${PCA_TYPE} STREQUAL "pca10036")
        if(KEIL_UV4)
            if(${SDK_VERSION} STREQUAL "11")
                set(APP_HEX_PATH "${PROJECT_PATH}/_build/nrf52832_xxaa_s132.hex" PARENT_SCOPE)
            else()
                message(STATUS "${PCA_TYPE} is not supported in SDK ${SDK_VERSION}")
            endif()
        else()
            message(STATUS "Not able to determine app hex path for PCA10036.")
        endif()
    elseif(${PCA_TYPE} STREQUAL "pca10040")
        if(KEIL_UV4)
            if(${SDK_VERSION} STREQUAL "11")
                set(APP_HEX_PATH "${PROJECT_PATH}/_build/nrf52832_xxaa_s132.hex" PARENT_SCOPE)
            else()
            set(APP_HEX_PATH "${PROJECT_PATH}/_build/nrf52832_xxaa.hex" PARENT_SCOPE)
            endif()
        else()
            message(STATUS "Not able to determine app hex path for PCA10040.")
        endif()
    elseif(${PCA_TYPE} STREQUAL "pca10056" OR ${PCA_TYPE} STREQUAL "pca10059")
        if(KEIL_UV4)
            set(APP_HEX_PATH "${PROJECT_PATH}/_build/nrf52840_xxaa.hex" PARENT_SCOPE)
        else()
            message(STATUS "Not able to determine app hex path for PCA10059 or PCA10056.")
        endif()
    else()
        message(STATUS "Not able to determine app hex path.")
    endif()
endfunction()

function(nrf_extract_info_from_path PROJECT_PATH PROJECT_NAME PCA_TYPE SOFTDEVICE_TYPE_VERSION TRANSPORT)
    string(REGEX MATCH ".*(pca100[1-9][0-9])" MATCHES ${PROJECT_PATH})

    if(MATCHES)
        set(${PCA_TYPE} "${CMAKE_MATCH_1}" PARENT_SCOPE)
    else()
        message(STATUS "No PCA information available in string '${PCA_STRING}'")
    endif()

    string(REGEX MATCH ".*(s1[0-9][0-9])(v[0-9])?" MATCHES ${PROJECT_PATH})

    if(MATCHES)
        set(${SOFTDEVICE_TYPE_VERSION} "${CMAKE_MATCH_1}${CMAKE_MATCH_2}" PARENT_SCOPE)
    else()
        message(STATUS "No SoftDevice type or version available in string '${PROJECT_PATH}'")
    endif()

    get_filename_component(PROJECT_NAME "${PROJECT_PATH}" NAME)
    set(PROJECT_NAME ${PROJECT_NAME} PARENT_SCOPE)

    string(REGEX MATCH ".*_usb_.*" MATCHES ${PROJECT_NAME})
    if(MATCHES)
        set(${TRANSPORT} "usb" PARENT_SCOPE)
    else()
        set(${TRANSPORT} "uart" PARENT_SCOPE)
    endif()
endfunction()

#[[
TODO:

Change choice of project file (compiler) in prioritized order:
- Segger Embedded Studio (.emProject)
- armgcc (Makefile)
- Keil (.uvprojx and if it is on Windows platform)
]]

function(nrf_create_connectivity_compile_targets)
    set(oneValueArgs PROJECT_DIRECTORY SDK_ROOT SDK_VERSION VERSION)
    cmake_parse_arguments(nrf_create_connectivity_compile_targets "" "${oneValueArgs}" "${multipleValuesArgs}" ${ARGN})

    if(NOT DEFINED nrf_create_connectivity_compile_targets_PROJECT_DIRECTORY)
        message(FATAL_ERROR "nrf_create_connectivity_compile_targets requires a PROJECT_DIRECTORY argument.")
    endif()
    if(NOT DEFINED nrf_create_connectivity_compile_targets_VERSION)
        message(FATAL_ERROR "nrf_create_connectivity_compile_targets requires a VERSION argument.")
    endif()
    if(NOT DEFINED nrf_create_connectivity_compile_targets_SDK_ROOT)
        message(FATAL_ERROR "nrf_create_connectivity_compile_targets requires a SDK_ROOT argument.")
    endif()
    if(NOT DEFINED nrf_create_connectivity_compile_targets_SDK_VERSION)
        message(FATAL_ERROR "nrf_create_connectivity_compile_targets requires a SDK_VERSION argument.")
    endif()

    set(SDK_ROOT "${nrf_create_connectivity_compile_targets_SDK_ROOT}")
    #message(STATUS "SDK_ROOT:" ${SDK_ROOT})
    set(SDK_VERSION ${nrf_create_connectivity_compile_targets_SDK_VERSION})
    set(VERSION ${nrf_create_connectivity_compile_targets_VERSION})

    if(KEIL_UV4)
        set(KEIL_HACK "$ENV{TMP}/${TEMP_FILE_COUNTER}")
        math(EXPR TEMP_FILE_COUNTER "${TEMP_FILE_COUNTER} + 1")
        set(TEMP_FILE_COUNTER "${TEMP_FILE_COUNTER}" PARENT_SCOPE)

        message(STATUS "Keil UV4 has issues with long paths, creating a link to the SDK from ${KEIL_HACK}")

        file(TO_NATIVE_PATH ${KEIL_HACK} KEIL_HACK_N)
        file(TO_NATIVE_PATH ${SDK_ROOT} SDK_ROOT_N)

        if(EXISTS "${KEIL_HACK}")
            message(STATUS "Removing old link")
            execute_process(COMMAND "cmd.exe" "/C" "rmdir" "${KEIL_HACK_N}")
        endif()
        execute_process(COMMAND "cmd.exe" "/C" "mklink" "/D" "${KEIL_HACK_N}" "${SDK_ROOT_N}")
        set(SDK_ROOT ${KEIL_HACK})
    endif()

    set(PROJECT_FULL_PATH "${SDK_ROOT}/${nrf_create_connectivity_compile_targets_PROJECT_DIRECTORY}")
    #message(STATUS "Searching for projects within project ${PROJECT_FULL_PATH}")
    file(GLOB FOUND_PCA_PATHS "${PROJECT_FULL_PATH}/pca100[1-9][0-9]")

    set(BUILD_TARGETS)

    foreach(PCA_PATH ${FOUND_PCA_PATHS})
        file(GLOB FOUND_PROJECTS "${PCA_PATH}/*_hci")
        message(STATUS "Found projects: ${FOUND_PROJECTS}")

        foreach(PROJECT ${FOUND_PROJECTS})
            set(PROJECT_NAME)
            set(PCA_TYPE)
            set(SOFTDEVICE_TYPE_VERSION)
            set(TRANSPORT)
            nrf_extract_info_from_path(${PROJECT} PROJECT_NAME PCA_TYPE SOFTDEVICE_TYPE_VERSION TRANSPORT)
            set(TRANSPORTS)

            if(PROJECT_NAME)
                #message(STATUS "PROJECT_NAME:${PROJECT_NAME} PCA_TYPE:${PCA_TYPE} SD_TV: ${SOFTDEVICE_TYPE_VERSION} TRANSPORT:${TRANSPORT}")

                if(TRANSPORT STREQUAL "usb")
                    list(APPEND TRANSPORTS "usb")
                endif()

                if(TRANSPORT STREQUAL "uart")
                    list(APPEND TRANSPORTS "115k2")
                    list(APPEND TRANSPORTS "1m")
                endif()

                foreach(TRANSPORT ${TRANSPORTS})
                    if(KEIL_UV4)
                        set(KEIL_PROJECT_DIRECTORY "${PROJECT}/arm5_no_packs")
                        file(GLOB FOUND_UV4_PROJECTS "${KEIL_PROJECT_DIRECTORY}/*.uvprojx")

                        if(FOUND_UV4_PROJECTS)
                            list(LENGTH "${FOUND_UV4_PROJECTS}" UV4_PROJECT_COUNT)

                            if(${UV4_PROJECT_COUNT} LESS_EQUAL 1)
                                if(${UV4_PROJECT_COUNT} EQUAL 1)
                                    list(${FOUND_UV4_PROJECTS} 0 UV4_PROJECT)
                                else()
                                    set(UV4_PROJECT "${FOUND_UV4_PROJECTS}")
                                endif()
                            else()
                                message(STATUS "Found ${UV4_PROJECT_COUNT} projects. Not supported.")
                                return()
                            endif()
                        endif()

                        set(SOFTDEVICE_HEX_PATH)
                        set(APP_HEX_PATH)
                        nrf_get_hex_paths("${SDK_ROOT}" "${SDK_VERSION}" "${KEIL_PROJECT_DIRECTORY}" APP_HEX_PATH SOFTDEVICE_HEX_PATH)
                        #message(STATUS "APP_HEX_PATH: ${APP_HEX_PATH} SOFTDEVICE_HEX_PATH: ${SOFTDEVICE_HEX_PATH}")

                        set(SD_VERSION)
                        set(SOC_FAMILY)
                        set(SD_API_VERSION)
                        set(SD_ID)
                        nrf_extract_softdevice_info("${SOFTDEVICE_HEX_PATH}" SD_VERSION SOC_FAMILY SD_API_VERSION SD_ID)

                        set(TARGET_NAME "connectivity_${VERSION}_${TRANSPORT}_with_${SD_VERSION}_${SD_API_VERSION}")

                        # Check if target name already exists
                        list(FIND BUILD_TARGETS ${TARGET_NAME} FOUND_TARGET)

                        if(FOUND_TARGET GREATER -1)
                            message(STATUS "Target ${TARGET_NAME} already exists.")
                        else()
                            set(COMMAND_NAME "${TARGET_NAME}_cmd")
                            set(KEIL_PROJECT_FULL_PATH "${UV4_PROJECT}")
                            set(KEIL_LOGFILE "${KEIL_PROJECT_DIRECTORY}/${TARGET_NAME}-build.txt")
                            
                            set(SER_CONFIG_IN_PATH "${SDK_ROOT}/components/serialization/common/ser_config.h.in")
                            if(NOT EXISTS ${SER_CONFIG_IN_PATH})
                                message(FATAL_ERROR "ser_config.h.in not found. The SDK needs to be prepared. Please delete the build folder and try again.")
                            endif()

                            # Setup ser_config.h
                            add_custom_command(
                                OUTPUT "${COMMAND_NAME}"
                                COMMAND ${CMAKE_COMMAND} -DSDK_VERSION=${SDK_VERSION} -DSER_CONFIG_IN_PATH=${SER_CONFIG_IN_PATH} -DBAUD_RATE=${TRANSPORT} -P ${CMAKE_CURRENT_SOURCE_DIR}/ser_config.cmake
                            )

                            # Compile project
                            add_custom_command(
                                OUTPUT "${COMMAND_NAME}"
                                APPEND COMMAND ${KEIL_UV4} -b ${KEIL_PROJECT_FULL_PATH} -j0 -o ${KEIL_LOGFILE}
                                WORKING_DIRECTORY ${KEIL_PROJECT_DIRECTORY}
                            )

                            set(HEX_MERGED "${TARGET_NAME}.hex")

                            # Merge SoftDevice and application
                            add_custom_command(
                                OUTPUT "${COMMAND_NAME}"
                                APPEND COMMAND ${MERGEHEX} -m "${APP_HEX_PATH}" "${SOFTDEVICE_HEX_PATH}" -o "${HEX_MERGED}"
                            )

                            # Copy merged file to binary directory
                            add_custom_command(
                                OUTPUT "${COMMAND_NAME}"
                                APPEND COMMAND ${CMAKE_COMMAND} -E copy "${HEX_MERGED}" "${CMAKE_BINARY_DIR}"
                            )

                            # Create a DFU package if it is a nRF52 device with USB support
                            if(SOC_FAMILY EQUAL 52 AND TRANSPORT STREQUAL "usb")
                                if(PCA_TYPE STREQUAL "pca10056" OR PCA_TYPE STREQUAL "pca10059")
                                    set(DFU_PKG "${TARGET_NAME}_dfu_pkg.zip")

                                    add_custom_command(
                                        OUTPUT "${COMMAND_NAME}"
                                        APPEND COMMAND ${NRFUTIL} pkg generate "--application" "${APP_HEX_PATH}" "--hw-version" 52 "--sd-req" 0 "--sd-id" ${SD_ID} "--debug-mode" "--softdevice" "${SOFTDEVICE_HEX_PATH}" "${DFU_PKG}"
                                    )

                                    add_custom_command(
                                        OUTPUT "${COMMAND_NAME}"
                                        APPEND COMMAND ${CMAKE_COMMAND} -E copy "${DFU_PKG}" "${CMAKE_BINARY_DIR}"
                                    )
                                endif()
                            endif()

                            add_custom_target("${TARGET_NAME}" DEPENDS ${COMMAND_NAME})
                            list(APPEND BUILD_TARGETS ${TARGET_NAME})
                        endif()
                    endif()
                endforeach()
            endif()
        endforeach()
    endforeach()

    add_custom_target("sdk_v${SDK_VERSION}" ALL DEPENDS ${BUILD_TARGETS})
endfunction(nrf_create_connectivity_compile_targets)

set(SDK_PATH )
set(CONNECTIVITY_FIRMWARE_VERSION 1.2.3)
set(CONNECTIVITY_FIRMWARE_PREFIX "connectivity")

nrf_prepare_sdk(
    SDK_VERSION 11
    URLS "https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v11.x.x/nRF5_SDK_11.0.0_89a8197.zip"
    FILENAME "nRF5_SDK_11.0.0_89a8197.zip"
    SHA512 4fe84c4080e2eab664672366a5abfcd605777297835b7b864e0f51f376cd946a28178dfda7e6cefdd04b791058038711d78901ec5d260477a68eb5527c3b2c06
    PATCH_FILES "${CMAKE_CURRENT_LIST_DIR}/sd_api_v2/sdk110_connectivity.patch"
)

nrf_create_connectivity_compile_targets(
    SDK_VERSION "11"
    SDK_ROOT ${SDK_PATH}
    PROJECT_DIRECTORY "examples/ble_central_and_peripheral/ble_connectivity"
    VERSION ${CONNECTIVITY_FIRMWARE_VERSION}
)

nrf_prepare_sdk(
    SDK_VERSION 15
    URLS "https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v15.x.x/nRF5_SDK_15.0.0_a53641a.zip"
    FILENAME "nRF5_SDK_15.0.0_a53641a.zip"
    SHA512 404905a2fffd09edd5a96e09201fdc7238f8102772df3f279048e3dda84220466741223e70de08f0022271e35717ee57882639b728bd8ca798b189baa732c550
    PATCH_FILES "${CMAKE_CURRENT_LIST_DIR}/sdk150_add_sd_v3v5_support.patch"
)

nrf_create_connectivity_compile_targets(
    SDK_VERSION "15"
    SDK_ROOT ${SDK_PATH}
    PROJECT_DIRECTORY "examples/connectivity/ble_connectivity"
    VERSION ${CONNECTIVITY_FIRMWARE_VERSION}
)
