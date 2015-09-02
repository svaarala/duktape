/* Example that creates two different Objects 
 *
 * Object 1 - Shape 
 * This object shows how to construct a basic object with C functions to
 * define the function move.
 *
 * Object 2 - vector2D 
 * This object shows how to properly clean up user data in a custom C object.
 *
 */

#include "custom-object.h"


duk_context* createMyHeap()
{
    duk_context *ctx = NULL;
    /* FIXME: resource limits */
    ctx = duk_create_heap_default();
    if (!ctx) {
        fprintf(stderr, "cannot allocate heap for testcase\n");
        return NULL;
    }
    return ctx;
}

void run_pcompile_file(const char* file)
{
    duk_context *ctx = NULL;
    ctx = createMyHeap();
    if (ctx) {
        duk_register_shape(ctx);
        duk_register_Vector2D(ctx);
        printf("\n\nduk_pcompile_file Test: on %s\n", file);
        if (duk_pcompile_file(ctx, 0, "myVectors2B.js") != 0) {
            printf("compile failed: %s\n", duk_safe_to_string(ctx, -1));
        } else {
            duk_call(ctx, 0);      /* [ func ] -> [ result ] */
            printf("program result: %s\n", duk_safe_to_string(ctx, -1));
        }
        duk_destroy_heap(ctx);
    }
}

void run_peval_file(const char* file)
{
    duk_context *ctx = NULL;
    ctx = createMyHeap();
    if (ctx) {
        duk_register_shape(ctx);
        duk_register_Vector2D(ctx);
        printf("\n\nduk_peval_file_noresult Test on %s \n",file);
        if (duk_peval_file_noresult(ctx, file) != 0) {
            printf("duk_peval_file_noresult: eval failed\n");
        }
        else {
            printf("duk_peval_file_noresult: eval successful\n");
        }
        duk_destroy_heap(ctx);
    }
}

/* Shape Example Built In */
const char *ShapeExamplePartA[] = {
    "var myShape = new Shape(0,1,2);\n",
    "print(myShape);\n",
    "myShape.move(3,9);\n",
    "print(myShape.toString());\n",
    NULL };
    
const char *ShapeExamplePartB[] = {
    {"myShape.move(9,3);\n" },
    { "print(myShape);\n" },
    { "if (myShape instanceof vector2D) {\n"
      "\tprint(\"myShape is an Instance of vector2D\");\n"
      "} else {\n"
      "\tprint(\"myShape is not an instance of vector2D\");\n"
      "}\n"
     },
    /* This Next Item looks odd, but it is Valid C/C++*/
    { "if (myShape instanceof Shape) {\n" 
      "print(\"myShape is an Instance of Shape\");\n"
      "} else {\n"
      "print(\"myShape is not an instance of Shape\");\n"
      "}\n" },
      {
          "if (isVector2D(myShape)) {\n"
          "\tprint(\"myShape is 2D Vector.\");"
          "} else {\n"
          "\t print(\"myShape is not a 2D Vector.\");\n}\n"
      },
      {
          "if (isShape(myShape)) {\n"
          "\tprint(\"myShape is Shape.\");"
          "} else {\n"
          "\t print(\"myShape is not a Shape.\");\n}\n"
      },
      { "try {\n"
      "\tvar myShapeB = Shape(1,2,3); print(myShapeB);\n"
      "\tif (isShape(myShapeB)) {\n"
      "\t\tprint(\"myShapeB is a Shape\");\n"
      "\t} else {\n"
      "\t\t print(\"myShapeB is not a Shape\");\n\t}\n"
      "} catch (e) {\n"
      "\tprint(\"Failed to Create myShapeOneArg: \" + e);\n"
      "}\n" },
    NULL };

