/*
 *  Example of how to use DUK_USE_CPP_EXCEPTIONS to support automatic
 *  variables (e.g. destructor calls) in Duktape/C functions.
 *
 *  Compile with -DDUK_OPT_CPP_EXCEPTIONS:
 *
 *    $ g++ -otest -DDUK_OPT_CPP_EXCEPTIONS -I<duktape_dist>/src/ \
 *      <duktape_dist>/src/duktape.c cpp_exceptions.cpp -lm
 *
 *  or ensure duk_config.h has DUK_USE_CPP_EXCEPTIONS enabled using
 *  genconfig.  When executed you should see something like:
 *
 *    $ ./test
 *    my_class instance created
 *    my_class instance destroyed      <== destructor gets called
 *    --> rc=1 (SyntaxError: parse error (line 1))
 *    [...]
 *
 *  Duktape uses a custom exception class (duk_internal_exception) which
 *  doesn't inherit from any base class, so that catching any base classes
 *  in user code won't accidentally catch exceptions thrown by Duktape.
 */

#if !defined(__cplusplus)
#error compile using a c++ compiler
#endif

#include <stdio.h>
#include <exception>
#include "duktape.h"

#if defined(__cplusplus) && (__cplusplus >= 201103L)
#define NOEXCEPT noexcept
#else
#define NOEXCEPT throw()
#endif

/*
 *  Example class with a destructor
 */

class my_class {
 public:
	my_class();
	~my_class();
};

my_class::my_class() {
	printf("my_class instance created\n");
}

my_class::~my_class() {
	printf("my_class instance destroyed\n");
}

/*
 *  SyntaxError caused by eval exits Duktape/C function but destructors
 *  are executed.
 */

duk_ret_t test1(duk_context *ctx) {
	my_class myclass;

	duk_eval_string(ctx, "aiee=");

	return 0;
}

/*
 *  You can use C++ exceptions inside Duktape/C functions for your own
 *  purposes but you should catch them before they propagate to Duktape.
 */

duk_ret_t test2(duk_context *ctx) {
	my_class myclass;

	try {
		throw 123;
	} catch (int myvalue) {
		printf("Caught: %d\n", myvalue);
	}

	return 0;
}

/*
 *  If you let your own C++ exceptions propagate out of a Duktape/C function
 *  it will be caught by Duktape and considered a programming error.  Duktape
 *  will catch the exception and convert it to a Duktape error.
 *
 *  This may be allowed in a later version once all the implications have been
 *  worked out.
 */

duk_ret_t test3(duk_context *ctx) {
	my_class myclass;

	throw 123;  /* ERROR: exception propagated to Duktape */

	return 0;
}

/*
 *  Same as above, but if the exception inherits from std::exception, it's
 *  "what()" will be included in the error message.
 */

class my_exception : public std::exception {
	virtual const char *what() const NOEXCEPT {
		return "my_exception";
	}
};

duk_ret_t test4(duk_context *ctx) {
	my_class myclass;
	my_exception myexc;

	throw myexc;  /* ERROR: exception propagated to Duktape */

	return 0;
}

/*
 *  Same as above, but if the exception inherits from std::exception with
 *  a NULL what().  Duktape will describe the error as 'unknown' if so.
 */

class my_exception2 : public std::exception {
	virtual const char *what() const NOEXCEPT {
		return NULL;
	}
};

duk_ret_t test5(duk_context *ctx) {
	my_class myclass;
	my_exception2 myexc;

	throw myexc;  /* ERROR: exception propagated to Duktape */

	return 0;
}

