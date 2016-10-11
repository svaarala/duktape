/* Ajduk fixups. */

extern uint8_t *ajsheap_ram;
extern duk_uint16_t ajsheap_enc16(void *ud, void *p);
extern void *ajsheap_dec16(void *ud, duk_uint16_t x);
extern const void *ajsheap_extstr_check_1(const void *ptr, duk_size_t len);
extern const void *ajsheap_extstr_check_2(const void *ptr, duk_size_t len);
extern const void *ajsheap_extstr_check_3(const void *ptr, duk_size_t len);
extern void ajsheap_extstr_free_1(const void *ptr);
extern void ajsheap_extstr_free_2(const void *ptr);
extern void ajsheap_extstr_free_3(const void *ptr);
extern duk_bool_t ajsheap_exec_timeout_check(void *udata);
