if(WIN32)
    if (NOT CMAKE_GENERATOR STREQUAL "Ninja")
        set(NOT_SUPPORTED_MESSAGE "clang-tidy is only supported with Ninja generator on Microsoft Windows.")
        message(STATUS "${NOT_SUPPORTED_MESSAGE}")

        # For the target not to fail:
        add_custom_target(tidy COMMAND echo "${NOT_SUPPORTED_MESSAGE}")
        return()
    else()
        find_package (Python COMPONENTS Interpreter)

        if(NOT Python_Interpreter_FOUND)
            set(PYTHON_MISSING_MESSAGE "Python interpreter not found, it is required for run-clang-tidy.py")
            message(STATUS "${PYTHON_MISSING_MESSAGE}")
            add_custom_target(tidy COMMAND echo "${PYTHON_MISSING_MESSAGE}")
            return()
        endif()
    endif()
endif()

find_program(CLANG_TIDY NAMES run-clang-tidy-7.py run-clang-tidy.py)

if(NOT CLANG_TIDY)
    message(STATUS "Did not find clang-tidy, target tidy is disabled.")
    message(STATUS "If clang-tidy is installed, make sure run-clang-tidy.py and clang-tidy is in PATH")

    # For the target not to fail:
    add_custom_target(tidy COMMAND echo "Clang-tidy is not installed")
else()
    message(STATUS "Found clang-tidy, use \"cmake --build . --target tidy\" to run it.")

    # This will create build/compile_commands.json
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    # Configuration of rules are picked up from .clang-tidy
    message(STATUS "Picking up clang-tidy rules from ${CMAKE_SOURCE_DIR}/.clang-tidy")
    set(CLANG_TIDY_ARGS "-header-filter=.*")

    if (WIN32)
        add_custom_target(tidy
            COMMAND ${Python_EXECUTABLE} ${CLANG_TIDY} ${CLANG_TIDY_ARGS} .
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    else()
        add_custom_target(tidy
            COMMAND ${CLANG_TIDY} ${CLANG_TIDY_ARGS} .
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()
endif()
