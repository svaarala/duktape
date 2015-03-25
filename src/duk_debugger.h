#ifndef DUK_DEBUGGER_H_INCLUDED
#define DUK_DEBUGGER_H_INCLUDED

/* Debugger protocol version is defined in the public API header. */

#define DUK_DBG_MARKER_EOM        0x00
#define DUK_DBG_MARKER_REQUEST    0x01
#define DUK_DBG_MARKER_REPLY      0x02
#define DUK_DBG_MARKER_ERROR      0x03
#define DUK_DBG_MARKER_NOTIFY     0x04

#define DUK_DBG_ERR_UNKNOWN       0x00
#define DUK_DBG_ERR_UNSUPPORTED   0x01
#define DUK_DBG_ERR_TOOMANY       0x02
#define DUK_DBG_ERR_NOTFOUND      0x03

/* Initiated by Duktape */
#define DUK_DBG_CMD_STATUS        0x01
#define DUK_DBG_CMD_PRINT         0x02
#define DUK_DBG_CMD_ALERT         0x03
#define DUK_DBG_CMD_LOG           0x04

/* Initiated by debug client */
#define DUK_DBG_CMD_BASICINFO     0x10
#define DUK_DBG_CMD_TRIGGERSTATUS 0x11
#define DUK_DBG_CMD_PAUSE         0x12
#define DUK_DBG_CMD_RESUME        0x13
#define DUK_DBG_CMD_STEPINTO      0x14
#define DUK_DBG_CMD_STEPOVER      0x15
#define DUK_DBG_CMD_STEPOUT       0x16
#define DUK_DBG_CMD_LISTBREAK     0x17
#define DUK_DBG_CMD_ADDBREAK      0x18
#define DUK_DBG_CMD_DELBREAK      0x19
#define DUK_DBG_CMD_GETVAR        0x1a
#define DUK_DBG_CMD_PUTVAR        0x1b
#define DUK_DBG_CMD_GETCALLSTACK  0x1c
#define DUK_DBG_CMD_GETLOCALS     0x1d
#define DUK_DBG_CMD_EVAL          0x1e
#define DUK_DBG_CMD_DETACH        0x1f
#define DUK_DBG_CMD_DUMPHEAP      0x20
#define DUK_DBG_CMD_GETBYTECODE   0x21

#if defined(DUK_USE_DEBUGGER_SUPPORT)
DUK_INTERNAL_DECL void duk_debug_do_detach(duk_heap *heap);

DUK_INTERNAL_DECL duk_bool_t duk_debug_read_peek(duk_hthread *thr);
DUK_INTERNAL_DECL void duk_debug_write_flush(duk_hthread *thr);

DUK_INTERNAL_DECL void duk_debug_skip_bytes(duk_hthread *thr, duk_size_t length);
DUK_INTERNAL_DECL void duk_debug_skip_byte(duk_hthread *thr);

DUK_INTERNAL_DECL void duk_debug_read_bytes(duk_hthread *thr, duk_uint8_t *data, duk_size_t length);
DUK_INTERNAL_DECL duk_uint8_t duk_debug_read_byte(duk_hthread *thr);
DUK_INTERNAL_DECL duk_int32_t duk_debug_read_int(duk_hthread *thr);
DUK_INTERNAL_DECL duk_hstring *duk_debug_read_hstring(duk_hthread *thr);
/* XXX: exposed duk_debug_read_pointer */
/* XXX: exposed duk_debug_read_buffer */
/* XXX: exposed duk_debug_read_hbuffer */
DUK_INTERNAL_DECL void duk_debug_read_tval(duk_hthread *thr);

DUK_INTERNAL_DECL void duk_debug_write_bytes(duk_hthread *thr, const duk_uint8_t *data, duk_size_t length);
DUK_INTERNAL_DECL void duk_debug_write_byte(duk_hthread *thr, duk_uint8_t x);
DUK_INTERNAL_DECL void duk_debug_write_unused(duk_hthread *thr);
DUK_INTERNAL_DECL void duk_debug_write_undefined(duk_hthread *thr);
DUK_INTERNAL_DECL void duk_debug_write_int(duk_hthread *thr, duk_int32_t x);
DUK_INTERNAL_DECL void duk_debug_write_uint(duk_hthread *thr, duk_uint32_t x);
DUK_INTERNAL_DECL void duk_debug_write_string(duk_hthread *thr, const char *data, duk_size_t length);
DUK_INTERNAL_DECL void duk_debug_write_cstring(duk_hthread *thr, const char *data);
DUK_INTERNAL_DECL void duk_debug_write_hstring(duk_hthread *thr, duk_hstring *h);
DUK_INTERNAL_DECL void duk_debug_write_buffer(duk_hthread *thr, const char *data, duk_size_t length);
DUK_INTERNAL_DECL void duk_debug_write_hbuffer(duk_hthread *thr, duk_hbuffer *h);
DUK_INTERNAL_DECL void duk_debug_write_pointer(duk_hthread *thr, const void *ptr);
#if defined(DUK_USE_DEBUGGER_DUMPHEAP)
DUK_INTERNAL_DECL void duk_debug_write_heapptr(duk_hthread *thr, duk_heaphdr *h);
#endif
DUK_INTERNAL_DECL void duk_debug_write_hobject(duk_hthread *thr, duk_hobject *obj);
DUK_INTERNAL_DECL void duk_debug_write_tval(duk_hthread *thr, duk_tval *tv);

#if 0  /* unused */
DUK_INTERNAL_DECL void duk_debug_write_request(duk_hthread *thr, duk_small_uint_t command);
#endif
DUK_INTERNAL_DECL void duk_debug_write_reply(duk_hthread *thr);
DUK_INTERNAL_DECL void duk_debug_write_error_eom(duk_hthread *thr, duk_small_uint_t err_code, const char *msg);
DUK_INTERNAL_DECL void duk_debug_write_notify(duk_hthread *thr, duk_small_uint_t command);
DUK_INTERNAL_DECL void duk_debug_write_eom(duk_hthread *thr);

DUK_INTERNAL duk_uint_fast32_t duk_debug_curr_line(duk_hthread *thr);
DUK_INTERNAL void duk_debug_send_status(duk_hthread *thr);

DUK_INTERNAL_DECL duk_bool_t duk_debug_process_messages(duk_hthread *thr, duk_bool_t no_block);

DUK_INTERNAL_DECL duk_small_int_t duk_debug_add_breakpoint(duk_hthread *thr, duk_hstring *filename, duk_uint32_t line);
DUK_INTERNAL_DECL duk_bool_t duk_debug_remove_breakpoint(duk_hthread *thr, duk_small_uint_t breakpoint_index);
#endif

#endif  /* DUK_DEBUGGER_H_INCLUDED */
