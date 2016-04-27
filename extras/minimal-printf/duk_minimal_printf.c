/*
 *  Minimal sprintf() for Duktape.
 */

#include <stdarg.h>  /* va_list etc */
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* SIZE_MAX */

#define DUK__WRITE_CHAR(c) do { \
		if (off < size) { \
			str[off] = (char) c; \
		} \
		off++; \
	} while (0)

static const char duk__fmt_nybbles[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

static size_t duk__format_long(char *str, size_t size, size_t off, int fixed_length, char pad, int radix, int neg_sign, unsigned long v) {
	char buf[24];  /* 2^64 = 18446744073709552000, length 20 */
	char *required;
	char *p;
	int i;

	/* Format in reverse order first.  Ensure at least one digit is output
	 * to handle '0' correctly.  Note that space padding and zero padding
	 * handle negative sign differently:
	 *
	 *     %9d and -321  => '     -321'
	 *     %09d and -321 => '-00000321'
	 */

	for (i = 0; i < (int) sizeof(buf); i++) {
		buf[i] = pad;  /* compiles into memset() equivalent, avoid memset() dependency */
	}

	p = buf;
	do {
		*p++ = duk__fmt_nybbles[v % radix];
		v /= radix;
	} while (v != 0);

	required = buf + fixed_length;
	if (pad == (char) '0' && fixed_length > 0 /* handle "%0d" correctly, though insane */) {
		/* Leave space for negative sign. */
		if (p < required - neg_sign) {
			p = required - neg_sign;
		}
	}
	if (neg_sign) {
		*p++ = '-';
	}
	if (p < required) {
		p = required;
	}

	/* Now [buf,p[ contains the result in reverse; copy into place. */

	while (p > buf) {
		p--;
		DUK__WRITE_CHAR(*p);
	}

	return off;
}

static int duk__parse_pointer(const char *str, void **out) {
	const unsigned char *p;
	long val;  /* assume void * fits into long */
	int count;
	unsigned char ch;

	/* We only need to parse what our minimal printf() produces, so that
	 * we can check for a '0x' prefix, and assume all hex digits are
	 * lowercase.
	 */

	p = (const unsigned char *) str;
	if (*p++ != (unsigned char) '0') {
		return 0;
	}
	if (*p++ != (unsigned char) 'x') {
		return 0;
	}

	for (val = 0, count = 0; count < (int) (sizeof(void *) * 2); count++) {
		ch = *p++;

		val <<= 4;
		if (ch >= (unsigned char) '0' && ch <= (unsigned char) '9') {
			val += ch - (unsigned char) '0';
		} else if (ch >= (unsigned char) 'a' && ch <= (unsigned char) 'f') {
			val += ch - (unsigned char) 'a' + 0x0a;
		} else {
			return 0;
		}
	}

	/* The input may end at a NUL or garbage may follow.  As long as we
	 * parse the '%p' correctly, garbage is allowed to follow, and the
	 * JX pointer parsing also relies on that.
	 */

	*out = (void *) val;
	return 1;
}

int duk_minimal_vsnprintf(char *str, size_t size, const char *format, va_list ap) {
	size_t off = 0;
	const char *p;
	const char *p_tmp;
	const char *p_fmt_start;
	char c;
	char pad;
	int fixed_length;
	int is_long;

	/* Assume str != NULL unless size == 0.
	 * Assume format != NULL.
	 */

	p = format;
	for (;;) {
		c = *p++;
		if (c == (char) 0) {
			break;
		}
		if (c != (char) '%') {
			DUK__WRITE_CHAR(c);
			continue;
		}

		/* Start format sequence.  Scan flags and format specifier. */

		p_fmt_start = p - 1;
		is_long = 0;
		pad = ' ';
		fixed_length = 0;
		for (;;) {
			c = *p++;
			if (c == (char) 'l') {
				is_long = 1;
			} else if (c == (char) '0') {
				/* Only support pad character '0'. */
				pad = '0';
			} else if (c >= (char) '1' && c <= (char) '9') {
				/* Only support fixed lengths 1-9. */
				fixed_length = (int) (c - (char) '0');
			} else if (c == (char) 'd') {
				long v;
				int neg_sign = 0;
				if (is_long) {
					v = va_arg(ap, long);
				} else {
					v = (long) va_arg(ap, int);
				}
				if (v < 0) {
					neg_sign = 1;
					v = -v;
				}
				off = duk__format_long(str, size, off, fixed_length, pad, 10, neg_sign, (unsigned long) v);
				break;
			} else if (c == (char) 'u') {
				unsigned long v;
				if (is_long) {
					v = va_arg(ap, unsigned long);
				} else {
					v = (unsigned long) va_arg(ap, unsigned int);
				}
				off = duk__format_long(str, size, off, fixed_length, pad, 10, 0, v);
				break;
			} else if (c == (char) 'x') {
				unsigned long v;
				if (is_long) {
					v = va_arg(ap, unsigned long);
				} else {
					v = (unsigned long) va_arg(ap, unsigned int);
				}
				off = duk__format_long(str, size, off, fixed_length, pad, 16, 0, v);
				break;
			} else if (c == (char) 'c') {
				char v;
				v = va_arg(ap, int);  /* intentionally not 'char' */
				DUK__WRITE_CHAR(v);
				break;
			} else if (c == (char) 's') {
				const char *v;
				char c_tmp;
				v = va_arg(ap, const char *);
				if (v) {
					for (;;) {
						c_tmp = *v++;
						if (c_tmp) {
							DUK__WRITE_CHAR(c_tmp);
						} else {
							break;
						}
					}
				}
				break;
			} else if (c == (char) 'p') {
				/* Assume a void * can be represented by 'long'.  This is not
				 * always the case.  NULL pointer is printed out as 0x0000...
				 */
				void *v;
				v = va_arg(ap, void *);
				DUK__WRITE_CHAR('0');
				DUK__WRITE_CHAR('x');
				off = duk__format_long(str, size, off, sizeof(void *) * 2, '0', 16, 0, (unsigned long) v);
				break;
			} else {
				/* Unrecognized, just copy verbatim. */
#if 0
				DUK__WRITE_CHAR('!');
#endif
				for (p_tmp = p_fmt_start; p_tmp != p; p_tmp++) {
					DUK__WRITE_CHAR(*p_tmp);
				}
				break;
			}
		}
	}

	if (off < size) {
		str[off] = (char) 0;  /* No increment for 'off', not counted in return value. */
	} else if (size > 0) {
		/* Forced termination. */
		str[size - 1] = 0;
	}

	return (int) off;
}

int duk_minimal_snprintf(char *str, size_t size, const char *format, ...) {
	va_list ap;
	int ret;
	va_start(ap, format);
	ret = duk_minimal_vsnprintf(str, size, format, ap);
	va_end(ap);
	return ret;
}

int duk_minimal_sprintf(char *str, const char *format, ...) {
	va_list ap;
	int ret;
	va_start(ap, format);
	ret = duk_minimal_vsnprintf(str, SIZE_MAX, format, ap);
	va_end(ap);
	return ret;
}

int duk_minimal_sscanf(const char *str, const char *format, ...) {
	va_list ap;
	int ret;
	void **out;

	/* Only the exact "%p" format is supported. */
	if (format[0] != (char) '%' || format[1] != (char) 'p' ||
	    format[2] != (char) 0) {
	}

	va_start(ap, format);
	out = va_arg(ap, void **);
	ret = duk__parse_pointer(str, out);
	va_end(ap);

	return ret;
}

#undef DUK__WRITE_CHAR
