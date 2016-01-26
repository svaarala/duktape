/*
 *  C++ 'static const' works differently from C 'static const'.
 *  This example compiles with gcc but not with g++.  G++ will
 *  issue errors.
 */

/*
$ g++ -o /tmp/foo cxx_static_const.c -lm

cxx_static_const.c:2:26: error: uninitialized const ‘N1’ [-fpermissive]
 static const struct node N1;
                          ^
cxx_static_const.c:1:21: note: ‘const struct node’ has no user-provided default constructor
 struct node; struct node { const struct node *other; };
                     ^
cxx_static_const.c:1:47: note: and the implicitly-defined constructor does not initialize ‘const node* node::other’
 struct node; struct node { const struct node *other; };
                                               ^
cxx_static_const.c:3:26: error: uninitialized const ‘N2’ [-fpermissive]
 static const struct node N2;
                          ^
cxx_static_const.c:1:21: note: ‘const struct node’ has no user-provided default constructor
 struct node; struct node { const struct node *other; };
                     ^
cxx_static_const.c:1:47: note: and the implicitly-defined constructor does not initialize ‘const node* node::other’
 struct node; struct node { const struct node *other; };
                                               ^
cxx_static_const.c:4:26: error: redefinition of ‘const node N1’
 static const struct node N1 = { &N2 };
                          ^
cxx_static_const.c:2:26: error: ‘const node N1’ previously declared here
 static const struct node N1;
                          ^
cxx_static_const.c:5:26: error: redefinition of ‘const node N2’
 static const struct node N2 = { &N1 };
                          ^
cxx_static_const.c:3:26: error: ‘const node N2’ previously declared here
 static const struct node N2;
*/

struct node; struct node { const struct node *other; };
static const struct node N1;
static const struct node N2;
static const struct node N1 = { &N2 };
static const struct node N2 = { &N1 };

int main(int argc, char *argv[]) {
	(void) argc; (void) argv; return 0;
}
