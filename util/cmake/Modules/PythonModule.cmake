# Python Module Detection Scripts.
# Created by Dylan Baker
# Source: http://cgit.freedesktop.org/piglit/commit/cmake/Modules/PythonModule.cmake?id=a55fbebd5e227a4a29c83e85d8b44154cdcbf145

function(find_python_module MODULE PREFIX)
	execute_process(
		COMMAND ${PYTHON_EXECUTABLE} -c "import sys, ${MODULE}; sys.stdout.write(${MODULE}.__version__)"
		OUTPUT_VARIABLE _version
		RESULT_VARIABLE _status
		ERROR_QUIET
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	# Export version variables to parent scope. This is needed by
	# find_package_handle_standard_args
	set(${PREFIX}_VERSION_STRING ${_version} PARENT_SCOPE)

	# A status returns 0 if everything is okay. And zero is false. To make
	# checking in the outer scope less surprising
	if (_status EQUAL 0)
		set("${PREFIX}_STATUS" "success" PARENT_SCOPE)
	endif (_status EQUAL 0)
endfunction(find_python_module MODULE PREFIX)

# This macro provides a simple way for basic python find modules to be
# extremely simple without duplicate boilerplate
macro(basic_python_module MODULE PREFIX)
	find_python_module("${MODULE}" "${PREFIX}")

	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(
		"${PREFIX}"
		REQUIRED_VARS "${PREFIX}_STATUS"
		VERSION_VAR "${PREFIX}_VERSION_STRING"
	)

	# This isn't needed in the parent scope, just here.
	unset("${PREFIX}_STATUS")
endmacro(basic_python_module MODULE PREFIX)
