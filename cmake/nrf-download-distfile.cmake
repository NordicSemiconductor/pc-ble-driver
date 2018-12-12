function(nrf_download_distfile VAR)
    set(oneValueArgs FILENAME SHA512 DOWNLOAD_DIRECTORY)
    set(multipleValuesArgs URLS)
    cmake_parse_arguments(nrf_download_distfile "${options}" "${oneValueArgs}" "${multipleValuesArgs}" ${ARGN})

    if(NOT DEFINED nrf_download_distfile_URLS)
        message(FATAL_ERROR "nrf_download_distfile requires a URLS argument.")
    endif()
    if(NOT DEFINED nrf_download_distfile_FILENAME)
        message(FATAL_ERROR "nrf_download_distfile requires a FILENAME argument.")
    endif()
    if(NOT DEFINED nrf_download_distfile_DOWNLOAD_DIRECTORY)
        message(FATAL_ERROR "nrf_download_distfile requires a DOWNLOAD_DIRECTORY argument.")
    endif()
    if(NOT nrf_download_distfile_SHA512)
        message(FATAL_ERROR "nrf_download_distfile requires a SHA512 argument.")
    endif()

    set(downloaded_file_path ${nrf_download_distfile_DOWNLOAD_DIRECTORY}/${nrf_download_distfile_FILENAME})
    set(download_file_path_part "${nrf_download_distfile_DOWNLOAD_DIRECTORY}/temp/${nrf_download_distfile_FILENAME}")

    file(REMOVE_RECURSE "${nrf_download_distfile_DOWNLOAD_DIRECTORY}/temp")
    file(MAKE_DIRECTORY "${nrf_download_distfile_DOWNLOAD_DIRECTORY}/temp")

    function(test_hash FILE_PATH FILE_KIND CUSTOM_ERROR_ADVICE)
        message(STATUS "Testing integrity of ${FILE_KIND}...")
        file(SHA512 ${FILE_PATH} FILE_HASH)
        if(NOT "${FILE_HASH}" STREQUAL "${nrf_download_distfile_SHA512}")
            message(FATAL_ERROR
                "\nFile does not have expected hash:\n"
                "        File path: [ ${FILE_PATH} ]\n"
                "    Expected hash: [ ${nrf_download_distfile_SHA512} ]\n"
                "      Actual hash: [ ${FILE_HASH} ]\n"
                "${CUSTOM_ERROR_ADVICE}\n")
        endif()
        message(STATUS "Testing integrity of ${FILE_KIND}... OK")
    endfunction()

    if(EXISTS ${downloaded_file_path})
        message(STATUS "Using cached ${downloaded_file_path}")
        test_hash("${downloaded_file_path}" "cached file" "Please delete the file and retry if this file should be downloaded again.")
    else()
        # Tries to download the file.
        list(GET nrf_download_distfile_URLS 0 SAMPLE_URL)
        foreach(url IN LISTS nrf_download_distfile_URLS)
            message(STATUS "Downloading ${url}...")
            file(DOWNLOAD ${url} "${download_file_path_part}" STATUS download_status)
            list(GET download_status 0 status_code)
            if (NOT "${status_code}" STREQUAL "0")
                message(STATUS "Downloading ${url}... Failed. Status: ${download_status}")
                set(download_success 0)
            else()
                message(STATUS "Downloading ${url}... OK")
                set(download_success 1)
                break()
            endif()
        endforeach(url)

        if (NOT download_success)
            message(FATAL_ERROR
            "    \n"
            "    Failed to download file.\n"
            "    If you use a proxy, please set the HTTPS_PROXY and HTTP_PROXY environment\n"
            "    variables to \"https://user:password@your-proxy-ip-address:port/\".\n")
        else()
            test_hash("${download_file_path_part}" "downloaded file" "The file may have been corrupted in transit. This can be caused by proxies. If you use a proxy, please set the HTTPS_PROXY and HTTP_PROXY environment variables to \"https://user:password@your-proxy-ip-address:port/\".\n")
            get_filename_component(downloaded_file_dir "${downloaded_file_path}" DIRECTORY)
            file(MAKE_DIRECTORY "${downloaded_file_dir}")
            file(RENAME ${download_file_path_part} ${downloaded_file_path})
        endif()
    endif()
    set(${VAR} ${downloaded_file_path} PARENT_SCOPE)
endfunction()
