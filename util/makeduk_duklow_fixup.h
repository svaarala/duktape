/* duk-low fixups. */

extern uint8_t *lowmem_ram;
extern duk_uint16_t lowmem_enc16(void *ud, void *p);
extern void *lowmem_dec16(void *ud, duk_uint16_t x);
extern const void *lowmem_extstr_check_1(const void *ptr, duk_size_t len);
extern const void *lowmem_extstr_check_2(const void *ptr, duk_size_t len);
extern const void *lowmem_extstr_check_3(const void *ptr, duk_size_t len);
extern void lowmem_extstr_free_1(const void *ptr);
extern void lowmem_extstr_free_2(const void *ptr);
extern void lowmem_extstr_free_3(const void *ptr);
extern duk_bool_t lowmem_exec_timeout_check(void *udata);

/* Needed for inline pointer compression functions. */
#include "duk_alloc_pool.h"
