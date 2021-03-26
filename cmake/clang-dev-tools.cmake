if(IGNORE_CLANG_TIDY)
    message(STATUS "IGNORE_CLANG_TIDY is ON. Ignoring clang-tidy.")
    set(CLANG_TIDY_PATH "")
else()
    find_program(CLANG_TIDY_PATH NAMES clang-tidy-9 clang-tidy-10 clang-tidy HINTS "C:/Program Files/LLVM/bin")

    if(CLANG_TIDY_PATH)
        # Need to pass c++17 to clang-tidy as an extra argument from cmake to clang-tidy
        set(CMAKE_CXX_CLANG_TIDY  "${CLANG_TIDY_PATH};--extra-arg-before=-std=c++17")

        if(WIN32)
            set(CMAKE_CXX_CLANG_TIDY  "${CMAKE_CXX_CLANG_TIDY};--extra-arg-before=/EHsc")
        endif()

        message(STATUS "clang-tidy linter found in directory ${CLANG_TIDY_PATH}")
    else()
        message(STATUS "clang-tidy linter not found, linting will not be done.")
    endif()
endif()
