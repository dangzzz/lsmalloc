/*
 * LSMALLOC_ALWAYS_INLINE and LSMALLOC_INLINE are used within header files for
 * functions that are static inline functions if inlining is enabled, and
 * single-definition library-private functions if inlining is disabled.
 *
 * LSMALLOC_ALWAYS_INLINE_C and LSMALLOC_INLINE_C are for use in .c files, in
 * which case the denoted functions are always static, regardless of whether
 * inlining is enabled.
 */
#if defined(LSMALLOC_DEBUG) || defined(LSMALLOC_CODE_COVERAGE)
   /* Disable inlining to make debugging/profiling easier. */
#  define LSMALLOC_ALWAYS_INLINE
#  define LSMALLOC_ALWAYS_INLINE_C static
#  define LSMALLOC_INLINE
#  define LSMALLOC_INLINE_C static
#  define inline
#else
#  define LSMALLOC_ENABLE_INLINE
#  ifdef LSMALLOC_HAVE_ATTR
#    define LSMALLOC_ALWAYS_INLINE \
	 static inline LSMALLOC_ATTR(unused) LSMALLOC_ATTR(always_inline)
#    define LSMALLOC_ALWAYS_INLINE_C \
	 static inline LSMALLOC_ATTR(always_inline)
#  else
#    define LSMALLOC_ALWAYS_INLINE static inline
#    define LSMALLOC_ALWAYS_INLINE_C static inline
#  endif
#  define LSMALLOC_INLINE static inline
#  define LSMALLOC_INLINE_C static inline

#endif

#ifdef LSMALLOC_CC_SILENCE
#  define UNUSED LSMALLOC_ATTR(unused)
#else
#  define UNUSED
#endif

#define	ZU(z)	((size_t)z)
#define	QU(q)	((uint64_t)q)
#define	QI(q)	((int64_t)q)

#ifndef __DECONST
#  define	__DECONST(type, var)	((type)(uintptr_t)(const void *)(var))
#endif

#ifndef LSMALLOC_HAS_RESTRICT
#  define restrict
#endif
