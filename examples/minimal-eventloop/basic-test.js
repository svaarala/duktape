/*
 *  A few basic tests
 */

var count = 0;
var intervalId;

setTimeout(function (x) { console.log('timer 1', x); }, 1234, 'foo');
setTimeout('console.log("timer 2");', 4321);
setTimeout(function () { console.log('timer 3'); }, 2345);
intervalId = setInterval(function (x, y) {
    console.log('interval', ++count, x, y);
    if (count >= 10) {
        clearInterval(intervalId);
    }
}, 400, 'foo', 'bar');
