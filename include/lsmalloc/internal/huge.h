/******************************************************************************/
#ifdef LSMALLOC_H_TYPES

typedef struct huge_s huge_t;

#endif /* JEMALLOC_H_TYPES */
/******************************************************************************/
#ifdef LSMALLOC_H_STRUCTS

struct huge_s
{
	/*红黑树结点，以addr为key*/
	rb_node(huge_t) link_ad;

	/*数据大小*/
	size_t size;

	/*用于标识文件*/
	int file_no;
	
	/*所对应的pmem中内存块地址*/
	void * addr;
};
typedef rb_tree(huge_t) huge_tree_t;

#endif /* LSMALLOC_H_STRUCTS */
/******************************************************************************/
#ifdef LSMALLOC_H_EXTERNS

#endif /* LSMALLOC_H_EXTERNS */
/******************************************************************************/

rb_proto(, huge_tree_, huge_tree_t, huge_t)

#ifdef LSMALLOC_H_INLINES

#endif /* LSMALLOC_H_INLINES */
/******************************************************************************/
