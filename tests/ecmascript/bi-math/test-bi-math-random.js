/*
 *  Math.random()
 */

/*@include util-number.js@*/

/*===
random
true true
true true
===*/

/* This is a statistical test, but should pass almost always. */

print('random');

var rnd_val;
var rnd_sum = 0;
var rnd_ge0 = true;
var rnd_lt0 = true;
for (i = 0; i < 1e6; i++) {
    rnd_val = Math.random();
    if (!(rnd_val >= 0)) { rnd_ge0 = false; }
    if (!(rnd_val < 1)) { rnd_lt0 = false; }
    rnd_sum += rnd_val;
}

print(rnd_ge0, rnd_lt0);
print(rnd_sum >= 0.497e6, rnd_sum <= 0.503e6);