void run_Shape_example()
{
    duk_context *ctx = NULL;
    int i = 0;
    ctx = createMyHeap();
    if (ctx) {
        printf("---------------------------------------\n");
        printf("Shape Example Script:\n");
        printf("---------------------------------------\n");
        duk_register_shape(ctx);
        duk_register_Vector2D(ctx);

        for (i = 0; ShapeExamplePartA[i]; i++) {
            printf("Eval: %s\n", ShapeExamplePartA[i]);
            if (duk_peval_string_noresult(ctx, ShapeExamplePartA[i])) {
                printf("eval failed\n");
            }
        }
        /* In this part you do internal stuff.*/
        /* TODO: Add internal stuff */

        for (i = 0; ShapeExamplePartB[i]; i++) {
            printf("Eval: %s\n", ShapeExamplePartB[i]);
            if (duk_peval_string_noresult(ctx, ShapeExamplePartB[i])) {
                printf("eval failed\n");
            }
        }
        duk_destroy_heap(ctx);
    }
}

/* Vector Example Built In */
const char * VectorExample[] = {
    "myVectorT = new vector2D(1.0, 2.0);\n",
    "myVectorTX = new vector2D(myVectorT);\n",
    "print(myVectorT);\n",
    { "\ntry {\n"
      "\tvar myBadVector = vector2D(1.0, 2.0);\n"
      "} catch (e) {\n"
      "\tprint(\"Error creating myBadVector: \" + e);\n"
      "}\n" },
      {
          "if (myVectorT instanceof vector2D) {\n"
          "\tprint(\"myVectorT is an Instance of vector2D\");\n"
          "} else {\n"
          "\tprint(\"myVectorT is not an instance of vector2D\");\n"
          "}\n"
      },
      {
          "if (isShape(myVectorTX)) {\n"
          "\tprint(\"myVectorTX is a Shape\");\n"
          "} else {\n"
          "\t print(\"myVectorTX is not a Shape\");\n}\n"
      },
      {
          "if (isVector2D(myVectorTX)) {\n"
          "\tprint(\"myVectorTX is 2D Vector.\");\n"
          "} else {\n"
          "\t print(\"myVectorTX is not a 2D Vector.\");\n}\n"
      },
    /* Force the Vector Finalizer to run */
    "delete myVectorT;\n",
    "myVectorT = null;\n",
    "print(myVectorT);\n",
    /* myVectorTX still exists. */
    "print(myVectorTX);\n",
    NULL};
    
void run_Vector_example()
{
    duk_context *ctx = NULL;
    int i = 0;
    ctx = createMyHeap();
    if (ctx) {

        printf("---------------------------------------\n");
        printf("Vector Example Script:\n");
        printf("---------------------------------------\n");
        duk_register_shape(ctx);
        duk_register_Vector2D(ctx);

        /* This works */
        for (i = 0; VectorExample[i]; i++) {
            printf("Eval: %s\n", VectorExample[i]);
            if (duk_peval_string_noresult(ctx, VectorExample[i])) {
                printf("eval failed\n");
            }
        }
        duk_destroy_heap(ctx);
    }
}
int main(int argc, char *argv[]) {
    int i;
    int have_files = 0;
    int runVectorExample = 0;
    int runShapeExample = 0;
    for (i = 1; i < argc; i++) {
        char *arg = argv[i];
        if (!arg) {
            goto usage;
        }
        if (strcmp(arg, "--vectorExample") == 0) {
            runVectorExample = 1;
        } else if (strcmp(arg, "--shapeExample") == 0) {
            runShapeExample = 1;
        } else {
            have_files = 1;
        }
    }
    if (have_files) {
        for (i = 1; i < argc; i++) {
            char *arg = argv[i];
            if (!arg) {
                goto usage;
            }
            if (strcmp(arg, "--vectorExample") == 0) {
                continue;
            }
            else if (strcmp(arg, "--shapeExample") == 0) {
                continue;
            }
            else {
                run_peval_file(arg);
            }
        }
    }
    
    if (runShapeExample) {
        run_Shape_example();
    }
    if (runVectorExample) {
        run_Vector_example();
    }
    return 0;
 usage:
    fprintf(stderr, "Usage: duk [options] [<filenames>]\n"
                    "\n"
                    "   --shapeExample    Run built in shape example script.\n"
                    "   --vectorExample   Run built in vector example script.\n"
                    );
    fflush(stderr);
    exit(1);
    return 0;
}
