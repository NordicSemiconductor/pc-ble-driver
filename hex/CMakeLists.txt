cmake_minimum_required(VERSION 3.11)
project(connectivity)

#[[
    This CMake file uses directory structure conventions in the SDKs 
    supported (v11 and v15) to supply information to targets and create them.
]]

include(../cmake/nrf-download-distfile.cmake)
include(../cmake/nrf-apply-patches.cmake)
include(./cmake/util.cmake)

find_program(KEIL_UV4 "UV4" HINTS "C:/Keil_v5/UV4")
find_program(MERGEHEX "mergehex" HINTS "c:/Program Files (x86)/Nordic Semiconductor/nrf5x/bin")
find_program(NRFUTIL "nrfutil")

if(NOT DEFINED CONNECTIVITY_VERSION)
    message(FATAL_ERROR "CONNECTIVITY_VERSION not provided, terminating compilation.")
endif()

#message(STATUS "KEILUV4: ${KEIL_UV4}")
#message(STATUS "MERGEHEX: ${MERGEHEX}")
#message(STATUS "NRFUTIL: ${NRFUTIL}")

set(PCA_BLACKLIST "pca10036;")

if(NOT KEIL_UV4)
    message(FATAL_ERROR "Keil UV4 not found, not able to compile connectivity firmware.")
endif()

if(NOT MERGEHEX)
    message(FATAL_ERROR "mergehex not found, not able to merge connectivity firmware with SoftDevice.")
endif()

if(NOT NRFUTIL)
    message(FATAL_ERROR "nrfutil not found, not able to create DFU packages of connectivity firmware.")
endif()


if(KEIL_UV4)
    # File counter used for linking the different versions of SDKs to 
    # directories with shorter name
    set(TEMP_FILE_COUNTER 0)
endif()

function(nrf_get_hex_paths SDK_ROOT SDK_VERSION PROJECT_PATH APP_HEX_PATH SOFTDEVICE_HEX_PATH)
    set(PROJECT_NAME)
    set(PCA_TYPE)
    set(SOFTDEVICE_TYPE_VERSION)
    set(TRANSPORT)

    nrf_extract_info_from_path(${PROJECT_PATH} PROJECT_NAME PCA_TYPE SOFTDEVICE_TYPE_VERSION TRANSPORT)
    set(SOFTDEVICE_SEARCH_PATH "${SDK_ROOT}/components/softdevice/${SOFTDEVICE_TYPE_VERSION}/hex/*.hex")
    file(GLOB FOUND_SOFTDEVICE_HEX "${SOFTDEVICE_SEARCH_PATH}")

    if(FOUND_SOFTDEVICE_HEX STREQUAL "")
        message(FATAL_ERROR "No SoftDevice found for project ${PROJECT_PATH}")
    endif()

    set(ALTERNATIVE_SOFTDEVICE_HEX)
    nrf_find_alternative_softdevice(${FOUND_SOFTDEVICE_HEX} ALTERNATIVE_SOFTDEVICE_HEX)

    if(ALTERNATIVE_SOFTDEVICE_HEX)
        set(FOUND_SOFTDEVICE_HEX "${ALTERNATIVE_SOFTDEVICE_HEX}")
    endif()

    set(SOFTDEVICE_HEX_PATH "${FOUND_SOFTDEVICE_HEX}" PARENT_SCOPE)

    if(${PCA_TYPE} STREQUAL "pca10028" OR ${PCA_TYPE} STREQUAL "pca10031")
        if(KEIL_UV4)
            if(SDK_VERSION EQUAL 11)
                set(APP_HEX_PATH "${PROJECT_PATH}/_build/nrf51422_xxac_s130.hex" PARENT_SCOPE)
            else()
                message(STATUS "${PCA_TYPE} is not supported in SDK ${SDK_VERSION}")
            endif()
        else()
            message(STATUS "Not able to determine app hex path for PCA10027 or PCA10031.")
        endif()
    elseif(${PCA_TYPE} STREQUAL "pca10036")
        if(KEIL_UV4)
            if(SDK_VERSION EQUAL 11)
                set(APP_HEX_PATH "${PROJECT_PATH}/_build/nrf52832_xxaa_s132.hex" PARENT_SCOPE)
            else()
                message(STATUS "${PCA_TYPE} is not supported in SDK ${SDK_VERSION}")
            endif()
        else()
            message(STATUS "Not able to determine app hex path for PCA10036.")
        endif()
    elseif(${PCA_TYPE} STREQUAL "pca10040")
        if(KEIL_UV4)
            if(SDK_VERSION EQUAL 11)
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

