if(NOT EXISTS "${PROJECT_BINARY_DIR}/src")
	file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src")
endif()


find_package(Git REQUIRED)
find_package(PythonInterp REQUIRED)

if(GIT_FOUND)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} log -n 1 --format=%h 
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_COMMIT_SHORT
		)
	string(STRIP "${GIT_COMMIT_SHORT}" GIT_COMMIT_SHORT)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} log -n 1 --format=%H
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_COMMIT
		)
	string(STRIP "${GIT_COMMIT}" GIT_COMMIT)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} describe --always --dirty
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_DESCRIBE
		)
	string(STRIP "${GIT_DESCRIBE}" GIT_DESCRIBE)
	set(GIT_DESCRIBE_CSTRING "\"${GIT_DESCRIBE}\"")
	execute_process(
		COMMAND ${GIT_EXECUTABLE} describe --always --dirty --abbrev=0
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		OUTPUT_VARIABLE DUK_VERSION
		)
	string(STRIP "${DUK_VERSION}" DUK_VERSION)
	#TODO: Fix this some more.
	string(REGEX REPLACE "^v([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" DUKTAPE_MAJOR_VERSION "${DUK_VERSION}")
	string(REGEX REPLACE "^v[0-9]+\\.([0-9]+)\\.[0-9]+.*" "\\1" DUKTAPE_MINOR_VERSION "${DUK_VERSION}")
	string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" DUKTAPE_PATCH_VERSION "${DUK_VERSION}")
	MATH( EXPR DUKTAPE_COMPACT_VERSION "
			10000 * ${DUKTAPE_MAJOR_VERSION} +
			100 * ${DUKTAPE_MINOR_VERSION}   +
			${DUKTAPE_PATCH_VERSION}"
			)
	message( STATUS "Project Git Version: ${DUK_VERSION}")
	message( STATUS "Project Short Revision: ${GIT_COMMIT_SHORT}")
	message( STATUS "Project Long Revision: ${GIT_COMMIT}")
	
endif()


