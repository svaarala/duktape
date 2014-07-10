if (typeof console === 'undefined') {
    this.console = {};
}
if (typeof console.log === 'undefined') {
    this.console.log = function () {
        print(Array.prototype.join.call(arguments, ' '));
    }
}