int main(int argc, char *argv[]) {
	duk_context *ctx = duk_create_heap_default();
	duk_int_t rc;

	(void) argc; (void) argv;  /* suppress warning */

	printf("*** test1 - duk_pcall()\n");
	duk_push_c_function(ctx, test1, 0);
	rc = duk_pcall(ctx, 0);
	printf("--> rc=%ld (%s)\n", (long) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	printf("\n");

	printf("*** test1 - duk_safe_call()\n");
	rc = duk_safe_call(ctx, test1, 0, 1);
	printf("--> rc=%ld (%s)\n", (long) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	printf("\n");

	printf("*** test1 - ecmascript try-catch\n");
	duk_push_c_function(ctx, test1, 0);
	duk_put_global_string(ctx, "test1");
	duk_eval_string_noresult(ctx,
		"try {\n"
		"    test1();\n"
		"} catch (e) {\n"
		"    print(e.stack || e);\n"
		"}\n");
	printf("\n");

	printf("*** test2 - duk_pcall()\n");
	duk_push_c_function(ctx, test2, 0);
	rc = duk_pcall(ctx, 0);
	printf("--> rc=%ld (%s)\n", (long) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	printf("\n");

	printf("*** test2 - duk_safe_call()\n");
	rc = duk_safe_call(ctx, test2, 0, 1);
	printf("--> rc=%ld (%s)\n", (long) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	printf("\n");

	printf("*** test2 - ecmascript try-catch\n");
	duk_push_c_function(ctx, test2, 0);
	duk_put_global_string(ctx, "test2");
	duk_eval_string_noresult(ctx,
		"try {\n"
		"    test2();\n"
		"} catch (e) {\n"
		"    print(e.stack || e);\n"
		"}\n");
	printf("\n");

	printf("*** test3 - duk_pcall()\n");
	duk_push_c_function(ctx, test3, 0);
	rc = duk_pcall(ctx, 0);
	printf("--> rc=%ld (%s)\n", (long) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	printf("\n");

	printf("*** test3 - duk_safe_call()\n");
	rc = duk_safe_call(ctx, test3, 0, 1);
	printf("--> rc=%ld (%s)\n", (long) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	printf("\n");

	printf("*** test3 - ecmascript try-catch\n");
	duk_push_c_function(ctx, test3, 0);
	duk_put_global_string(ctx, "test3");
	duk_eval_string_noresult(ctx,
		"try {\n"
		"    test3();\n"
		"} catch (e) {\n"
		"    print(e.stack || e);\n"
		"}\n");
	printf("\n");

	printf("*** test4 - duk_pcall()\n");
	duk_push_c_function(ctx, test4, 0);
	rc = duk_pcall(ctx, 0);
	printf("--> rc=%ld (%s)\n", (long) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	printf("\n");

	printf("*** test4 - duk_safe_call()\n");
	rc = duk_safe_call(ctx, test4, 0, 1);
	printf("--> rc=%ld (%s)\n", (long) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	printf("\n");

	printf("*** test4 - ecmascript try-catch\n");
	duk_push_c_function(ctx, test4, 0);
	duk_put_global_string(ctx, "test4");
	duk_eval_string_noresult(ctx,
		"try {\n"
		"    test4();\n"
		"} catch (e) {\n"
		"    print(e.stack || e);\n"
		"}\n");
	printf("\n");

	printf("*** test5 - duk_pcall()\n");
	duk_push_c_function(ctx, test5, 0);
	rc = duk_pcall(ctx, 0);
	printf("--> rc=%ld (%s)\n", (long) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	printf("\n");

	printf("*** test5 - duk_safe_call()\n");
	rc = duk_safe_call(ctx, test5, 0, 1);
	printf("--> rc=%ld (%s)\n", (long) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	printf("\n");

	printf("*** test5 - ecmascript try-catch\n");
	duk_push_c_function(ctx, test5, 0);
	duk_put_global_string(ctx, "test5");
	duk_eval_string_noresult(ctx,
		"try {\n"
		"    test5();\n"
		"} catch (e) {\n"
		"    print(e.stack || e);\n"
		"}\n");
	printf("\n");

	printf("*** done\n");

	duk_destroy_heap(ctx);

	return 0;
}
