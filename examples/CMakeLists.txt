# Example: Adder Program
add_executable(duk_example_adder hello/hello.c)
target_link_libraries(duk_example_adder duktape)


# Example: Command line application.
list(APPEND DUK_CMDLINE_SRC
	"cmdline/duk_cmdline.c"
	"alloc-logging/duk_alloc_logging.c"
	"alloc-torture/duk_alloc_torture.c"
	"alloc-hybrid/duk_alloc_hybrid.c"
	)
list(APPEND DUK_CMDLINE_HDRS
	"alloc-logging/duk_alloc_logging.h"
	"alloc-torture/duk_alloc_torture.h"
	"alloc-hybrid/duk_alloc_hybrid.h"
	)
if(MSVC)
	# Make MSVC warning C4996 go away.
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
include_directories( "alloc-logging" "alloc-torture" "alloc-hyrid")
if(DUKTAPE_DEBUGGER_SUPPORT)
	list(APPEND DUK_CMDLINE_HDRS
		"debug-trans-socket/duk_trans_socket.h"
		)
	list(APPEND DUK_CMDLINE_SRC
		"debug-trans-socket/duk_trans_socket.c"
		)
    # enable --debugger in ./duk
	add_definitions(-DDUK_CMDLINE_DEBUGGER_SUPPORT )
	include_directories( "debug-trans-socket" )
endif()

add_executable (duk ${DUK_CMDLINE_SRC} ${DUK_CMDLINE_HDRS})	
target_link_libraries(duk duktape)
set_property(TARGET duk APPEND PROPERTY COMPILE_DEFINITIONS
	$<$<CONFIG:Debug>:DUK_OPT_DEBUG>
	$<$<CONFIG:RelWithDebInfo>:DUK_OPT_DEBUG>
)

# Example: Debug transport with local debug protocol encoding/decoding
SET( DUK_DEBUG_TRANS_DVALUE_SRC
	"debug-trans-dvalue/duk_trans_dvalue.c"
	)
SET( DUK_DEBUG_TRANS_DVALUE_HDRS
	"debug-trans-dvalue/duk_trans_dvalue.h"
	)
add_executable (duk-trans-dvalue-test "debug-trans-dvalue/test.c" ${DUK_DEBUG_TRANS_DVALUE_SRC} ${DUK_DEBUG_TRANS_DVALUE_HDRS})	
target_link_libraries(duk-trans-dvalue-test duktape)
set_property(TARGET duk-trans-dvalue-test APPEND PROPERTY COMPILE_DEFINITIONS
	$<$<CONFIG:Debug>:DUK_OPT_DEBUG>
	$<$<CONFIG:RelWithDebInfo>:DUK_OPT_DEBUG>
)

add_executable (duk-example-objects
	"custom-c-object/custom-object-main.c"
	"custom-c-object/custom-object-shape.c"
	"custom-c-object/custom-object-vector2d.c"
	"custom-c-object/custom-object.h"
	)	
target_link_libraries(duk-example-objects duktape)
set_property(TARGET duk-example-objects APPEND PROPERTY COMPILE_DEFINITIONS
	$<$<CONFIG:Debug>:DUK_OPT_DEBUG>
	$<$<CONFIG:RelWithDebInfo>:DUK_OPT_DEBUG>
)