if(NOT EXISTS "${PROJECT_BINARY_DIR}/src/duk_unicode_ids_noa.c")
	message(STATUS "Preparing Unicode Data:")
	EXECUTE_PROCESS(COMMAND
		${PYTHON_EXECUTABLE} "src/prepare_unicode_data.py" "src/UnicodeData.txt" "${PROJECT_BINARY_DIR}/UnicodeData-expanded.tmp"
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		OUTPUT_VARIABLE EXTRACT_UNICODE_DATA)
	message(STATUS "Extracting Case Conventions:")
	EXECUTE_PROCESS(COMMAND
		${PYTHON_EXECUTABLE} "${PROJECT_SOURCE_DIR}/src/extract_caseconv.py"
			"--unicode-data=UnicodeData-expanded.tmp"
			"--special-casing=${PROJECT_SOURCE_DIR}/src/SpecialCasing.txt"
			"--out-source=src/duk_unicode_caseconv.c"
			"--out-header=src/duk_unicode_caseconv.h"
			"--table-name-lc=duk_unicode_caseconv_lc"
			"--table-name-uc=duk_unicode_caseconv_uc"
			WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
			OUTPUT_VARIABLE EXTRACT_CASECONV
		)
	message(STATUS "Extracting Character Maps:")
	function(duktape_extract_chars include_cat exclude_cat out_desc)
		message( STATUS "Generating Unicode Table: duk_unicode_${out_desc}")
		EXECUTE_PROCESS(COMMAND
		${PYTHON_EXECUTABLE} "${PROJECT_SOURCE_DIR}/src/extract_chars.py"
				"--unicode-data=UnicodeData-expanded.tmp"
				"--include-categories=\"${include_cat}\""
				"--exclude-categories=\"${exclude_cat}\""
				"--out-source=src/duk_unicode_${out_desc}.c "
				"--out-header=src/duk_unicode_${out_desc}.h"
				"--table-name=duk_unicode_${out_desc}"
		
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
		OUTPUT_VARIABLE EXTRACT_CHARS_${out_desc})
	endfunction(duktape_extract_chars)
	#Prepare Unicode Data Extraction
	
	# Whitespace (unused now)
	set(WHITESPACE_INCL "Zs") # USP = Any other Unicode space separator
	set(WHITESPACE_EXCL "NONE")
	# Unicode letter (unused now)
	set(LETTER_INCL "Lu,Ll,Lt,Lm,Lo")
	set(LETTER_EXCL  "NONE")
	set(LETTER_NOA_INCL "Lu,Ll,Lt,Lm,Lo")
	set(LETTER_NOA_EXCL "ASCII")
	set(LETTER_NOABMP_INCL "${LETTER_NOA_INCL}")
	set(LETTER_NOABMP_EXCL "ASCII,NONBMP")
	
	# Identifier start
	# E5 Section 7.6
	set(IDSTART_INCL "Lu,Ll,Lt,Lm,Lo,Nl,0024,005F")
	set(IDSTART_EXCL "NONE")
	set(IDSTART_NOA_INCL "Lu,Ll,Lt,Lm,Lo,Nl,0024,005F")
	set(IDSTART_NOA_EXCL "ASCII")
	set(IDSTART_NOABMP_INCL "${IDSTART_NOA_INCL}")
	set(IDSTART_NOABMP_EXCL "ASCII,NONBMP")

	# Identifier start - Letter: allows matching of (rarely needed) 'Letter'
	# production space efficiently with the help of IdentifierStart.  The
	# 'Letter' production is only needed in case conversion of Greek final
	# sigma.
	set(IDSTART_MINUS_LETTER_INCL "${IDSTART_NOA_INCL}")
	set(IDSTART_MINUS_LETTER_EXCL "Lu,Ll,Lt,Lm,Lo")
	set(IDSTART_MINUS_LETTER_NOA_INCL "${IDSTART_NOA_INCL}")
	set(IDSTART_MINUS_LETTER_NOA_EXCL "Lu,Ll,Lt,Lm,Lo,ASCII")
	set(IDSTART_MINUS_LETTER_NOABMP_INCL "${IDSTART_NOA_INCL}")
	set(IDSTART_MINUS_LETTER_NOABMP_EXCL "Lu,Ll,Lt,Lm,Lo,ASCII,NONBMP")

	# Identifier start - Identifier part
	# E5 Section 7.6: IdentifierPart, but remove IdentifierStart (already above)
	set(IDPART_MINUS_IDSTART_INCL "Lu,Ll,Lt,Lm,Lo,Nl,0024,005F,Mn,Mc,Nd,Pc,200C,200D")
	set(IDPART_MINUS_IDSTART_EXCL "Lu,Ll,Lt,Lm,Lo,Nl,0024,005F")
	set(IDPART_MINUS_IDSTART_NOA_INCL "Lu,Ll,Lt,Lm,Lo,Nl,0024,005F,Mn,Mc,Nd,Pc,200C,200D")
	set(IDPART_MINUS_IDSTART_NOA_EXCL "Lu,Ll,Lt,Lm,Lo,Nl,0024,005F,ASCII")
	set(IDPART_MINUS_IDSTART_NOABMP_INCL "${IDPART_MINUS_IDSTART_NOA_INCL}")
	set(IDPART_MINUS_IDSTART_NOABMP_EXCL "Lu,Ll,Lt,Lm,Lo,Nl,0024,005F,ASCII,NONBMP")

	duktape_extract_chars( "${WHITESPACE_INCL}" "${WHITESPACE_EXCL}" "ws")
	duktape_extract_chars( "${LETTER_INCL}" "${LETTER_EXCL}" "let")
	duktape_extract_chars( "${LETTER_NOA_INCL}" "${LETTER_NOA_EXCL}" "let_noa")
	duktape_extract_chars( "${LETTER_NOABMP_INCL}" "${LETTER_NOABMP_EXCL}" "let_noabmp")
	duktape_extract_chars( "${IDSTART_INCL}" "${IDSTART_EXCL}" "ids")
	duktape_extract_chars( "${IDSTART_NOA_INCL}" "${IDSTART_NOA_EXCL}" "ids_noa")
	duktape_extract_chars( "${IDSTART_NOABMP_INCL}" "${IDSTART_NOABMP_EXCL}" "ids_noabmp")
	duktape_extract_chars( "${IDSTART_MINUS_LETTER_INCL}" "${IDSTART_MINUS_LETTER_EXCL}" "ids_m_let")
	duktape_extract_chars( "${IDSTART_MINUS_LETTER_NOA_INCL}" "${IDSTART_MINUS_LETTER_NOA_EXCL}" "ids_m_let_noa")
	duktape_extract_chars( "${IDSTART_MINUS_LETTER_NOABMP_INCL}" "${IDSTART_MINUS_LETTER_NOABMP_EXCL}" "ids_m_let_noabmp")
	duktape_extract_chars( "${IDPART_MINUS_IDSTART_INCL}" "${IDPART_MINUS_IDSTART_EXCL}" "idp_m_ids")
	duktape_extract_chars( "${IDPART_MINUS_IDSTART_NOA_INCL}" "${IDPART_MINUS_IDSTART_NOA_EXCL}" "idp_m_ids_noa")
	duktape_extract_chars( "${IDPART_MINUS_IDSTART_NOABMP_INCL}" "${IDPART_MINUS_IDSTART_NOABMP_EXCL}" "idp_m_ids_noabmp")
