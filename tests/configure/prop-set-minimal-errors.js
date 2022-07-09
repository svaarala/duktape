/*---
duktape_config:
  DUK_USE_VERBOSE_ERRORS: false
  DUK_USE_PARANOID_ERRORS: false
---*/

/*===
TypeError: 6
===*/

try {
    (null).foo = 123;
} catch (e) {
    print(String(e));
}
