#include "custom-object.h"
#include <string.h>
#include <stdio.h>
// Enable User Data pointer in the object.
#define MY_USER_DATA_PROP  "\xff""vecData"

/* Custom Functions */
duk_ret_t duk_vector2d_toString(duk_context* ctx);
duk_ret_t duk_vector2d_get_y(duk_context* ctx);
duk_ret_t duk_vector2d_get_x(duk_context* ctx);
duk_ret_t duk_vector2d_set_y(duk_context* ctx);
duk_ret_t duk_vector2d_set_x(duk_context* ctx);
duk_ret_t duk_vector2d_object_builder(duk_context* ctx, duk_idx_t myThis, float x, float y, int myValue);


duk_ret_t duk_vector2d_add(duk_context* ctx);

duk_ret_t duk_vector2d_finalizer(duk_context *ctx)
{
	// TODO: Is this really working.
	vector2D* ptr = NULL;
	//duk_push_this(ctx);
	printf("finalizer begin: Vec2D\n");
	duk_get_prop_string(ctx, 0, MY_USER_DATA_PROP);
	ptr = duk_get_pointer(ctx, -1);
	duk_pop_2(ctx);
	if (ptr) {
		printf("Freed UserData.\n");
		free(ptr);
		ptr = NULL;
	}
	printf("finalizer end: Vec2D\n");
	return 1;
}

duk_ret_t duk_vector2d_constructor(duk_context* ctx)
{
	printf("Constructor begin: Vec2D\n");
	int n = duk_get_top(ctx);  /* #args */
	duk_idx_t myThis;
	double _new_vector_x = 0.0, _new_vector_y = 0.0;
	duk_bool_t calledByConstructor = duk_is_constructor_call(ctx);
	if (!calledByConstructor)
	{
		duk_push_string(ctx,"vector2D must be instantiated using new.");
		duk_throw(ctx);
		return 1;
	} else {
		duk_push_this(ctx); /* Object Already Created by new operator.*/
		myThis = duk_require_normalize_index(ctx, -1);
	}
	switch (n) {
	case 0:
	{
		duk_vector2d_object_builder(ctx, myThis, 0.0, 0.0, 12345);
		return 0;
	}
	case 1:
	{
		if (duk_is_object(ctx, 0)) {
			duk_push_global_object(ctx);
			duk_get_prop_string(ctx, -1, "vector2D");
			if (duk_instanceof(ctx, 0, -1)) {
				/* We have ourselves a vector2D object.*/

				/* Get the User Data of the Object to be Cloned.*/
				vector2D* ptr = NULL;
				duk_get_prop_string(ctx, 0, MY_USER_DATA_PROP);
				ptr = duk_get_pointer(ctx, -1);
				duk_pop(ctx);
				if (ptr) {
					duk_vector2d_object_builder(ctx, myThis, ptr->x, ptr->y, 54321);
					return 0;
				}
			} else{
				duk_push_string(ctx, "vector2D: Argument must be an vector2D object.");
				duk_throw(ctx);
			}
			return 0;
		} else {
			duk_push_string(ctx, "vector2D: Argument must be an object.");
			duk_throw(ctx);
			printf("Constructor end: Vec2D (ERROR)\n");
			return 0;
		}
	}
	case 2:
	{
		if (!duk_is_number(ctx, 0) || !duk_is_number(ctx, 1)) {
			duk_push_string(ctx, "vector2D: Both Arguments Must be numbers.");
			duk_throw(ctx);
			return 0;
		} else {
			_new_vector_x = duk_get_number(ctx, 0);
			_new_vector_y = duk_get_number(ctx, 1);
			duk_vector2d_object_builder(ctx, myThis, (float)_new_vector_x, (float)_new_vector_y, 54321);
			return 0;
		}
	}
	default:
		duk_push_string(ctx, "vector2D: Incorrect Number of Arguments.");
		duk_throw(ctx);
		return 0;
	}

	duk_push_string(ctx, "vector2D: Unexpected Error.");
	duk_throw(ctx);
	return 1;

}


duk_ret_t duk_register_Vector2D(duk_context *ctx) {
	duk_idx_t obj_index;
	duk_push_global_object(ctx);
	printf("Registering: Vector2D\n");
	/* Register vector2D object Constructor(s).*/
	// NOTE: if you want multiple constructors, this is the only way to go.
	duk_push_c_function(ctx, duk_vector2d_constructor, DUK_VARARGS);
	obj_index = duk_push_object(ctx);			
	duk_put_prop_string(ctx, -2, "prototype"); 
	duk_put_prop_string(ctx, -2, "vector2D");
	duk_pop(ctx);

	return 0;
}





