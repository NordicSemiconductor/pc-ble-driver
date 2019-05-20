set(clang_format_min_version "6.0.0")

find_program(clang_format clang-format)
if (NOT clang_format)
    message(FATAL_ERROR "clang-format not found")
endif()

set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
file(GLOB_RECURSE cpp_files 
    "${ROOT_DIR}/src/**/*.cpp"
    "${ROOT_DIR}/../include/common/**/*.h"
    "${ROOT_DIR}/examples/**/.c"
    "${ROOT_DIR}/test/**/*.cpp"
    "${ROOT_DIR}/test/**/*.h"
)

execute_process(COMMAND ${clang_format} "-version" RESULT_VARIABLE result OUTPUT_VARIABLE version)
if (NOT ${result} EQUAL 0)
    message(FATAL_ERROR "Not able to get version from clang-format")
endif()

string(REPLACE " " ";" version "${version}")
list(GET version 2 version)

if(NOT ${version} VERSION_GREATER_EQUAL "${clang_format_min_version}")
    message(FATAL_ERROR "clang-format version ${clang_format_min_version} or higher required, found ${version}")
endif()

foreach(cpp_file IN LISTS cpp_files)
    execute_process(COMMAND ${clang_format} "-style=file" ${cpp_file} -i RESULT_VARIABLE result ERROR_VARIABLE stderr)

    if (NOT ${result} EQUAL 0)
        message(FATAL_ERROR "Error formatting file: ${cpp_file}, please revert the changes done by this target.")
    endif()
endforeach()
