/* Odd Bug is shown by this code:
 * duk_eval_string - This exact Code works without issues.
 * duk_peval_file_noresult - This code Fails.
 */
print ("Testing vector2D");
var myVectorScript = new vector2D(1.0, 2.0);
print("Vector Script: " myVectorScript)

/* Modify and access x value. */
myVectorScript.x = 3.25;
print(myVectorScript.x);

/* Modify and access y value. */
myVectorScript.y = 6.0;
print(myVectorScript.y);