#[[
TODO:

Change choice of project file (compiler) in prioritized order:
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
    message(STATUS "SDK_ROOT:" ${SDK_ROOT})
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

    foreach(PCA_PATH ${FOUND_PCA_PATHS})
        file(GLOB FOUND_PROJECTS "${PCA_PATH}/*_hci")
        #message(STATUS "Found projects: ${FOUND_PROJECTS}")

        foreach(PROJECT ${FOUND_PROJECTS})
            set(PROJECT_NAME)
            set(PCA_TYPE)
            set(SOFTDEVICE_TYPE_VERSION)
            set(TRANSPORT)
            nrf_extract_info_from_path(${PROJECT} PROJECT_NAME PCA_TYPE SOFTDEVICE_TYPE_VERSION TRANSPORT)
            set(TRANSPORTS)

            if(PROJECT_NAME AND NOT (PCA_TYPE IN_LIST PCA_BLACKLIST))
                message(STATUS "PROJECT_NAME:${PROJECT_NAME} PCA_TYPE:${PCA_TYPE} SD_TV: ${SOFTDEVICE_TYPE_VERSION} TRANSPORT:${TRANSPORT}")

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
                        list(FIND CONNECTIVITY_BUILD_TARGETS "compile_${TARGET_NAME}" FOUND_TARGET)

                        if(FOUND_TARGET GREATER -1)
                            message(STATUS "Target ${TARGET_NAME} already exists.")
                        else()
                            set(COMMAND_NAME "${TARGET_NAME}_cmd")
                            set(KEIL_PROJECT_FULL_PATH "${UV4_PROJECT}")
                            set(KEIL_LOGFILE "${KEIL_PROJECT_DIRECTORY}/${TARGET_NAME}-build.txt")

                            set(SER_CONFIG_PATH)
                            set(MAIN_PATH)
                            nrf_configure_sdk_affected_files(${SDK_VERSION} ${SDK_ROOT} SER_CONFIG_PATH MAIN_PATH)

                            set(SER_CONFIG_IN_PATH "${SER_CONFIG_PATH}.in")
                            if(NOT EXISTS ${SER_CONFIG_IN_PATH})
                                message(FATAL_ERROR "ser_config.h.in not found. The SDK needs to be prepared. Please delete the build folder and try again  (${SDK_ROOT}).")
                            endif()

                            set(MAIN_IN_PATH "${MAIN_PATH}.in")
                            if(NOT EXISTS ${MAIN_IN_PATH})
                                message(FATAL_ERROR "main.c.in not found. The SDK needs to be prepared. Please delete the build folder and try again (${SDK_ROOT}).")
                            endif()

                            # Setup ser_config.h
                            add_custom_command(
                                OUTPUT "${COMMAND_NAME}"
                                COMMAND ${CMAKE_COMMAND} -DSDK_VERSION=${SDK_VERSION} -DSER_CONFIG_IN_PATH=${SER_CONFIG_IN_PATH} -DBAUD_RATE=${TRANSPORT} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_cmd_ser_config.cmake
                            )

                            # Setup main.c
                            add_custom_command(
                                OUTPUT "${COMMAND_NAME}"
                                APPEND COMMAND ${CMAKE_COMMAND} -DMAIN_IN_PATH=${MAIN_IN_PATH} -DCONNECTIVITY_VERSION=${VERSION} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_cmd_main.cmake
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

                            set(MAJOR)
                            set(MINOR)
                            set(PATCH)
                            nrf_extract_version_number("${SD_API_VERSION}" MAJOR MINOR PATCH)
                            set(OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/hex/sd_api_v${MAJOR}")
                            #message(STATUS "Storing files in ${OUTPUT_DIRECTORY}")

                            # Copy merged file to binary directory
                            add_custom_command(
                                OUTPUT "${COMMAND_NAME}"
                                APPEND COMMAND ${CMAKE_COMMAND} -E copy "${HEX_MERGED}" "${OUTPUT_DIRECTORY}"
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
                                        APPEND COMMAND ${CMAKE_COMMAND} -E copy "${DFU_PKG}" "${OUTPUT_DIRECTORY}"
                                    )
                                endif()
                            endif()

                            # Make the last target added to list of targets a dependency to this target
                            # We need to make sure that the targets are compile in sequence
                            # since the source code changes in every target (configuration of .in files)
                            list(LENGTH CONNECTIVITY_BUILD_TARGETS CONNECTIVITY_BUILD_TARGETS_LENGTH)
                            if(CONNECTIVITY_BUILD_TARGETS_LENGTH GREATER_EQUAL 1)
                                list(GET CONNECTIVITY_BUILD_TARGETS -1 PREVIOUS_TARGET)
                            endif()

                            add_custom_target("compile_${TARGET_NAME}" DEPENDS ${COMMAND_NAME} ${PREVIOUS_TARGET})
                            list(APPEND CONNECTIVITY_BUILD_TARGETS "compile_${TARGET_NAME}")
                        endif()
                    endif()
                endforeach()
            endif()
        endforeach()
    endforeach()

    set(CONNECTIVITY_BUILD_TARGETS ${CONNECTIVITY_BUILD_TARGETS} PARENT_SCOPE)
endfunction(nrf_create_connectivity_compile_targets)

set(SDK_PATH )
set(CONNECTIVITY_BUILD_TARGETS "")

nrf_prepare_sdk(
    SDK_VERSION 11
    URLS "https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v11.x.x/nRF5_SDK_11.0.0_89a8197.zip"
    FILENAME "nRF5_SDK_11.0.0_89a8197.zip"
    SHA512 4fe84c4080e2eab664672366a5abfcd605777297835b7b864e0f51f376cd946a28178dfda7e6cefdd04b791058038711d78901ec5d260477a68eb5527c3b2c06
    PATCH_FILES "${CMAKE_CURRENT_LIST_DIR}/sdk110_connectivity.patch"
)

nrf_create_connectivity_compile_targets(
    SDK_VERSION 11
    SDK_ROOT "${SDK_PATH}"
    PROJECT_DIRECTORY "examples/ble_central_and_peripheral/ble_connectivity"
    VERSION "${CONNECTIVITY_VERSION}"
)

nrf_prepare_sdk(
    SDK_VERSION 15
    URLS "https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v15.x.x/nRF5_SDK_15.0.0_a53641a.zip"
    FILENAME "nRF5_SDK_15.0.0_a53641a.zip"
    SHA512 404905a2fffd09edd5a96e09201fdc7238f8102772df3f279048e3dda84220466741223e70de08f0022271e35717ee57882639b728bd8ca798b189baa732c550
    PATCH_FILES "${CMAKE_CURRENT_LIST_DIR}/sdk150_add_sd_v3v5_support.patch"
)

nrf_create_connectivity_compile_targets(
    SDK_VERSION 15
    SDK_ROOT "${SDK_PATH}"
    PROJECT_DIRECTORY "examples/connectivity/ble_connectivity"
    VERSION "${CONNECTIVITY_VERSION}"
)

# Add a connectivity compilation target that depends on the last target added to CONNECTIVITY_BUILD_TARGETS
# This target makes it easier for the developer to know which target to invoke compile
# all versions of connectivity firmware
list(LENGTH CONNECTIVITY_BUILD_TARGETS CONNECTIVITY_BUILD_TARGETS_LENGTH)
if(CONNECTIVITY_BUILD_TARGETS_LENGTH GREATER_EQUAL 1)
    list(GET CONNECTIVITY_BUILD_TARGETS -1 PREVIOUS_TARGET)
    add_custom_target("compile_connectivity" DEPENDS ${PREVIOUS_TARGET})
endif()
