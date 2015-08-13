add_custom_target(duk_dist)

function(duktape_dist_echo myMessage)
	ADD_CUSTOM_COMMAND(TARGET duk_dist
	  COMMAND ${CMAKE_COMMAND} -E echo "${myMessage}"
	  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endfunction(duktape_dist_echo)

function(duktape_dist_copy source_dir target_dir)
	list(REMOVE_AT ARGV 0 1)
	foreach(_DUK_SRC IN ITEMS ${ARGV})
		ADD_CUSTOM_COMMAND(TARGET duk_dist
		  COMMAND ${CMAKE_COMMAND} -E copy "${source_dir}/${_DUK_SRC}" "${target_dir}"
		  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
	 endforeach()
endfunction(duktape_dist_copy)

#Generate dist target:
ADD_CUSTOM_COMMAND(TARGET duk_dist
	  POST_BUILD
	  COMMAND ${CMAKE_COMMAND} -E make_directory dist
	  COMMAND ${CMAKE_COMMAND} -E make_directory dist/src
	  COMMAND ${CMAKE_COMMAND} -E make_directory dist/src-seperate
	  COMMAND ${CMAKE_COMMAND} -E make_directory dist/debugger
	  COMMAND ${CMAKE_COMMAND} -E make_directory dist/examples
	  COMMAND ${CMAKE_COMMAND} -E make_directory dist/polyfills
	  COMMAND ${CMAKE_COMMAND} -E make_directory dist/licenses
	  COMMAND ${CMAKE_COMMAND} -E copy_directory src "${CMAKE_BINARY_DIR}/dist/src-seperate"
	  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

ADD_CUSTOM_COMMAND(TARGET duk_dist
	  POST_BUILD
	  COMMAND ${CMAKE_COMMAND} -E copy LICENSE.txt "${CMAKE_BINARY_DIR}/dist"
	  COMMAND ${CMAKE_COMMAND} -E copy AUTHORS.rst "${CMAKE_BINARY_DIR}/dist"
  	  COMMAND ${CMAKE_COMMAND} -E copy_directory dist-files/ "${CMAKE_BINARY_DIR}/dist"
	  #COMMAND ${CMAKE_COMMAND} -E echo "Copying Debugger..."
	  #COMMAND ${CMAKE_COMMAND} -E copy_directory debugger "${CMAKE_BINARY_DIR}/dist/debugger"
	  COMMAND ${CMAKE_COMMAND} -E echo "Copying Examples..."
	  COMMAND ${CMAKE_COMMAND} -E copy_directory examples "${CMAKE_BINARY_DIR}/dist/examples"
	  COMMAND ${CMAKE_COMMAND} -E copy_directory licenses "${CMAKE_BINARY_DIR}/dist/licenses"
	  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
duktape_dist_echo("Copying Debugger.")
duktape_dist_copy( "${CMAKE_SOURCE_DIR}/debugger" "${CMAKE_BINARY_DIR}/dist/debugger"
	"README.rst"
	"Makefile"
	"package.json"
	"duk_debug.js"
	"duk_classnames.yaml"
	"duk_debugcommands.yaml"
	"duk_opcodes.yaml")
duktape_dist_copy(  "${CMAKE_SOURCE_DIR}/debugger/static" "${CMAKE_BINARY_DIR}/dist/debugger/static"
	"index.html"
	"style.css"
	"webui.js"
	)

duktape_dist_echo("Copying Source....")
foreach(_DUK_SRC IN ITEMS ${DUKTAPE_LIB_H_SRC} ${DUKTAPE_LIB_C_SRC})
	ADD_CUSTOM_COMMAND(TARGET duk_dist
		  POST_BUILD
		  COMMAND ${CMAKE_COMMAND} -E copy 
			"${_DUK_SRC}" "${CMAKE_BINARY_DIR}/dist/src-seperate"
		  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}	
	)
endforeach()
set(DUK_SINGLE_FILE "#define DUK_SINGLE_FILE")

ADD_CUSTOM_COMMAND(TARGET duk_dist
	  POST_BUILD
	  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}	
)
configure_file(src/duktape.h.in dist/src/duktape.h)

ADD_CUSTOM_COMMAND(TARGET duk_dist
	 COMMAND ${CMAKE_COMMAND} -E echo "Combining Sources..."
	 COMMAND ${PYTHON_EXECUTABLE} "util/combine_src.py"
	"${CMAKE_BINARY_DIR}/dist/src-seperate" "${CMAKE_BINARY_DIR}/dist/src/duktape.c"
	"${DUKTAPE_COMPACT_VERSION}" "${GIT_COMMIT}" "${GIT_DESCRIBE}"
	"LICENSE.txt.tmp" "AUTHORS.rst.tmp"
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}		
	)
	
ADD_CUSTOM_COMMAND(TARGET duk_dist
	 COMMAND ${CMAKE_COMMAND} -E echo "Creating Zip File..."
	 COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_SOURCE_DIR}/util/dist_pack.py"
		"--input-path" "dist" "-o" "duktape" "-z"
		"--compact-ver" "${DUKTAPE_COMPACT_VERSION}"
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
	
ADD_CUSTOM_COMMAND(TARGET duk_dist
	 COMMAND ${CMAKE_COMMAND} -E echo "Creating tar.gz File..."
	 COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_SOURCE_DIR}/util/dist_pack.py"
		"--input-path" "dist" "-o" "duktape" "--tgz"
		"--compact-ver" "${DUKTAPE_COMPACT_VERSION}"
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)

ADD_CUSTOM_COMMAND(TARGET duk_dist
	 COMMAND ${CMAKE_COMMAND} -E echo "Creating tar.bz2 File..."
	 COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_SOURCE_DIR}/util/dist_pack.py"
		"--input-path" "dist" "-o" "duktape" "--tbz2"
		"--compact-ver" "${DUKTAPE_COMPACT_VERSION}"
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)	