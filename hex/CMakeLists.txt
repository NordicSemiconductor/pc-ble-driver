cmake_minimum_required(VERSION 3.11)
project(connectivity)

#[[
    This CMake file uses directory structure conventions in the SDKs
    supported (v11 and v15) to supply information to targets and create them.
]]

set(GIT_REQUIRED_VERSION "2.19.0")

include(../cmake/nrf-download-distfile.cmake)
include(../cmake/nrf-apply-patches.cmake)
include(./cmake/util.cmake)

find_program(MAKE "make")
find_program(MERGEHEX "mergehex" HINTS "c:/Program Files (x86)/Nordic Semiconductor/nrf5x/bin")
find_program(NRFUTIL "nrfutil")
find_program(GIT "git")

if(NOT DEFINED CONNECTIVITY_VERSION)
    message(FATAL_ERROR "CONNECTIVITY_VERSION not provided, terminating compilation.")
endif()

if(NOT DEFINED ENV{GCCARMEMB_TOOLCHAIN_PATH} AND NOT DEFINED GCCARMEMB_TOOLCHAIN_PATH)
    message(FATAL_ERROR "GNU Tools ARM Embedded environment variable or cmake define GCCARMEMB_TOOLCHAIN_PATH not set. Stopping build.")
else()
    if(DEFINED GCCARMEMB_TOOLCHAIN_PATH)
        set(GCC_TOOLCHAIN_PATH "${GCCARMEMB_TOOLCHAIN_PATH}")
    else()
        set(GCC_TOOLCHAIN_PATH "$ENV{GCCARMEMB_TOOLCHAIN_PATH}")
    endif()
endif()

#message(STATUS "GCC_TOOLCHAIN_PATH: ${GCC_TOOLCHAIN_PATH}")
#message(STATUS "MERGEHEX: ${MERGEHEX}")
#message(STATUS "NRFUTIL: ${NRFUTIL}")

set(PCA_BLACKLIST "pca10036;")

if(NOT MERGEHEX)
    message(FATAL_ERROR "mergehex not found, not able to merge connectivity firmware with SoftDevice.")
endif()

if(NOT NRFUTIL)
    message(FATAL_ERROR "nrfutil not found, not able to create DFU packages of connectivity firmware.")
endif()

if(NOT MAKE)
    message(FATAL_ERROR "make not found, not able to compile connectivity firmware.")
endif()

if(NOT GIT)
    message(FATAL_ERROR "git not found, not able to compile connectivity firmware.")
endif()

execute_process(
    COMMAND ${GIT} "--version"
    OUTPUT_VARIABLE version_info
    RESULT_VARIABLE result
)

string(STRIP "${version_info}" version_info)

string(REGEX MATCH "([0-9]+\.[0-9]+\.[0-9]+)" found_version "${version_info}")

if (found_version VERSION_LESS "2.19.0")
    message(FATAL_ERROR "git version ${found_version} found, version ${GIT_REQUIRED_VERSION} or higher required.")
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
        message(FATAL_ERROR "No SoftDevice found for project ${PROJECT_PATH}. Tried to locate it in ${SOFTDEVICE_SEARCH_PATH}")
    endif()

    set(ALTERNATIVE_SOFTDEVICE_HEX)
    nrf_find_alternative_softdevice(${FOUND_SOFTDEVICE_HEX} ALTERNATIVE_SOFTDEVICE_HEX)

    if(ALTERNATIVE_SOFTDEVICE_HEX)
        set(FOUND_SOFTDEVICE_HEX "${ALTERNATIVE_SOFTDEVICE_HEX}")
    endif()

    set(SOFTDEVICE_HEX_PATH "${FOUND_SOFTDEVICE_HEX}" PARENT_SCOPE)

    if(${PCA_TYPE} STREQUAL "pca10028" OR ${PCA_TYPE} STREQUAL "pca10031")
        if(SDK_VERSION EQUAL 11)
            set(APP_HEX_PATH "${PROJECT_PATH}/_build/nrf51422_xxac_s130.hex" PARENT_SCOPE)
        else()
            message(STATUS "${PCA_TYPE} is not supported in SDK ${SDK_VERSION}")
        endif()
    elseif(${PCA_TYPE} STREQUAL "pca10036")
        if(SDK_VERSION EQUAL 11)
            set(APP_HEX_PATH "${PROJECT_PATH}/_build/nrf52832_xxaa_s132.hex" PARENT_SCOPE)
        else()
            message(STATUS "${PCA_TYPE} is not supported in SDK ${SDK_VERSION}")
        endif()
    elseif(${PCA_TYPE} STREQUAL "pca10040")
        if(SDK_VERSION EQUAL 11)
            set(APP_HEX_PATH "${PROJECT_PATH}/_build/nrf52832_xxaa_s132.hex" PARENT_SCOPE)
        else()
            set(APP_HEX_PATH "${PROJECT_PATH}/_build/nrf52832_xxaa.hex" PARENT_SCOPE)
        endif()
    elseif(${PCA_TYPE} STREQUAL "pca10056" OR ${PCA_TYPE} STREQUAL "pca10059")
        set(APP_HEX_PATH "${PROJECT_PATH}/_build/nrf52840_xxaa.hex" PARENT_SCOPE)
    else()
        message(STATUS "Not able to determine app hex path.")
    endif()
