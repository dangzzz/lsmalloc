/*
 * Lock-free singly linked list.
 * <1> It include insert, delete, iteration.
 * <2> It is safe to 
 */


#define	sl_head(a_type)							\
struct {								\
	a_type *qlh_first;						\
}

#define	sl_head_initializer(a_head) {NULL}

//next,prev双指针
#define	sl_elm(a_type)							\
struct {								\
	a_type	*qle_next;							\
}

/* List functions. */
#define	sl_new(a_head) do {						\
	(a_head)->qlh_first = NULL;					\
} while (0)

//让next指向自身
#define	sl_elm_new(a_elm, a_field) do {					\
	(a_elm)->a_field.qle_next = NULL;				\
} while (0)

#define	sl_first(a_head) ((a_head)->qlh_first)

#define sl_head_insert(a_head, a_elm, a_field) do{	\
	do{												\
		(a_elm)->a_field.qle_next = (a_head)->qlh_first;\
	}while(!__sync_bool_compare_and_swap(&(a_head)->qlh_first,(a_elm)->a_field.qle_next,(a_elm)));\
}while(0)

#define sl_after_insert(a_qlelm,a_elm,a_field) do{\
	do{													\
		(a_elm)->a_field.qle_next = (a_qlelm)->a_field.qle_next;\
	}while(!__sync_bool_compare_and_swap(&(a_qlelm)->a_field.qle_next,(a_elm)->a_field.qle_next,(a_elm)));\
}

#define	sl_next(a_elm, a_field)					\
	((a_elm)->a_field.qle_next)

#define sl_after_remove(a_qlelm, a_elm, a_field) do {     	\
	(a_qlelm)->a_field.qle_next = (a_elm)->a_field.qle_next;\
}while(0);

