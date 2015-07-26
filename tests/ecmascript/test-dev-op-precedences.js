/*
 *  Various operator precedence and associativity tests.
 */

/*===
true
false
===*/

/* equality operators are left associative */

/*     1 == 2 === false
   <=> (1 == 2) === false
   <=> false === false
   <=> true
*/

print( 1 == 2 === false );

/*     1 == (2 === false)
   <=> 1 == false
   <=> false
*/

print( 1 == (2 === false) );
