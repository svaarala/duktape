#if !defined(DUK_PROP_H_INCLUDED)
#define DUK_PROP_H_INCLUDED

#include "duk_internal.h"

#define DUK_DELPROP_FLAG_THROW (1U << 0)
#define DUK_DELPROP_FLAG_FORCE (1U << 1)

DUK_INTERNAL_DECL duk_bool_t duk_prop_double_idx_check(duk_double_t d, duk_uarridx_t *out_idx);
DUK_INTERNAL_DECL void duk_prop_push_plainstr_idx(duk_hthread *thr, duk_hstring *h, duk_uarridx_t idx);
DUK_INTERNAL_DECL void duk_prop_frompropdesc_propattrs(duk_hthread *thr, duk_int_t attrs);
DUK_INTERNAL_DECL void duk_prop_frompropdesc_with_idx(duk_hthread *thr, duk_idx_t idx_desc, duk_int_t attrs);
DUK_INTERNAL_DECL duk_small_uint_t duk_prop_topropdesc(duk_hthread *thr);
DUK_INTERNAL_DECL void duk_prop_pop_propdesc(duk_hthread *thr, duk_small_int_t attrs);
DUK_INTERNAL_DECL duk_small_uint_t duk_prop_propdesc_valcount(duk_small_int_t attrs);

DUK_INTERNAL_DECL duk_hobject *duk_prop_switch_stabilized_target_top(duk_hthread *thr, duk_hobject *target, duk_hobject *next);

DUK_INTERNAL_DECL duk_bool_t duk_prop_arguments_map_prep(duk_hthread *thr,
                                                         duk_hobject *obj,
                                                         duk_hobject **out_map,
                                                         duk_hobject **out_env);
DUK_INTERNAL_DECL duk_hstring *duk_prop_arguments_map_prep_idxkey(duk_hthread *thr,
                                                                  duk_hobject *obj,
                                                                  duk_uarridx_t idx,
                                                                  duk_hobject **out_map,
                                                                  duk_hobject **out_env);

DUK_INTERNAL_DECL duk_bool_t duk_prop_getvalue_outidx(duk_hthread *thr, duk_idx_t idx_obj, duk_tval *tv_key, duk_idx_t idx_out);
DUK_INTERNAL_DECL duk_bool_t duk_prop_getvalue_idxkey_outidx(duk_hthread *thr,
                                                             duk_idx_t idx_obj,
                                                             duk_uarridx_t idx,
                                                             duk_idx_t idx_out);
DUK_INTERNAL_DECL duk_bool_t duk_prop_getvalue_strkey_outidx(duk_hthread *thr,
                                                             duk_idx_t idx_obj,
                                                             duk_hstring *key,
                                                             duk_idx_t idx_out);
DUK_INTERNAL_DECL duk_bool_t duk_prop_getvalue_push(duk_hthread *thr, duk_idx_t idx_obj, duk_tval *tv_key);
DUK_INTERNAL_DECL duk_bool_t duk_prop_getvalue_stridx_push(duk_hthread *thr, duk_idx_t idx_obj, duk_small_uint_t stridx);
DUK_INTERNAL_DECL duk_bool_t duk_prop_getvalue_stridx_outidx(duk_hthread *thr,
                                                             duk_idx_t idx_obj,
                                                             duk_small_uint_t stridx,
                                                             duk_idx_t idx_out);

DUK_INTERNAL_DECL duk_bool_t
duk_prop_putvalue_inidx(duk_hthread *thr, duk_idx_t idx_recv, duk_tval *tv_key, duk_idx_t idx_val, duk_bool_t throw_flag);
DUK_INTERNAL_DECL duk_bool_t
duk_prop_putvalue_strkey_inidx(duk_hthread *thr, duk_idx_t idx_recv, duk_hstring *key, duk_idx_t idx_val, duk_bool_t throw_flag);
DUK_INTERNAL_DECL duk_bool_t
duk_prop_putvalue_idxkey_inidx(duk_hthread *thr, duk_idx_t idx_recv, duk_uarridx_t idx, duk_idx_t idx_val, duk_bool_t throw_flag);

