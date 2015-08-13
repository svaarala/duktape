# Duktape CMake Build file for Git Master
cmake_minimum_required(VERSION 2.8.10)
 
project (duktape)
option(DUKTAPE_BUILD_EXAMPLES "Build Example Code" ON)
option(DUKTAPE_BUILD_SHARED "Build duktape as a Shared library" OFF)
option(DUKTAPE_DEBUGGER_SUPPORT "Build duktape with Debugger Support." OFF)
option(DUKTAPE_DEBUGGER_JQUERY  "Build duktape with Debugger Support." OFF)
option(DUKTAPE_ENABLE_DEBUG_PRINT
	"Build duktape library with dubug prining enabled." ON)
option(DUKTAPE_ENABLE_CODE_POLICY_CHECK "Add Code Policy Target to Build" ON)
option(DUKTAPE_GENERATE_DIST "Generate Dist release" ON)
if(EXISTS "${PROJECT_SOURCE_DIR}/.git")
	include(CMake-build-git.cmake)
else()
	MESSAGE(FATAL_ERROR "This CMake File is meant to be ran from the Duktape Git Master only.")
endif()

if(DUKTAPE_ENABLE_CODE_POLICY_CHECK)
	include(CMake-check-code-policy.cmake)
endif()

if(DUKTAPE_DEBUGGER_SUPPORT)
	# These Options are used for Basic Debugging support.
	add_definitions(
		-DDUK_OPT_DEBUGGER_SUPPORT         # enable debugger support in Duktape
		-DDUK_OPT_INTERRUPT_COUNTER        # prerequisite for debugging
		-DDUK_OPT_DEBUGGER_FWD_PRINTALERT  # optional debugger features
		-DDUK_OPT_DEBUGGER_FWD_LOGGING     # Debug Logging Printouts
		-DDUK_OPT_DEBUGGER_DUMPHEAP
	)
endif()

include_directories(${PROJECT_SOURCE_DIR}/src )
include_directories(${PROJECT_BINARY_DIR}/src )
file(GLOB DUKTAPE_LIB_H_SRC "src/*.h")
file(GLOB DUKTAPE_LIB_C_SRC "src/*.c")

# Create The Core Include file.
include_directories(${PROJECT_SOURCE_DIR}/src )
include_directories(${PROJECT_BINARY_DIR}/src )
if(DUKTAPE_BUILD_SHARED)
	add_library(duktape SHARED ${DUKTAPE_LIB_C_SRC} ${DUKTAPE_LIB_H_SRC})
else()
	add_library(duktape STATIC ${DUKTAPE_LIB_C_SRC}  ${DUKTAPE_LIB_H_SRC})
endif()
get_target_property(DUKTAPE_DEFINITIONS duktape COMPILE_DEFINITIONS)
get_target_property(DUKTAPE_DEBUG_DEFINITIONS duktape COMPILE_DEFINITIONS_DEBUG)
set_property(TARGET duktape APPEND_STRING PROPERTY COMPILE_DEFINITIONS
	# debug Build
	$<$<CONFIG:Debug>:DUK_OPT_DEBUG>
	# Release With debug information.	
	$<$<CONFIG:RelWithDebInfo>:DUK_OPT_DEBUG>
	)

if(DUKTAPE_ENABLE_DEBUG_PRINT)	
	# Prepare the Debug options of Duktape.
	set_property(TARGET duktape APPEND_STRING PROPERTY COMPILE_DEFINITIONS
		# debug Build
		$<$<CONFIG:Debug>:DUK_OPT_DPRINT>
		$<$<CONFIG:Debug>:DUK_OPT_DDPRINT>
		$<$<CONFIG:Debug>:DUK_OPT_DDDPRINT>
		# Release With debug information.
		$<$<CONFIG:RelWithDebInfo>:DUK_OPT_DPRINT>
		$<$<CONFIG:RelWithDebInfo>:DUK_OPT_DDPRINT>
		$<$<CONFIG:RelWithDebInfo>:DUK_OPT_DDDPRINT>
	)
endif()

if(DUKTAPE_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

if(DUKTAPE_DEBUGGER_SUPPORT AND DUKTAPE_DEBUGGER_JQUERY)
	include(CMake-build-debugger.cmake)
endif()

if(DUKTAPE_GENERATE_DIST)
	include(CMake-generate-dist.cmake)
endif()