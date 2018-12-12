function(nrf_apply_patches)
    cmake_parse_arguments(nrf_apply_patches "QUIET" "SOURCE_PATH" "PATCHES" ${ARGN})

    find_program(GIT NAMES git git.cmd)
    set(PATCHNUM 0)

    # Check if we are inside a git work tree, if so, git apply will
    # not apply the patch if the files to patcha are not in the git
    # index. We only want to use git as a patch utlity without any
    # of the git features.
    execute_process(
        COMMAND "${GIT}" rev-parse --is-inside-work-tree
        WORKING_DIRECTORY ${nrf_apply_patches_SOURCE_PATH}
        RESULT_VARIABLE error_code
        ERROR_VARIABLE error
        OUTPUT_VARIABLE output
    )
     if(error_code)
        message(STATUS "Not inside a git work tree, applying patches.")
        message(STATUS "Applying patches in directory ${nrf_apply_patches_SOURCE_PATH}")
        string(STRIP "${error}" error)
        message(STATUS "git error: ${error}")
    else()
        message(FATAL_ERROR "Inside git work tree, not able to apply patches. Path is ${nrf_apply_patches_SOURCE_PATH}.")
    endif()

    foreach(PATCH ${nrf_apply_patches_PATCHES})
        message(STATUS "Applying patch ${PATCH}")

        execute_process(
            COMMAND "${GIT}" apply "${PATCH}" --ignore-whitespace
            WORKING_DIRECTORY ${nrf_apply_patches_SOURCE_PATH}
            RESULT_VARIABLE error_code
            ERROR_VARIABLE error
            OUTPUT_VARIABLE output
        )

        if(error_code)
            message(WARNING "Applying patch failed with error \"${error_code}\". This is expected if this patch was previously applied.")
            string(STRIP "${error}" error)
            message(STATUS "git error: ${error}")
        endif()

        math(EXPR PATCHNUM "${PATCHNUM}+1")
    endforeach()
endfunction()
