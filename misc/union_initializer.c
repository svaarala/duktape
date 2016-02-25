/*
 *  Union initializers are part of C99 but not C++.
 *
 *  Embed the test union inside a struct because there was an issue with
 *  that in VS2013 and union-inside-struct is needed by API macros.
 */

#include <stdio.h>

typedef union {
	int a;
	double b;
	const char *c;
} test_union;

typedef struct {
	int etype;  /* 0=int, 1=double, 2=string */
	test_union u;
} test_struct;

static void dump_struct(test_struct *t) {
	switch (t->etype) {
	case 0:
		printf("Integer: %d\n", t->u.a);
		break;
	case 1:
		printf("Double: %lf\n", t->u.b);
		break;
	case 2:
		printf("String: %s\n", t->u.c);
		break;
	default:
		printf("Unknown type: %d\n", t->etype);
	}
}

int main(int argc, char *argv[]) {
	test_struct v1 = { 0, 123 };  /* default is to initialize first alternative */
	test_struct v2 = { 0, { .a = 234 } };
	test_struct v3 = { 1, { .b = 123.456 } };
	test_struct v4 = { 2, { .c = "foo" } };

	dump_struct(&v1);
	dump_struct(&v2);
	dump_struct(&v3);
	dump_struct(&v4);
}
