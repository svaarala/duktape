# Duktape CMake Build file.
cmake_minimum_required(VERSION 3.0)
 
project (Duktape)

option(DUKTAPE_BUILD_EXAMPLES "Build Example Code" ON)
option(DUKTAPE_BUILD_SHARED "Build duktape as a Shared library" OFF)
option(DUKTAPE_DEBUGGER_SUPPORT "Build duktape with Debugger Support." ON)
option(DUKTAPE_BUILD_SEPARABLE "Build Duktape using Split Files" OFF)
option(DUKTAPE_ENABLE_DEBUG_PRINT
	"Build duktape library with dubug prining enabled." ON)

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


if(EXISTS ${PROJECT_SOURCE_DIR}/src-separate AND DUKTAPE_BUILD_SEPARABLE)
	file(GLOB DUKTAPE_LIB_H_SRC "src-separate/*.h")
	file(GLOB DUKTAPE_LIB_C_SRC "src-separate/*.c")
	include_directories(${PROJECT_SOURCE_DIR}/src-separate )
else()
	file(GLOB DUKTAPE_LIB_H_SRC "src/*.h")
	file(GLOB DUKTAPE_LIB_C_SRC "src/duktape.c")
	include_directories(${PROJECT_SOURCE_DIR}/src )
endif()


if(DUKTAPE_BUILD_SHARED)
	add_library(duktape SHARED ${DUKTAPE_LIB_C_SRC})
else()
	add_library(duktape STATIC ${DUKTAPE_LIB_C_SRC})
endif()

# add some default Debug Defines.
set_property(TARGET duktape APPEND_STRING PROPERTY COMPILE_DEFINITIONS
		$<$<CONFIG:Debug>:DUK_OPT_DEBUG>
		$<$<CONFIG:RelWithDebInfo>:DUK_OPT_DEBUG>
		)
		
if(DUKTAPE_ENABLE_DEBUG_PRINT)		
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

