#ifndef LSMALLOC_H_
#define	LSMALLOC_H_
#ifdef __cplusplus
extern "C" {
#endif

#define LSMALLOC_HAVE_ATTR

#ifdef LSMALLOC_HAVE_ATTR
#  define LSMALLOC_ATTR(s) __attribute__((s))
#  define LSMALLOC_EXPORT LSMALLOC_ATTR(visibility("default"))
#  define LSMALLOC_ALIGNED(s) LSMALLOC_ATTR(aligned(s))
#  define LSMALLOC_SECTION(s) LSMALLOC_ATTR(section(s))
#  define LSMALLOC_NOINLINE LSMALLOC_ATTR(noinline)
#endif

#ifdef __cplusplus
};
#endif
#endif /* LSMALLOC_H_ */