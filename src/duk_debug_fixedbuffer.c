/*
 *  Fixed buffer helper useful for debugging, requires no allocation
 *  which is critical for debugging.
 */

#include "duk_internal.h"

#ifdef DUK_USE_DEBUG

void duk_fb_put_bytes(duk_fixedbuffer *fb, duk_uint8_t *buffer, duk_uint32_t length) {
	duk_uint32_t avail;

	avail = (fb->offset >= fb->length ? (duk_uint32_t) 0 : (duk_uint32_t) (fb->length - fb->offset));
	if (length > avail) {
		DUK_MEMCPY(fb->buffer + fb->offset, buffer, avail);
		fb->offset += avail;
		fb->truncated = 1;
	} else {
		DUK_MEMCPY(fb->buffer + fb->offset, buffer, length);
		fb->offset += length;
	}
}

void duk_fb_put_byte(duk_fixedbuffer *fb, duk_uint8_t x) {
	duk_fb_put_bytes(fb, &x, 1);
}

void duk_fb_put_cstring(duk_fixedbuffer *fb, const char *x) {
	duk_fb_put_bytes(fb, (duk_uint8_t *) x, (duk_uint32_t) DUK_STRLEN(x));
}

void duk_fb_sprintf(duk_fixedbuffer *fb, const char *fmt, ...) {
	duk_uint32_t avail;
	va_list ap;

	va_start(ap, fmt);
	avail = (fb->offset >= fb->length ? (duk_uint32_t) 0 : (duk_uint32_t) (fb->length - fb->offset));
	if (avail > 0) {
		int res = DUK_VSNPRINTF((char *) (fb->buffer + fb->offset), avail, fmt, ap);
		if (res < 0) {
			/* error */
		} else if ((duk_uint32_t) res >= avail) {
			/* (maybe) truncated */
			fb->offset += avail;
			if ((duk_uint32_t) res > avail) {
				/* actual chars dropped (not just NUL term) */
				fb->truncated = 1;
			}
		} else {
			/* normal */
			fb->offset += res;
		}
	}
	va_end(ap);
}

int duk_fb_is_full(duk_fixedbuffer *fb) {
	return (fb->offset >= fb->length);
}

#endif  /* DUK_USE_DEBUG */

