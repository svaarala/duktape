/*
 *  Example test.
 *
 *  Expected result is delimited as follows; the expected response
 *  here is "hello world\n".  Multiple expected blocks may exist,
 *  they are simply concatenated.  This allows tests to be written
 *  in segments.
 */

/*---
{
    "comment": "a single metadata block may exist; it is formatted as a JSON object"
}
---*/

/*===
hello world
===*/

if (1) {
    print("hello world");   /* automatic newline */
} else {
    print("not quite");
}

/*===
another
===*/

print("another");
