print("Testing Create:");
var myVector = new vector2D();
/* This Should cause the finalizer to run */
Duktape.fin(myVector);
myVector = new vector2D();
var myVectorA = new vector2D(1.0, 2.0); 
var myVectorEX = new vector2D(myVectorA); 
print("Create OK");
// All of these print the same thing, but it shows that the c Functions are tied correctly.
print("myVector = " + myVector);
print("myVector.toString() = " + myVector.toString());
printTest(myVector);
print("myVectorEX:");
printTest(myVectorEX);
print("myVectorA:");
printTest(myVectora);
var myVectorB = myVector; // Test Cloning 
print("myVectorB:");
printTest(myVectorB);

/* This shows that the Getters and Setters work */
print("Testing Getter/Setters: ");
print(myVector.x);myVector.x = 3.25;print(myVector.x);
print(myVector.y);myVector.y = 6.0;print(myVector.y);
print("myVector = " + myVector);
print("myVectorB = " + myVectorB);
delete myVector; myVector = null;
print("myVectorB = " + myVectorB);
myVectorB = null;
print("myVectorB = " + myVectorB);
/* This will cause an error since a new value for myVectorC has not been set yet. */
//print("myVectorC = " + myVectorC);

function printTest(vectorData) {
	print(vectorData);
	print(vectorData.myValue);
};