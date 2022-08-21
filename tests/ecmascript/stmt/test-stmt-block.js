/*
 *  Block statement (E5 Section 12.1).
 */

/*===
before
inside 1
inside 2
after
===*/

print('before');
{
    print('inside 1');
    print('inside 2');
}
print('after');
