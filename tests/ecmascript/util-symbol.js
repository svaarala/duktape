/*
 *  Some testing utilities for Symbols.
 */

function restoreSymbolCoercionMethods(old) {
    Object.defineProperty(Symbol.prototype, Symbol.toPrimitive, { writable: true });

    Symbol.prototype.toString = old.oldToString;
    Symbol.prototype.valueOf = old.oldValueOf;
    Symbol.prototype[Symbol.toPrimitive] = old.oldToPrimitive;

    Object.defineProperty(Symbol.prototype, Symbol.toPrimitive, { writable: false });

    if (Symbol.prototype.toString !== old.oldToString) {
        throw new Error('failed to restore .toString');
    }
    if (Symbol.prototype.valueOf !== old.oldValueOf) {
        throw new Error('failed to restore .valueOf');
    }
    if (Symbol.prototype[Symbol.toPrimitive] !== old.oldToPrimitive) {
        throw new Error('failed to restore @@toPrimitive');
    }
}

function setupLoggingSymbolCoercionMethods() {
    var oldToString = Symbol.prototype.toString;
    var oldValueOf = Symbol.prototype.valueOf;
    var oldToPrimitive = Symbol.prototype[Symbol.toPrimitive];

    Object.defineProperty(Symbol.prototype, Symbol.toPrimitive, { writable: true });

    Symbol.prototype.toString = function replacementToString() {
        print('replacement toString called:', String(this));
        return oldToString.call(this);
    };
    Symbol.prototype.valueOf = function replacementValueOf() {
        print('replacement valueOf called:', String(this));
        return oldValueOf.call(this);
    };
    Symbol.prototype[Symbol.toPrimitive] = function replacementToPrimitive() {
        // Avoid String(this) here, it causes infinite recursion and a RangeError.
        print('replacement @@toPrimitive called', typeof this, Object.prototype.toString.call(this));
        return oldValueOf.call(this);
    };

    Object.defineProperty(Symbol.prototype, Symbol.toPrimitive, { writable: false });

    if (Symbol.prototype.toString === oldToString) {
        throw new Error('failed to replace .toString');
    }
    if (Symbol.prototype.valueOf === oldValueOf) {
        throw new Error('failed to replace .valueOf');
    }
    if (Symbol.prototype[Symbol.toPrimitive] === oldToPrimitive) {
        throw new Error('failed to replace @@toPrimitive');
    }

    return {
        oldToString: oldToString,
        oldValueOf: oldValueOf,
        oldToPrimitive: oldToPrimitive
    };
}
