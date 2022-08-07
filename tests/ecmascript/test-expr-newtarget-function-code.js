/*===
undefined
function
===*/

function test() {
    print(typeof new.target);
}

test();
new test();
