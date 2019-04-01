function(nrf_extract_version_number VERSION_NUMBER MAJOR MINOR PATCH)
    string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" MATCHES ${VERSION_NUMBER})

    if(VERSION)
        set(MAJOR "${CMAKE_MATCH_1}" PARENT_SCOPE)
        set(MINOR "${CMAKE_MATCH_2}" PARENT_SCOPE)
        set(PATCH "${CMAKE_MATCH_3}" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Not able to parse version:${VERSION}")
    endif()
endfunction()

function(nrf_configure_sdk_values SDK_VERSION SDK_DIRECTORY)
     # Configure armgcc related files (if armgcc is available)
    find_program(GCC "arm-none-eabi-gcc")

    if(DEFINED ENV{GCCARMEMB_TOOLCHAIN_PATH} OR GCC)
        # Get gcc version
        if(GCC)
            get_filename_component(GCC_TOOLCHAIN_PATH "${GCC}" DIRECTORY)
            set(GCC_TOOLCHAIN_PATH "${GCC_TOOLCHAIN_PATH}/..")
        else()
            set(GCC_TOOLCHAIN_PATH "$ENV{GCCARMEMB_TOOLCHAIN_PATH}")
            # Environment variables are quoted, remove the quote
            string(REPLACE "\"" "" GCC_TOOLCHAIN_PATH "${GCC_TOOLCHAIN_PATH}")
        endif()

        file(TO_CMAKE_PATH "${GCC_TOOLCHAIN_PATH}" GCC_TOOLCHAIN_PATH)
        set(GCC "${GCC_TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc")

        message(STATUS "Found armgcc in path ${GCC}.")

        execute_process(
            COMMAND "${GCC}" -dumpversion
            RESULT_VARIABLE GCC_RUN_RESULT
            OUTPUT_VARIABLE GCC_VERSION
        )

        # SDKv15 requires bin in addition to GCCARMEMB_TOOLCHAIN_PATH set by gccvar
        if(SDK_VERSION EQUAL 15)
            set(GCC_TOOLCHAIN_PATH "${GCC_TOOLCHAIN_PATH}/bin/")
        endif()

        if(GCC_RUN_RESULT EQUAL 0)
            string(STRIP "${GCC_VERSION}" GCC_VERSION)
            message(STATUS "armgcc returned with version number ${GCC_VERSION}")
        else()
            message(FATAL_ERROR "armgcc returned ${GCC_RUN_RESULT}. Assuming armgcc is non usable (ran ${GCC} -dumpversion).")
            return()
        endif()

        set(ARM_GCC_TOOLCHAIN_VERSION "7.3.1")

        if(NOT GCC_VERSION VERSION_EQUAL "${ARM_GCC_TOOLCHAIN_VERSION}")
            message(FATAL_ERROR "Required armgcc toolchain version not provided. Needs to be ${ARM_GCC_TOOLCHAIN_VERSION}.")
        endif()

        set(TOOLCHAIN_PATH "${SDK_DIRECTORY}/components/toolchain/gcc")
        if(EXISTS "${TOOLCHAIN_PATH}")
            if(WIN32)
                set(MAKEFILE "${TOOLCHAIN_PATH}/Makefile.windows")
                if(EXISTS "${MAKEFILE}")
                    message(STATUS "Altering Makefile using armgcc for Windows")
                else()
                    message(STATUS "Makefile not found: ${MAKEFILE}")
                    set(MAKEFILE)
                endif()

                set(MAKEFILE_COMMON "${TOOLCHAIN_PATH}/Makefile.common")

                if(EXISTS "${MAKEFILE_COMMON}")
                    message(STATUS "Replacing \"rm -rf\" with \"cmake -E remove_directory\" on Windows")

                    file(READ "${MAKEFILE_COMMON}" MAKEFILE_CONTENT)
                    file(WRITE "${MAKEFILE_COMMON}.pristine" "${MAKEFILE_CONTENT}")

                    string(
                        REPLACE "RM := rm -rf" "RM := \"${CMAKE_COMMAND}\" -E remove_directory"
                        MAKEFILE_CONTENT_NEW "${MAKEFILE_CONTENT}"
                    )

                    set(MAKEFILE_CONTENT "${MAKEFILE_CONTENT_NEW}")
                    file(WRITE "${MAKEFILE_COMMON}" "${MAKEFILE_CONTENT}")
                else()
                    message(STATUS "Makefile.common not found: ${MAKEFILE}")
                endif()
            else()
                # Assume POSIX if not WIN32
                set(MAKEFILE "${TOOLCHAIN_PATH}/Makefile.posix")
                if(EXISTS "${MAKEFILE}")
                    message(STATUS "Altering Makefile using armgcc for POSIX .")
                else()
                    message(STATUS "Makefile not found: ${MAKEFILE}")
                    set(MAKEFILE)
                endif()
            endif()

            if(MAKEFILE)
                file(READ "${MAKEFILE}" MAKEFILE_CONTENT)
                file(WRITE "${MAKEFILE}.pristine" "${MAKEFILE_CONTENT}")

                string(REGEX MATCH ";" LIST_SEPARATOR_FOUND "${MAKEFILE_CONTENT}")
                if(LIST_SEPARATOR_FOUND STREQUAL ";")
                    message(FATAL_ERROR "${MAKEFILE} contains ; which is the list split operator, cannot continue.")
                endif()

                # Make the file into a list since the ^$ expressions will not work
                string(REGEX REPLACE "\r?\n" ";" MAKEFILE_LINES "${MAKEFILE_CONTENT}")
                set(MAKEFILE_CONTENT_NEW)

                foreach(LINE ${MAKEFILE_LINES})
                    #message(STATUS "L:${LINE}")
                    string(REGEX REPLACE "^(GNU_INSTALL_ROOT [\\?:]= ).*$" "\\1${GCC_TOOLCHAIN_PATH}" MAKEFILE_LINE "${LINE}")
                    string(REGEX REPLACE "^(GNU_VERSION [\\?:]= ).*$" "\\1${GCC_VERSION}" MAKEFILE_LINE "${MAKEFILE_LINE}")
                    #message(STATUS "M:${MAKEFILE_LINE}")
                    string(APPEND MAKEFILE_CONTENT_NEW "${MAKEFILE_LINE}\n")
                endforeach()

                set(MAKEFILE_CONTENT "${MAKEFILE_CONTENT_NEW}")
                file(WRITE "${MAKEFILE}" "${MAKEFILE_CONTENT}")
                #message(STATUS "Makefile content after change: ${MAKEFILE_CONTENT}")
            endif()
        endif()
    endif()
endfunction()

#[[
    Function that prepares the SDK by:
        - downloading the SDK
        - applying patches
        - modifying the SDK source code so that baud rate and version number is configurable in the build steps
    The function sets the parent scope variable SDK to location of the prepared SDK
]]
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

    if(NOT DEFINED ENV{TMP})
        message(FATAL_ERROR "Temporary directory not set, not able to proceed.")
    endif()

    set(SDKS_DIRECTORY "$ENV{TMP}/pc-ble-driver/sdks")
    file(TO_CMAKE_PATH "${SDKS_DIRECTORY}" SDKS_DIRECTORY)
    set(SDK_DIRECTORY "${SDKS_DIRECTORY}/v${nrf_prepare_sdk_SDK_VERSION}")
    set(SDK_VERSION "${nrf_prepare_sdk_SDK_VERSION}")

    SET(SDK_VERSION "${nrf_prepare_sdk_SDK_VERSION}")

    set(SDK_SETUP_SUCCESS_FILE "${SDK_DIRECTORY}/.sdk-setup-success")

    if(EXISTS "${SDK_SETUP_SUCCESS_FILE}")
        message(STATUS "SDK directory already exists, reusing that.")
    else()
        nrf_download_distfile(
            SDK
            URLS ${nrf_prepare_sdk_URLS}
            DOWNLOAD_DIRECTORY "${SDKS_DIRECTORY}"
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
        else()
            message(STATUS "No patches to apply to public SDK.")
        endif()

        file(WRITE "${SDK_SETUP_SUCCESS_FILE}" "Successfully setup SDK.")
    endif()

    nrf_get_sdk_dir_root(${SDK_DIRECTORY})
    set(SDK_PATH "${SDK_DIRECTORY}" PARENT_SCOPE)
endfunction()

function(nrf_get_sdk_dir_root SDK_DIRECTORY)
    # Detect if it is an old or new directory structure in the SDK
    if(NOT EXISTS "${SDK_DIRECTORY}/components")
        # New structure
        string(REGEX REPLACE "\\.[^.]*$" "" EXTRACTED_SDK_DIR ${nrf_prepare_sdk_FILENAME})
        set(SDK_DIRECTORY "${SDK_DIRECTORY}/${EXTRACTED_SDK_DIR}" PARENT_SCOPE)
    endif()
endfunction()

function(nrf_transport_baud_rate_to_numeric BAUD_RATE BAUD_RATE_NUMERIC)
    if(BAUD_RATE STREQUAL "115k2")
        set(${BAUD_RATE_NUMERIC} "115200" PARENT_SCOPE)
    elseif(BAUD_RATE STREQUAL "1m" OR BAUD_RATE STREQUAL "usb")
        set(${BAUD_RATE_NUMERIC} "1000000" PARENT_SCOPE)
    else()
        set(${BAUD_RATE_NUMERIC} "-1" PARENT_SCOPE)
    endif()
endfunction()

function(nrf_extract_softdevice_info SOFTDEVICE_FILENAME SD_VERSION SOC_FAMILY SD_API_VERSION SD_ID)
    string(REGEX MATCH "(s[0-9]+)_nrf(5[0-9]+)_([0-9].[0-9].[0-9])" MATCHES ${SOFTDEVICE_FILENAME})

    if(MATCHES)
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
                if(_SD_API_VERSION STREQUAL "2.0.0")
                    set(_SD_ID "0x0081")
                elseif(_SD_API_VERSION STREQUAL "2.0.1")
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
                elseif(_SD_API_VERSION STREQUAL "6.1.0")
                    set(_SD_ID "0x00AF")
                elseif(_SD_API_VERSION STREQUAL "6.1.1")
                    set(_SD_ID "0x00B7")
                else()
                    message(FATAL_ERROR "No firmware ID found for SoftDevice ${_SD_VERSION} v${_SD_API_VERSION}")
                endif()
            endif()
        elseif(_SD_VERSION STREQUAL "s140")
            if(_SOC_FAMILY EQUAL 52)
                if(_SD_API_VERSION STREQUAL "6.0.0")
                    set(_SD_ID "0x00A9")
                elseif(_SD_API_VERSION STREQUAL "6.1.0")
                    set(_SD_ID "0x00AE")
                elseif(_SD_API_VERSION STREQUAL "6.1.1")
                    set(_SD_ID "0x00B6")
                else()
                    message(FATAL_ERROR "No firmware ID found for SoftDevice ${_SD_VERSION} v${_SD_API_VERSION}")
                endif()
            endif()
        endif()

        #message(STATUS "SD_VERSION:${_SD_VERSION} SOC_FAMILY:${_SOC_FAMILY} SD_API_VERSION:${_SD_API_VERSION} SD_ID:${_SD_ID}")

        set(${SD_ID} "${_SD_ID}" PARENT_SCOPE)
    else()
        message(STATUS "No SoftDevice information available in filename.")
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

function(nrf_find_alternative_softdevice SOFTDEVICE_HEX_PATH ALTERNATIVE_SOFTDEVICE_HEX)
    set(SD_VERSION)
    set(SOC_FAMILY)
    set(SOC_SD_API_VERSION)
    set(SD_ID)
    nrf_extract_softdevice_info(${FOUND_SOFTDEVICE_HEX} SD_VERSION SOC_FAMILY SD_API_VERSION SD_ID)
    #message(STATUS "SD_VERSION:${SD_VERSION} SOC_FAMILY:${SD_VERSION} SD_API_VERSION:${SD_API_VERSION} SD_ID:${SD_ID}")

    set(MAJOR)
    set(MINOR)
    set(PATCH)
    nrf_extract_version_number("${SD_API_VERSION}" MAJOR MAJOR MINOR PATCH)
    set(SOFTDEVICE_SEARCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/sd_api_v${MAJOR}/${SD_VERSION}_nrf${SOC_FAMILY}_${MAJOR}.*_softdevice.hex")
    file(GLOB ALTERNATIVE_SOFTDEVICE_HEX LIST_DIRECTORIES false "${SOFTDEVICE_SEARCH_PATH}")

    if(ALTERNATIVE_SOFTDEVICE_HEX)
        list(LENGTH ALTERNATIVE_SOFTDEVICE_HEX HEX_COUNT)
        if(HEX_COUNT GREATER 1)
            message(FATAL_ERROR "Found ${HEX_COUNT} alternative hex files, not able to process that.")
        endif()

        set(ALTERNATIVE_SD_VERSION)
        set(ALTERNATIVE_SOC_FAMILY)
        set(ALTERNATIVE_SOC_SD_API_VERSION)
        set(ALTERNATIVE_SD_ID)

        nrf_extract_softdevice_info(${ALTERNATIVE_SOFTDEVICE_HEX} ALTERNATIVE_SD_VERSION ALTERNATIVE_SOC_FAMILY ALTERNATIVE_SD_API_VERSION ALTERNATIVE_SD_ID)

        if(ALTERNATIVE_SD_API_VERSION VERSION_GREATER SD_API_VERSION)
            message(STATUS "Newer SoftDevice version found (${ALTERNATIVE_SD_API_VERSION}) than the SDK provides (${SD_API_VERSION}).")
            set(ALTERNATIVE_SOFTDEVICE_HEX "${ALTERNATIVE_SOFTDEVICE_HEX}" PARENT_SCOPE)
        else()
            message(STATUS "SoftDevice version found is the same version(${ALTERNATIVE_SOFTDEVICE_HEX}) as the one in the SDK(${SD_API_VERSION}). Using the one found in the SDK.")
        endif()
    else()
        #message(STATUS "No newer SoftDevice found.")
    endif()
endfunction()
