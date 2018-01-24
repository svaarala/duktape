/*
 *  Proxy object representation.
 */

#if !defined(DUK_HPROMISE_H_INCLUDED)
#define DUK_HPROMISE_H_INCLUDED

#define DUK_ASSERT_HPROXY_VALID(h) do { \
		DUK_ASSERT((h) != NULL); \
		DUK_ASSERT((h)->resolver != NULL); \
	} while (0)

// DUK_ASSERT(DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ((duk_hobject *) (h))); \

struct duk_hpromise {
	/* Shared object part. */
	duk_hobject obj;

	/* Proxy target object. */
	duk_hobject *resolver;
};

#endif  /* DUK_HPROMISE_H_INCLUDED */
