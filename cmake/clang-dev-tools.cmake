if(WIN32)
	# CMAKE_EXPORT_COMPILE_COMMANDS is not working on Windows
	message(STATUS "clang-tidy is not supported on Windows")

	# For the target not to fail:
	add_custom_target(tidy COMMAND echo "not supported on Windows")

else()
	find_program(CLANG_TIDY NAMES run-clang-tidy.py run-clang-tidy-6.0.py)
	if(NOT CLANG_TIDY)

		message(STATUS "Did not find clang-tidy, target tidy is disabled.")
		message(STATUS "If clang-tidy is installed, make sure run-clang-tidy.py is in PATH")

		# For the target not to fail:
		add_custom_target(tidy COMMAND echo "Clang-tidy is not installed")

	else()
		message(STATUS "Found clang-tidy, use \"make tidy\" to run it.")

		# This will create build/compile_commands.json
		set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

		set(CLANG_TIDY_CHECKS "*")
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-llvm-header-guard")
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-llvm-include-order")
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-llvm-namespace-comment")
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-readability-else-after-return")
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-misc-macro-parentheses")
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-clang-analyzer-alpha.core.CastToStruct")
		# Modernize
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-modernize-raw-string-literal")
		# CPP Core Guidelines
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-cppcoreguidelines-pro-bounds-array-to-pointer-decay")
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-cppcoreguidelines-pro-bounds-constant-array-index")
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-cppcoreguidelines-pro-bounds-pointer-arithmetic")
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-cppcoreguidelines-pro-type-member-init") # as of https://llvm.org/bugs/show_bug.cgi?id=31039
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-cppcoreguidelines-pro-type-reinterpret-cast")
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-cppcoreguidelines-pro-type-vararg")
		# Google
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-google-readability-namespace-comments")
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-google-readability-braces-around-statements,-readability-braces-around-statements")
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-google-readability-todo")
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-google-runtime-int")
		set(CLANG_TIDY_CHECKS "${CLANG_TIDY_CHECKS},-google-runtime-references")
		set(CLANG_TIDY_CHECKS "-checks='${CLANG_TIDY_CHECKS}'")

		add_custom_target(tidy
			COMMAND ${CLANG_TIDY} ${CLANG_TIDY_CHECKS}
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build/
		)
	endif()
endif()

