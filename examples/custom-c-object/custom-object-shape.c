#include "custom-object.h"
#include <string.h>
#include <stdio.h>

/* These Functions usually will go in a Header */
duk_ret_t duk_class_shape_constructor(duk_context *ctx);
duk_ret_t duk_class_shape_finalizer(duk_context *ctx);
duk_ret_t duk_class_shape_func_move(duk_context *ctx);
duk_ret_t duk_class_shape_func_toString(duk_context *ctx);
duk_ret_t duk_class_shape_staticFunc(duk_context *ctx);
duk_ret_t  duk_isShape(duk_context *ctx);

duk_ret_t  duk_register_shape(duk_context *ctx)
{
	duk_idx_t object_id;
	duk_push_global_object(ctx);
	printf("Registering: Shape\n");
	duk_push_c_function(ctx, duk_class_shape_constructor, 3);
	object_id = duk_push_object(ctx);
	
	/* All Functions must be defined at the prototype level */
	/* move Function  - C Function */
	duk_put_prop_string(ctx, -2, "prototype"); 
	duk_put_prop_string(ctx, -2, "Shape");

	/* Add isShape global function */
	duk_push_c_function(ctx, duk_isShape, 3);
	duk_put_prop_string(ctx, -2, "isShape");
	duk_pop(ctx);	
	return 0;
}

duk_ret_t duk_class_shape_constructor(duk_context *ctx)
{
	int id = duk_require_int(ctx, 0);
	int x = duk_require_int(ctx, 1);
	int y = duk_require_int(ctx, 2);
	duk_idx_t obj_idx;
	duk_bool_t calledByConstructor = duk_is_constructor_call(ctx);
	if (!calledByConstructor)
	{
		/* Not Called my new operator
		 */

		/* Create a new instance by duplicating args and calling again */
		duk_push_global_object(ctx);
		duk_get_prop_string(ctx, -1, "Shape");
		duk_dup(ctx, -1);
		duk_dup(ctx, 0);
		duk_dup(ctx, 1);
		duk_dup(ctx, 2);
		duk_new(ctx, 3);  /* generate Object */
		return 1;
	}
	else {
		duk_push_this(ctx); /* Object Already Created by new operator.*/
		obj_idx = duk_require_normalize_index(ctx, -1);
		/* This is the Wrong place to put a Finalizer.*/
		duk_push_c_function(ctx, duk_class_shape_finalizer, 1);
		duk_set_finalizer(ctx, obj_idx);
	} 

	duk_push_c_function(ctx, duk_class_shape_func_move, 2 /*nargs*/);
	duk_put_prop_string(ctx, obj_idx, "move");

	duk_push_string(ctx, "function () { return \"Shape (\" + this.id + \") located at (\"+ this.x +\",\"+this.y+\")\"; }");
	duk_push_string(ctx, "duk_class_shape_toString"); /* This does not Really matter */
	duk_compile(ctx, DUK_COMPILE_FUNCTION);
	duk_put_prop_string(ctx, obj_idx, "toString");

	/* add Properties*/
	duk_push_int(ctx, id);
	duk_put_prop_string(ctx, obj_idx, "id");
	duk_push_int(ctx, x);
	duk_put_prop_string(ctx, obj_idx, "x");
	duk_push_int(ctx, y);
	duk_put_prop_string(ctx, obj_idx, "y");
	if (calledByConstructor)
	{
		return 0; // Object already exists.
	} else {
		return 1; // Object created by this function.
	}
}
duk_ret_t duk_class_shape_finalizer(duk_context *ctx)
{
	if (duk_is_constructor_call(ctx))
	{
		printf("Shape: Finalizer\n");
	}
	/* Take care of New object Clean up Here */
	return 0;
}
duk_ret_t duk_class_shape_staticFunc(duk_context *ctx)
{
	printf("Shape: Static Function\n");
	/* Do something that is normally done in static functions */
	return 0;
}

duk_ret_t duk_class_shape_func_move(duk_context *ctx)
{
	duk_idx_t idx = 0, object_id;
	/* Replace the Old Values to "Move " to a new location */
	duk_push_this(ctx);
	object_id = duk_require_normalize_index(ctx, -1);

	/* Update X cordinate */
	duk_get_prop_string(ctx, -1, "x");
	idx = duk_require_normalize_index(ctx, -1);
	duk_dup(ctx, 0);
	duk_replace(ctx, idx);
	duk_put_prop_string(ctx, object_id, "x");

	/* Update Y cordinate */
	duk_get_prop_string(ctx, -1, "y");
	idx = duk_require_normalize_index(ctx, -1);
	duk_dup(ctx, 1);
	duk_replace(ctx, idx);
	duk_put_prop_string(ctx, object_id, "y");

	return 0;
}

duk_ret_t duk_class_shape_func_toString(duk_context *ctx)
{
	duk_push_this(ctx);
	/* Re-Add constructor*/
	duk_push_string(ctx, "function () { return \"Shape (\" + this.id + \") located at (\"+ this.x +\",\"+this.y+\")\"; }");
	duk_push_string(ctx, "duk_class_shape_toString"); /* This does not Really matter */
	duk_compile(ctx, DUK_COMPILE_FUNCTION);
	duk_put_prop_string(ctx, -2, "toString");
	duk_get_prop_string(ctx, 0, "toString");
	duk_push_this(ctx);
	duk_push_this(ctx);
	duk_call(ctx, 1);

	return 1; /* Do nothing.*/
}

duk_ret_t  duk_isShape(duk_context *ctx)
{
	//duk_require_object_coercible(ctx, 0);
	duk_push_global_object(ctx);
	duk_get_prop_string(ctx, -1, "Shape");
	if (duk_is_object(ctx, 0)) {
		if (duk_instanceof(ctx, 0, -1)) {
			/* Argument is a Shape.*/
			duk_push_true(ctx);
		} else{
			/* Argument is not a Shape */
			duk_push_false(ctx);
		}
	} else {
		/* default to False.*/
		duk_push_false(ctx);
	}

	return 1;
}
