#TODO: Run Python Script.
find_package(PythonInterp REQUIRED)

if(WIN32 AND NOT EXISTS "${PROJECT_SOURCE_DIR}/debugger/node.exe")
	message(STATUS "Preparing Debugger:")
	EXECUTE_PROCESS(COMMAND
		${PYTHON_EXECUTABLE} "../util/prepare_debugger.py"
		WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/debugger"
		)
	# Update Node.js Package Manager.
	EXECUTE_PROCESS(COMMAND
		"${PROJECT_SOURCE_DIR}/node.exe" "bin\npm-cli.js" "update"
		WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/debugger"
		)
	# Install Node.js Packages
	EXECUTE_PROCESS(COMMAND
		"${PROJECT_SOURCE_DIR}/node.exe" "bin\npm-cli.js" "install"
		WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/debugger"
		)

endif()