endif()



if(NOT EXISTS "${PROJECT_SOURCE_DIR}/src/duk_config.h" AND NOT EXISTS "${PROJECT_BINARY_DIR}/src/duk_config.h")
	#TODO: Check for PyYAML
	message(STATUS "Generating Config Parameters:")
	EXECUTE_PROCESS(COMMAND
		${PYTHON_EXECUTABLE} "config/genconfig.py"
		"--metadata=config"
		"--output=${PROJECT_BINARY_DIR}/src/duk_config.h"
		"autodetect-header"
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		OUTPUT_FILE genconfig.txt
		OUTPUT_VARIABLE GEN_BUILD_PARAMS_DATA
	)
endif()	

if(NOT EXISTS "${PROJECT_BINARY_DIR}/buildparams.json")
	message(STATUS "Generating Build Parameters:")
	EXECUTE_PROCESS(COMMAND
		${PYTHON_EXECUTABLE} "src/genbuildparams.py"
		"--version=${DUKTAPE_COMPACT_VERSION}"
		"--git-describe=${GIT_DESCRIBE}"
		"--out-json=buildparams.json"
		"--out-header=src/duk_buildparams.h"
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		OUTPUT_FILE genbuildparms.txt
		OUTPUT_VARIABLE GEN_BUILD_PARAMS_DATA
	)
endif()
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/src/duk_builtins.c")
	message(STATUS "Generating Built-ins:")		
	EXECUTE_PROCESS(COMMAND
		${PYTHON_EXECUTABLE} "src/genbuiltins.py"
		"--buildinfo=buildparams.json"
		"--initjs-data=src/duk_initjs.js"
		"--out-header=src/duk_builtins.h"
		"--out-source=src/duk_builtins.c"
		"--out-metadata-json=duk_build_meta.json"
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		OUTPUT_FILE genbuiltins.txt
		OUTPUT_VARIABLE GEN_BUILD_PARAMS_DATA
		)	
endif()


# Prepare License.txt for inclusion.
EXECUTE_PROCESS(COMMAND
	${PYTHON_EXECUTABLE} "util/make_ascii.py"
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE LICENSE_TXT_FILE
    INPUT_FILE "license.txt"
)

string(REGEX REPLACE "\n" "\n *  " LICENSE_TXT ${LICENSE_TXT_FILE})
set(LICENSE_TXT "/*\n *  ${LICENSE_TXT} */\n")

# Prepare AUTHORS.rst for inclusion.
EXECUTE_PROCESS(COMMAND
	${PYTHON_EXECUTABLE} "util/make_ascii.py"
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE AUTHORS_RST_FILE
    INPUT_FILE "AUTHORS.rst"
)
string(REGEX REPLACE "\n" "\n *  " AUTHORS_RST ${AUTHORS_RST_FILE})
set(AUTHORS_RST "/*\n *  ${AUTHORS_RST}\n */\n")


configure_file(src/duk_api_public.h.in duk_api_public.h)
configure_file(src/duk_dblunion.h.in duk_dblunion.h)
file(READ "${PROJECT_BINARY_DIR}/duk_api_public.h" DUK_API_PUBLIC_H)
file(READ "${PROJECT_BINARY_DIR}/duk_dblunion.h" DUK_DBLUNION_H)
#configure_file(src/duk_config.h.cmake.in src/duk_config.h)
configure_file(src/duktape.h.in src/duktape.h)