DUK_INTERNAL_DECL duk_bool_t duk_prop_deleteoper(duk_hthread *thr, duk_idx_t idx_obj, duk_tval *tv_key, duk_bool_t throw_flag);
DUK_INTERNAL_DECL duk_bool_t duk_prop_delete_strkey(duk_hthread *thr, duk_idx_t idx_obj, duk_hstring *key, duk_bool_t throw_flag);
DUK_INTERNAL_DECL duk_bool_t duk_prop_delete_obj_strkey(duk_hthread *thr,
                                                        duk_hobject *obj,
                                                        duk_hstring *key,
                                                        duk_bool_t throw_flag);
DUK_INTERNAL_DECL duk_bool_t duk_prop_delete_idxkey(duk_hthread *thr, duk_idx_t idx_obj, duk_uarridx_t idx, duk_bool_t throw_flag);
DUK_INTERNAL_DECL duk_bool_t duk_prop_delete_obj_idxkey(duk_hthread *thr,
                                                        duk_hobject *obj,
                                                        duk_uarridx_t idx,
                                                        duk_bool_t throw_flag);

DUK_INTERNAL_DECL duk_small_int_t duk_prop_getowndesc_obj_strkey(duk_hthread *thr, duk_hobject *obj, duk_hstring *key);
DUK_INTERNAL_DECL duk_small_int_t duk_prop_getowndesc_obj_idxkey(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx);
DUK_INTERNAL_DECL duk_small_int_t duk_prop_getowndesc_obj_tvkey(duk_hthread *thr, duk_hobject *obj, duk_tval *tv_key);

DUK_INTERNAL_DECL duk_small_int_t duk_prop_getownattr_obj_strkey(duk_hthread *thr, duk_hobject *obj, duk_hstring *key);
DUK_INTERNAL_DECL duk_small_int_t duk_prop_getownattr_obj_idxkey(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx);
DUK_INTERNAL_DECL duk_small_int_t duk_prop_getownattr_obj_tvkey(duk_hthread *thr, duk_hobject *obj, duk_tval *tv_key);

DUK_INTERNAL_DECL duk_bool_t duk_prop_has(duk_hthread *thr, duk_tval *tv_obj, duk_tval *tv_key);
DUK_INTERNAL_DECL duk_bool_t duk_prop_has_strkey(duk_hthread *thr, duk_tval *tv_obj, duk_hstring *key);
DUK_INTERNAL_DECL duk_bool_t duk_prop_has_idxkey(duk_hthread *thr, duk_tval *tv_obj, duk_uarridx_t idx);

DUK_INTERNAL_DECL duk_bool_t
duk_prop_defown(duk_hthread *thr, duk_hobject *obj, duk_tval *tv_key, duk_idx_t idx_desc, duk_uint_t defprop_flags);
DUK_INTERNAL_DECL duk_bool_t
duk_prop_defown_strkey(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_idx_t idx_desc, duk_uint_t defprop_flags);
DUK_INTERNAL_DECL duk_bool_t
duk_prop_defown_idxkey(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_desc, duk_uint_t defprop_flags);

#define DUK_OWNPROPKEYS_FLAG_INCLUDE_ARRIDX     (1U << 0)
#define DUK_OWNPROPKEYS_FLAG_INCLUDE_STRING     (1U << 1)
#define DUK_OWNPROPKEYS_FLAG_INCLUDE_SYMBOL     (1U << 2)
#define DUK_OWNPROPKEYS_FLAG_INCLUDE_HIDDEN     (1U << 3)
#define DUK_OWNPROPKEYS_FLAG_REQUIRE_ENUMERABLE (1U << 4)
#define DUK_OWNPROPKEYS_FLAG_STRING_COERCE      (1U << 5)
#define DUK_OWNPROPKEYS_FLAG_NO_PROXY_BEHAVIOR  (1U << 6)

DUK_INTERNAL_DECL void duk_prop_ownpropkeys(duk_hthread *thr, duk_hobject *obj, duk_uint_t ownpropkeys_flags);

DUK_INTERNAL_DECL void duk_prop_enum_keylist(duk_hthread *thr, duk_hobject *obj, duk_uint_t enum_flags);
DUK_INTERNAL_DECL void duk_prop_enum_create_enumerator(duk_hthread *thr, duk_hobject *obj, duk_uint_t enum_flags);
DUK_INTERNAL_DECL duk_bool_t duk_prop_enum_next(duk_hthread *thr, duk_idx_t idx_enum, duk_bool_t get_value);

#endif /* DUK_PROP_H_INCLUDED */