endfunction()

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
                    list(APPEND TRANSPORTS "1m")
                endif()

                foreach(TRANSPORT ${TRANSPORTS})
                    set(ARMGCC_PROJECT_DIRECTORY "${PROJECT}/armgcc")

                    set(SOFTDEVICE_HEX_PATH)
                    set(APP_HEX_PATH)
                    nrf_get_hex_paths("${SDK_ROOT}" "${SDK_VERSION}" "${ARMGCC_PROJECT_DIRECTORY}" APP_HEX_PATH SOFTDEVICE_HEX_PATH)
                    #message(STATUS "APP_HEX_PATH: ${APP_HEX_PATH} SOFTDEVICE_HEX_PATH: ${SOFTDEVICE_HEX_PATH}")

                    set(SD_VERSION)
                    set(SOC_FAMILY)
                    set(SD_API_VERSION)
                    set(SD_ID)
                    nrf_extract_softdevice_info("${SOFTDEVICE_HEX_PATH}" SD_VERSION SOC_FAMILY SD_API_VERSION SD_ID)

                    set(CONNECTIVITY_NAME "connectivity_${VERSION}_${TRANSPORT}")
                    set(TARGET_NAME "${CONNECTIVITY_NAME}_with_${SD_VERSION}_${SD_API_VERSION}")

                    # Check if target name already exists
                    list(FIND CONNECTIVITY_BUILD_TARGETS "compile_${TARGET_NAME}" FOUND_TARGET)

                    if(FOUND_TARGET GREATER -1)
                        message(STATUS "Target ${TARGET_NAME} already exists.")
                    else()
                        set(COMMAND_NAME "${TARGET_NAME}_cmd")
                        message(STATUS "Running make:${MAKE} in ARMGCC_PROJECT_DIRECTORY:${ARMGCC_PROJECT_DIRECTORY}")

                        # Add CFLAGS that specify application version, used by SDFU
                        set(MAJOR)
                        set(MINOR)
                        set(PATCH)
                        nrf_extract_version_number("${CONNECTIVITY_VERSION}" MAJOR MINOR PATCH)
                        set(VERSION_CFLAGS "-DAPP_VERSION_MAJOR=${MAJOR} -DAPP_VERSION_MINOR=${MINOR} -DAPP_VERSION_PATCH=${PATCH}")

                        # Compile project
                        add_custom_command(
                            OUTPUT "${COMMAND_NAME}"
                            COMMAND ${CMAKE_COMMAND} -E env CFLAGS=${VERSION_CFLAGS} ${MAKE}
                            WORKING_DIRECTORY ${ARMGCC_PROJECT_DIRECTORY}
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
                        set(OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/sd_api_v${MAJOR}")

                        message(STATUS "Storing files in ${OUTPUT_DIRECTORY}")

                        if(NOT EXISTS ${OUTPUT_DIRECTORY})
                            message(STATUS "Adding step to create directory ${OUTPUT_DIRECTORY} during build process")
                            add_custom_command(
                                OUTPUT "${COMMAND_NAME}"
                                APPEND COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIRECTORY}
                            )
                        endif()

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

                                # Copy connectivity hex file to binary make_directory
                                add_custom_command(
                                    OUTPUT "${COMMAND_NAME}"
                                    APPEND COMMAND ${CMAKE_COMMAND} -E copy "${APP_HEX_PATH}" "${OUTPUT_DIRECTORY}/${CONNECTIVITY_NAME}_for_${SD_VERSION}_${SD_API_VERSION}.hex"
                                )
                            endif()
                        endif()

                        add_custom_target("compile_${TARGET_NAME}" DEPENDS ${COMMAND_NAME})
                        list(APPEND CONNECTIVITY_BUILD_TARGETS "compile_${TARGET_NAME}")
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
    PATCH_FILES "${CMAKE_CURRENT_LIST_DIR}/nRF5_SDK_11.0.0_connectivity.patch"
)

nrf_create_connectivity_compile_targets(
    SDK_VERSION 11
    SDK_ROOT "${SDK_PATH}"
    PROJECT_DIRECTORY "examples/ble_central_and_peripheral/ble_connectivity"
    VERSION "${CONNECTIVITY_VERSION}"
)

nrf_prepare_sdk(
    SDK_VERSION 15
    URLS "https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v15.x.x/nRF5_SDK_15.3.0_59ac345.zip"
    FILENAME "nRF5_SDK_15.3.0_59ac345.zip"
    SHA512 7cdf1652e828b7c5266c21e1a12e16c90f51213130268e7138950c070c2271d357410a103427e0f67dc56816a8cf6f69df93a6462e2b5e18d6be0a9bb465484f
    PATCH_FILES "${CMAKE_CURRENT_LIST_DIR}/nRF5_SDK_15.3.0_connectivity.patch"
)

nrf_create_connectivity_compile_targets(
    SDK_VERSION 15
    SDK_ROOT "${SDK_PATH}"
    PROJECT_DIRECTORY "examples/connectivity/ble_connectivity"
    VERSION "${CONNECTIVITY_VERSION}"
)

add_custom_target("compile_connectivity" DEPENDS ${CONNECTIVITY_BUILD_TARGETS})
