/*
 *  Date.prototype.toGMTString is required to have the same Function
 *  object as Date.prototype.toUTCString in E5 Section B.2.6.
 *
 *  Note that while Smjs respects this, V8 does not (the Function
 *  objects are distinct).
 */

/*===
true
===*/

print(Date.prototype.toGMTString === Date.prototype.toUTCString);
