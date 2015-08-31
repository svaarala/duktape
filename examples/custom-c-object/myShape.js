var myShape = new Shape(0,1,2);
print(myShape);
myShape.move(3,9);
print(myShape.toString());

/* These are expected to fail */
try {
   var myShapeOneArg = new Shape(1);
} catch (e) {
   print("Failed to Create myShapeOneArg: " + e)
}
try {
   var myShapeOneArg = new Shape(1,2);
} catch (e) {
   print("Failed to Create myShapeTwoArgs: " + e)
}
try {
   var myShapeInvalidArgParmA = new Shape(myShape,0,0);
} catch (e) {
   print("Failed to Create myShapeInvalidArgParmA: " + e)
}
try {
   var myShapeInvalidArgParmB = new Shape(1,myShape,0);
} catch (e) {
   print("Failed to Create myShapeInvalidArgParmB: " + e)
}
try {
   var myShapeInvalidArgParmC = new Shape(1,0,"Random String");
} catch (e) {
   print("Failed to Create myShapeInvalidArgParmC: " + e)
}