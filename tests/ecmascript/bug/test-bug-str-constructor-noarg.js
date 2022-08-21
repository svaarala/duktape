/*===

undefined

undefined
===*/

print(String());           // required to result in empty string: ""
print(String(undefined));  // required to result in "undefined"

print(new String().toString());           // required to result in empty string: ""
print(new String(undefined).toString());  // required to result in "undefined"
