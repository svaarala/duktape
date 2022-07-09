/*---
duktape_config:
  DUK_USE_VERBOSE_ERRORS: true
  DUK_USE_PARANOID_ERRORS: true
---*/

/*===
TypeError: cannot write property of null
===*/

try {
    (null).foo = 123;
} catch (e) {
    print(String(e));
}
