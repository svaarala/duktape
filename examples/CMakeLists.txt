
# Example: Adder Program
add_executable(duk_example_adder hello/hello.c)
target_link_libraries(duk_example_adder duktape)


# Example: Command line application.
SET( DUK_CMDLINE_SRC
	"cmdline/duk_cmdline.c"
	"alloc-logging/duk_alloc_logging.c"
	"alloc-torture/duk_alloc_torture.c"
	"alloc-hybrid/duk_alloc_hybrid.c"
	"debug-trans-socket/duk_trans_socket.c"
	)
SET( DUK_CMDLINE_HDRS
	"alloc-logging/duk_alloc_logging.h"
	"alloc-torture/duk_alloc_torture.h"
	"alloc-hybrid/duk_alloc_hybrid.h"
	"debug-trans-socket/duk_trans_socket.h"
	)

if(MSVC)
	# Make MSVC warning C4996 go away.
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(DUKTAPE_DEBUGGER_SUPPORT)
    # enable --debugger in ./duk
	add_definitions(-DDUK_CMDLINE_DEBUGGER_SUPPORT )
endif()
include_directories( "alloc-logging" "alloc-torture" "alloc-hyrid" "debug-trans-socket")
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
	
add_executable (duk-test "debug-trans-dvalue/test.c" ${DUK_DEBUG_TRANS_DVALUE_SRC} ${DUK_DEBUG_TRANS_DVALUE_HDRS})	
target_link_libraries(duk-test duktape)

add_executable (duk-example-objects
	"custom-c-object/custom-object-main.c"
	"custom-c-object/custom-object-shape.c"
	"custom-c-object/custom-object-vector2d.c"
	"custom-c-object/custom-object.h"
	)	
target_link_libraries(duk-example-objects duktape)
