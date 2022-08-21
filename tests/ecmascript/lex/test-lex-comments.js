/*
 *  Comment lexer test.
 */

/*===
===*/

/*
print("in multi-line comment 1");
*/

/*===
after multi-line comment 2
===*/

/*
print("in multi-line comment 2");
*/print("after multi-line comment 2");

/*===
before multi-line comment 3
===*/

print("before multi-line comment 3");/*print("in multi-line comment 3");
*/

/*===
after multi-line comment 4
===*/

/*print("in multi-line comment 4");*/print("after multi-line comment 4");

/*===
===*/

<!--print("in html open comment 1");

/*===
before html open comment 2
after html open comment 2
===*/

print("before html open comment 2")<!--print("in html open comment 2");
print("after html open comment 2");

/*===
before html open comment 3
after html open comment 3
===*/

print("before html open comment 3")<!-- -->print("in html open comment 3");
print("after html open comment 3");

/*===
after html open comment 4
===*/

<!--
print("after html open comment 4");

/*===
===*/

/*
  print("in multi-line comment 5");
  See: https://github.com/whatwg/javascript/issues/13
*/-->print("in html close comment 1")

/*===
after html close comment 2
===*/

/*
  print("in multi-line comment 6");
*/  -->print("in html close comment 2")
print("after html close comment 2")

/*===
after html close comment 3
===*/

-->print("in html close comment 3")
print("after html close comment 3")

/*===
after html close comment 4
===*/

    -->print("in html close comment 4")
print("after html close comment 4")
