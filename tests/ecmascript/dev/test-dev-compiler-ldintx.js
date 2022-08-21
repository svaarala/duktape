/*
 *  Code which exercises integer loading using LDINT, LDINT+LDINTX, and a
 *  double constant.
 */

/*===
-131072
131071
-2147483648
2147483647
-2147483649
-10000000000000
2147483648
10000000000000
===*/

// LDINT.
print(-131072);      // smallest LDINT
print(131071);       // largest LDINT

// LDINT+LDINTX.  At the moment all signed 32-bit values are emitted as
// LDINT+LDINTX although LDINT+LDINTX could emit larger values (36-bit signed).
print(-2147483648);  // smallest LDINTX
print(2147483647);   // largest LDINTX

// Double constants.
print(-2147483649);
print(-10000000000000);
print(2147483648);
print(10000000000000);
