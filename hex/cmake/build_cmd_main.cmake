#[[
    This script is invoked from CMakeLists in this directory during the build steps 
    for the connectivity firmware.
]]

if(NOT DEFINED MAIN_IN_PATH)
    message(FATAL_ERROR "MAIN_IN_PATH not provided.")
endif()

if(NOT DEFINED CONNECTIVITY_VERSION)
    message(FATAL_ERROR "CONNECTIVITY_VERSION not provided.")
endif()

get_filename_component(MAIN_DIR "${MAIN_IN_PATH}" DIRECTORY)
set(MAIN_PATH "${MAIN_DIR}/main.c")

string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" MATCHES ${CONNECTIVITY_VERSION})

if(CONNECTIVITY_VERSION)
    set(VERSION_MAJOR "${CMAKE_MATCH_1}")
    set(VERSION_MINOR "${CMAKE_MATCH_2}")
    set(VERSION_PATCH "${CMAKE_MATCH_3}")
    #message(STATUS "Configuring ${MAIN_IN_PATH} with version ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}, output file ${MAIN_PATH}")
    configure_file(${MAIN_IN_PATH} ${MAIN_PATH} @ONLY)
else()
    message(FATAL_ERROR "Not able to parse version:${CONNECTIVITY_VERSION}")
endif()
