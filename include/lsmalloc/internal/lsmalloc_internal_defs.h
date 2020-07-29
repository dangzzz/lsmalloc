#ifndef LSMALLOC_INTERNAL_DEFS_H_
#define	LSMALLOC_INTERNAL_DEFS_H_

/* Non-empty if the tls_model attribute is supported. */
#define LSMALLOC_TLS_MODEL 

/* TLS is used to map arenas and magazine caches to threads. */
#define LSMALLOC_TLS 

/* sizeof(int) == 2^LG_SIZEOF_INT. */
#define LG_SIZEOF_INT 2

/* sizeof(long) == 2^LG_SIZEOF_LONG. */
#define LG_SIZEOF_LONG 3

/* sizeof(intmax_t) == 2^LG_SIZEOF_INTMAX_T. */
#define LG_SIZEOF_INTMAX_T 3

/* One page is 2^STATIC_PAGE_SHIFT bytes. */
#define STATIC_PAGE_SHIFT 12

#endif /* LSMALLOC_INTERNAL_DEFS_H_ */

