function(nrf_configure_sdk_affected_files SDK_VERSION SDK_DIRECTORY SER_CONFIG_PATH MAIN_PATH)
    if(SDK_VERSION EQUAL 11)
        set(MAIN_PATH "${SDK_DIRECTORY}/examples/ble_central_and_peripheral/ble_connectivity/main.c" PARENT_SCOPE)
    elseif(SDK_VERSION EQUAL 15)
        set(MAIN_PATH "${SDK_DIRECTORY}/examples/connectivity/ble_connectivity/main.c" PARENT_SCOPE)
    else()
        message(STATUS "Not able to prepare SDK with configuration values because SDK v${SDK_VERSION} is unknown to me.")
        return()
    endif()

    set(SER_CONFIG_PATH "${SDK_DIRECTORY}/components/serialization/common/ser_config.h" PARENT_SCOPE)
endfunction()

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
    set(SER_CONFIG_PATH)
    set(MAIN_PATH)

    nrf_configure_sdk_affected_files(${SDK_VERSION} ${SDK_DIRECTORY} SER_CONFIG_PATH MAIN_PATH)

    if(NOT EXISTS ${SER_CONFIG_PATH})
        message("Not able to find ser_config.h in path ${SER_CONFIG_PATH}")
        return()
    endif()

    if(NOT EXISTS ${MAIN_PATH})
        message("Not able to find main.c in path ${SER_CONFIG_PATH}")
        return()
    endif()

    file(READ ${SER_CONFIG_PATH} SER_CONFIG)
    file(READ ${MAIN_PATH} MAIN)

    if(SDK_VERSION STREQUAL 11)
        string(REGEX REPLACE "#define (SER_PHY_UART_BAUDRATE)(.+)UART_BAUDRATE_BAUDRATE_Baud1M\n" "#cmakedefine \\1\\2@SER_PHY_UART_BAUDRATE@\n" SER_CONFIG_IN ${SER_CONFIG})
    elseif(SDK_VERSION STREQUAL 15)
        string(REGEX REPLACE "#define (SER_PHY_UART_BAUDRATE_VAL)(.+)1000000\n" "#cmakedefine \\1\\2@SER_PHY_UART_BAUDRATE_VAL@\n" SER_CONFIG_IN ${SER_CONFIG})
    endif()

    file(WRITE "${SER_CONFIG_PATH}.in" "${SER_CONFIG_IN}")

    # Version number is defined the same way in supported SDKS
    string(REGEX REPLACE "(\\.version_major[ ]+\\=[ ]+)0xf1(,\n)" "\\1@VERSION_MAJOR@\\2" MAIN_IN "${MAIN}")
    string(REGEX REPLACE "(\\.version_minor[ ]+\\=[ ]+)0xf2(,\n)" "\\1@VERSION_MINOR@\\2" MAIN_IN "${MAIN_IN}")
    string(REGEX REPLACE "(\\.version_patch[ ]+\\=[ ]+)0xf3(,\n)" "\\1@VERSION_PATCH@\\2" MAIN_IN "${MAIN_IN}")
    file(WRITE "${MAIN_PATH}.in" "${MAIN_IN}")
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
                elseif(_SD_API_VERSION STREQUAL "6.1.0")
                    set(_SD_ID "0x00AF")
                endif()
            endif()
        elseif(_SD_VERSION STREQUAL "s140")
            if(_SOC_FAMILY EQUAL 52)
                if(_SD_API_VERSION STREQUAL "6.0.0")
                    set(_SD_ID "0x00A9")
                elseif(_SD_API_VERSION STREQUAL "6.1.0")
                    set(_SD_ID "0x00AE")
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
    set(SOFTDEVICE_SEARCH_PATH "${CMAKE_SOURCE_DIR}/sd_api_v${MAJOR}/${SD_VERSION}_nrf${SOC_FAMILY}_${MAJOR}.*_softdevice.hex")
    file(GLOB ALTERNATIVE_SOFTDEVICE_HEX LIST_DIRECTORIES false "${SOFTDEVICE_SEARCH_PATH}")

    if(ALTERNATIVE_SOFTDEVICE_HEX)
        list(LENGTH ALTERNATIVE_SOFTDEVICE_HEX HEX_COUNT)
        if(HEX_COUNT GREATER 1)
            message(STATUS "Found ${HEX_COUNT} alternative hex files, not able to process that.")
            return()
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
