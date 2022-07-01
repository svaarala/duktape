/* Generate a table indicating how many digits should be considered
 * significant for each radix (2 to 36) when doing string-to-number
 * conversion.
 *
 * For decimal, the E5/E5.1 specification indicates that anything
 * after the 20th digit can be ignored (treated as zero) and the
 * 20th digit can be rounded upwards.  We estimate the significant
 * bits of precision from this and compute similar values for other
 * radix values.
 *
 * Also generate a table of minimum and maximum radix-specific
 * exponent values above and below which a number is guaranteed
 * to overflow to Infinity or underflow to zero.  This allows the
 * C code to quick reject such exponent values, and to keep bigint
 * values bounded.  The exponent limit is relative to an integer
 * significand padded to the precision-related digit count (e.g.
 * 20 for decimal).
 */

'use strict';

function genNumDigits() {
    var digits = [];
    var limits = [];

    for (let radix = 2; radix <= 36; radix++) {
        var bits_per_digit = Math.log2(radix);
        var prec_digits;
        if (radix === 10) {
            prec_digits = 20;
        } else {
            var target_bits = Math.ceil(Math.log2(10) * 20) + 2; // +2 is extra, just in case
            prec_digits = Math.ceil(target_bits / bits_per_digit);
        }
        digits.push(prec_digits);

        // these are conservative (details are off by one etc); +/- 2 is the extra
        var overflow_limit = Math.ceil(1024.0 / bits_per_digit) + 2 - prec_digits;
        var underflow_limit = Math.floor((-1024.0 - 52.0) / bits_per_digit) - 2 - prec_digits;

        limits.push([overflow_limit, underflow_limit]);
    }

    var doc = { digits: digits, limits: limits };
    console.log(JSON.stringify(doc));

    var res;

    res = [];
    res.push('DUK_LOCAL const duk_uint8_t duk__str2num_digits_for_radix[] = {');
    res.push('\t' + digits.join(', '));
    res.push('};');
    console.log(res.join('\n'));

    res = [];
    res.push('DUK_LOCAL const duk__exp_limits duk__str2num_exp_limits[] = {');
    res.push('\t' + limits.map(function (v) { return '{ ' + v[0] + ', ' + v[1] + ' }'; }).join(', '));
    res.push('};');
    console.log(res.join('\n'));
}
exports.genNumDigits = genNumDigits;
