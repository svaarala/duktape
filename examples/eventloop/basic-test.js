/*
 *  A few basic tests
 */

var count = 0;
var intervalId;

setTimeout(function () { print('timer 1'); }, 1234);
setTimeout('print("timer 2");', 4321);
setTimeout(function () { print('timer 3'); }, 2345);
intervalId = setInterval(function () {
    print('interval', ++count);
    if (count >= 10) {
        clearInterval(intervalId);
    }
}, 400);

