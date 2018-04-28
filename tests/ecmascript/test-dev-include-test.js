/*
 *  Test the include mechanism - not an actual ECMAScript testcase.
 */

/*===
Hello world!
===*/

/*@include util-helloworld.js@*/

try {
    testHelloWorldUtility();
} catch (e) {
    print(e.stack || e);
}
