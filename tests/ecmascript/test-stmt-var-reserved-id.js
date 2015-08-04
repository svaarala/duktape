/*
 *  VariableDeclaration production accepts Identifier as the variable name.
 *  Identifier accepts "IdentifierName but not ReservedWord".
 *
 *  This means that reserved words (keywords, future reserved words, null
 *  literal, and boolean literal) must be rejected in both strict and
 *  non-strict mode.  In strict mode the set of reserved words contains
 *  more future reserved words.
 *
 *  Note that not all tokens are reserved words.  For example, 'set' and 'get'
 *  are acceptable identifier names, e.g. 'var set = 1;' is a valid statement.
 *  Such statements also appear in the wild, e.g. in qunit.js.
 */

/*===
=== non-strict ===
break SyntaxError
case SyntaxError
catch SyntaxError
class SyntaxError
const SyntaxError
continue SyntaxError
debugger SyntaxError
default SyntaxError
delete SyntaxError
do SyntaxError
else SyntaxError
enum SyntaxError
export SyntaxError
extends SyntaxError
false SyntaxError
finally SyntaxError
for SyntaxError
function SyntaxError
get function
if SyntaxError
implements function
import SyntaxError
in SyntaxError
instanceof SyntaxError
interface function
let function
new SyntaxError
null SyntaxError
package function
private function
protected function
public function
return SyntaxError
set function
static function
super SyntaxError
switch SyntaxError
this SyntaxError
throw SyntaxError
true SyntaxError
try SyntaxError
typeof SyntaxError
var SyntaxError
void SyntaxError
while SyntaxError
with SyntaxError
yield function
===*/

/*===
=== strict ===
break SyntaxError
case SyntaxError
catch SyntaxError
class SyntaxError
const SyntaxError
continue SyntaxError
debugger SyntaxError
default SyntaxError
delete SyntaxError
do SyntaxError
else SyntaxError
enum SyntaxError
export SyntaxError
extends SyntaxError
false SyntaxError
finally SyntaxError
for SyntaxError
function SyntaxError
get function
if SyntaxError
implements SyntaxError
import SyntaxError
in SyntaxError
instanceof SyntaxError
interface SyntaxError
let SyntaxError
new SyntaxError
null SyntaxError
package SyntaxError
private SyntaxError
protected SyntaxError
public SyntaxError
return SyntaxError
set function
static SyntaxError
super SyntaxError
switch SyntaxError
this SyntaxError
throw SyntaxError
true SyntaxError
try SyntaxError
typeof SyntaxError
var SyntaxError
void SyntaxError
while SyntaxError
with SyntaxError
yield SyntaxError
===*/

var keywords = "break do instanceof typeof case else new var catch finally return void continue for switch while debugger function this with default if throw delete in try";
var future1 = "class enum extends super const export import";
var future2 = "implements interface yield let package private protected public static";
var other = "null true false";
var accepted = "set get";  // these must be accepted!
var names = (keywords + " " + future1 + " " + future2 + " " + other + " " + accepted).split(' ');
names.sort();

function reservedWordNonStrictTest() {
    names.forEach(function(n) {
        var code = '(function () { var ' + n + ' = 123; })';
        try {
            var res = eval(code);
            print(n, typeof res);
        } catch (e) {
            print(n, e.name);
        }
    });
}

function reservedWordStrictTest() {
    names.sort();
    names.forEach(function(n) {
        var code = '(function () { "use strict"; var ' + n + ' = 123; })';
        try {
            var res = eval(code);
            print(n, typeof res);
        } catch (e) {
            print(n, e.name);
        }
    });
}

print('=== non-strict ===');
try {
    reservedWordNonStrictTest();
} catch (e) {
    print(e);
}

print('=== strict ===');
try {
    reservedWordStrictTest();
} catch (e) {
    print(e);
}
