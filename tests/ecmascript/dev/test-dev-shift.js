/*===
1
2
4
1073741824
-2147483648
1
===*/

print(1 << 0);
print(1 << 1);
print(1 << 2);
print(1 << 30);
print(1 << 31);  /* becomes signed */
print(1 << 32);  /* shiftcount is masked with 0x1f, so this is the same as 1 << 0 */

/*===
256
128
64
256
0
-1
-1
-1
===*/

print(256 >> 0);
print(256 >> 1);
print(256 >> 2);
print(256 >> 32);  /* == 256 >> 0 */
print(4294967296 >> 0);  /* ToInt32() -> 0 */
print(4294967295 >> 0);  /* ToInt32() -> -1 */
print(4294967295 >> 1);  /* -1 >> N is still -1 */
print(4294967295 >> 10);

/*===
256
128
64
256
0
4294967295
2147483647
4194303
===*/

print(256 >>> 0);
print(256 >>> 1);
print(256 >>> 2);
print(256 >>> 32);  /* == 256 >> 0 */
print(4294967296 >>> 0);  /* ToUint32() -> 0 */
print(4294967295 >>> 0);  /* ToUint32() -> 4294967295 */
print(4294967295 >>> 1);
print(4294967295 >>> 10);

/*===
32
===*/

/* Addition binds more tightly, thus: 1 << 2 + 3 === 1 << 5 === 32 */

print(1 << 2 + 3);

/*===
64
===*/

/* Shifting is left associative, thus: 1 << 2 << 4 === (1 << 2) << 4
 * === 4 << 4 === 64, and not 1 << (2 << 4) === 1 (!).
 */

print(1 << 2 << 4);
