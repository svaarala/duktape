/*
 *  Fixed buffer helper useful for debugging, requires no allocation
 *  which is critical for debugging.
 */

#include "duk_internal.h"

#ifdef DUK_USE_DEBUG

void duk_fb_put_bytes(duk_fixedbuffer *fb, duk_u8 *buffer, duk_u32 length) {
	duk_u32 avail;

	avail = (fb->offset >= fb->length ? (duk_u32) 0 : (duk_u32) (fb->length - fb->offset));
	if (length > avail) {
		DUK_MEMCPY(fb->buffer + fb->offset, buffer, avail);
		fb->offset += avail;
		fb->truncated = 1;
	} else {
		DUK_MEMCPY(fb->buffer + fb->offset, buffer, length);
		fb->offset += length;
	}
}

void duk_fb_put_byte(duk_fixedbuffer *fb, duk_u8 x) {
	duk_fb_put_bytes(fb, &x, 1);
}

void duk_fb_put_cstring(duk_fixedbuffer *fb, char *x) {
	duk_fb_put_bytes(fb, (duk_u8 *) x, (duk_u32) strlen(x));
}

void duk_fb_sprintf(duk_fixedbuffer *fb, const char *fmt, ...) {
	duk_u32 avail;
	va_list ap;

	va_start(ap, fmt);
	avail = (fb->offset >= fb->length ? (duk_u32) 0 : (duk_u32) (fb->length - fb->offset));
	if (avail > 0) {
		int res = DUK_VSNPRINTF((char *) (fb->buffer + fb->offset), avail, fmt, ap);
		if (res < 0) {
			/* error */
		} else if (res >= avail) {
			/* truncated */
			fb->offset += avail;
			if (res > avail) {
				/* actual chars dropped (not just null term) */
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