duk_ret_t duk_vector2d_toString(duk_context* ctx) {
  vector2D* ptr = NULL;
   // The only print out on this functions hould never exceed 64 characters.
  char outputBuffer[64];
  duk_push_this(ctx);
  duk_get_prop_string(ctx, -1, MY_USER_DATA_PROP);
  ptr = duk_get_pointer(ctx, -1);
  duk_pop_2(ctx);
  if (ptr) {
	  sprintf(outputBuffer, "vector2D( %9.9f , %9.9f )", ptr->x, ptr->y);
	  duk_push_string(ctx, outputBuffer);
  } else {
	  duk_push_string(ctx, "vector2D().toString()");
  }
  return 1;
}

duk_ret_t duk_vector2d_get_x(duk_context* ctx) {
  duk_push_this(ctx);
  duk_get_prop_string(ctx, -1, MY_USER_DATA_PROP);
  vector2D* ptr = duk_get_pointer(ctx, -1);
  duk_pop_2(ctx);
  if (ptr) {
	  // do stuff with ptr
	  duk_push_number(ctx, ptr->x);
  }  else {
	  printf("duk_vector2d_get_x\n");
	  duk_push_null(ctx);
  }
  return 1;  /* one return value */
}

duk_ret_t duk_vector2d_get_y(duk_context* ctx) {
  duk_push_this(ctx);
  duk_get_prop_string(ctx, -1, MY_USER_DATA_PROP);
  vector2D* ptr = duk_get_pointer(ctx, -1);
  duk_pop_2(ctx);
  // do stuff with ptr
  if (ptr) {
	  // do stuff with ptr
	  duk_push_number(ctx, ptr->y);
  }  else {
	  printf("duk_vector2d_get_y\n");
	  duk_push_null(ctx);
  }
  return 1;  /* one return value */
}
duk_ret_t duk_vector2d_set_x(duk_context* ctx) {
  duk_push_this(ctx);
  duk_get_prop_string(ctx, -1, MY_USER_DATA_PROP);
  vector2D* ptr = duk_get_pointer(ctx, -1);
  duk_pop_2(ctx);
  if (ptr) {
	  // Value Set.
	  ptr->x = (float)duk_to_number(ctx, 0);
  } else { 
	  printf("duk_vector2d_set_x\n");
  }
  // do stuff with ptr
  return 0;  /* one return value */
}

duk_ret_t duk_vector2d_set_y(duk_context* ctx) {
  duk_push_this(ctx);
  duk_get_prop_string(ctx, -1, MY_USER_DATA_PROP);
  vector2D* ptr = duk_get_pointer(ctx, -1);
  duk_pop_2(ctx);
  // do stuff with ptr
  if (ptr) {
	  // Value Set.
	  ptr->y = (float)duk_to_number(ctx, 0);
  }  else {
	  printf("duk_vector2d_set_y\n");
  }
  return 0;  /* one return value */
}


duk_ret_t duk_vector2d_object_builder(duk_context* ctx, duk_idx_t myThis, float x, float y, int myValue)
{
	vector2D* ptr = NULL;
	//duk_pop(ctx);
	/* Set the Finalizer First */
	duk_push_c_function(ctx, duk_vector2d_finalizer, 1 /*nargs*/);
	duk_set_finalizer(ctx, myThis);

	/* Add a few User Functions*/
	duk_push_c_function(ctx, duk_vector2d_toString, 1);   /* toString (function) */
	duk_put_prop_string(ctx, myThis, "toString");

	/* Add a few Properties.*/
	duk_push_int(ctx, myValue);
	duk_put_prop_string(ctx, myThis, "myValue");



	/* Add Some User Data*/

	ptr = calloc(1, sizeof(vector2D));
	ptr->x = x;
	ptr->y = y;
	duk_push_pointer(ctx, ptr);
	duk_put_prop_string(ctx, myThis, MY_USER_DATA_PROP);

	/* Add some Getter/setter functions to modify User Data.*/
	duk_push_string(ctx, "y");
	duk_push_c_function(ctx, duk_vector2d_get_y, 0 /*nargs*/);
	duk_push_c_function(ctx, duk_vector2d_set_y, 1 /*nargs*/);
	duk_def_prop(ctx, myThis, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

	duk_push_string(ctx, "x");
	duk_push_c_function(ctx, duk_vector2d_get_x, 0 /*nargs*/);
	duk_push_c_function(ctx, duk_vector2d_set_x, 1 /*nargs*/);
	duk_def_prop(ctx, myThis, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

	
	return 1;
}
