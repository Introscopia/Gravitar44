#ifndef CVEC_HDR
#define CVEC_HDR
#include <SDL.h>

typedef struct {
    size_t allocated;
    size_t used;
} vec_t;


/* Attempts to grow [VECTOR] by [MORE]*/
#define vec_try_grow(VECTOR, MORE) \
    (((!(VECTOR) || vec_meta(VECTOR)->used + (MORE) >= vec_meta(VECTOR)->allocated)) ? \
        (void)vec_grow(((void **)&(VECTOR)), (MORE), sizeof(*(VECTOR))) : (void)0)

/* Get the metadata block for [VECTOR] */
#define vec_meta(VECTOR) \
    ((vec_t *)(((unsigned char *)(VECTOR)) - sizeof(vec_t)))

/* Deletes [VECTOR] and sets it to NULL */
#define vec_free(VECTOR) \
    ((void)((VECTOR) ? (vec_delete((void *)(VECTOR)), (VECTOR) = NULL) : 0))

/* Pushes back [VALUE] into [VECTOR] */
#define vec_push(VECTOR, VALUE) \
    (vec_try_grow((VECTOR), 1), (VECTOR)[vec_meta(VECTOR)->used++] = (VALUE))

/* returns a reference to a new element at the end of [VECTOR] */
#define vec_new(VECTOR) \
    (vec_try_grow((VECTOR), 1), (VECTOR)+(vec_meta(VECTOR)->used++) )

/* Get the size of [VECTOR] */
#define vec_size(VECTOR) \
    ((VECTOR) ? vec_meta(VECTOR)->used : 0)

/* Get the capacity of [VECTOR] */
#define vec_capacity(VECTOR) \
    ((VECTOR) ? vec_meta(VECTOR)->allocated : 0)

/* Resize [VECTOR] to accomodate [SIZE] more elements */
#define vec_resize(VECTOR, SIZE)       \
    (vec_try_grow((VECTOR), (SIZE)),   \
     vec_meta(VECTOR)->used += (SIZE), \
     &(VECTOR)[vec_meta(VECTOR)->used - (SIZE)])

/* Get the last element in [VECTOR] */
#define vec_last(VECTOR) \
    ((VECTOR)[vec_meta(VECTOR)->used - 1])

/* Pop an element off the back of [VECTOR] */
#define vec_pop(VECTOR) \
    ((void)(vec_meta(VECTOR)->used -= 1))

/* Shrink the size of [VECTOR] down to [SIZE] */
#define vec_shrinkto(VECTOR, SIZE) \
    ((void)(vec_meta(VECTOR)->used = (SIZE)))

/* Shrink [VECTOR] down by [AMOUNT] */
#define vec_shrinkby(VECTOR, AMOUNT) \
    ((void)(vec_meta(VECTOR)->used -= (AMOUNT)))

/* Append to [VECTOR], [COUNT] elements from [POINTER] */
#define vec_append(VECTOR, COUNT, POINTER) \
    ((void)(SDL_memcpy(vec_resize((VECTOR), (COUNT)), (POINTER), (COUNT) * sizeof(*(POINTER)))))

/* Remove from [VECTOR], [COUNT] elements starting from [INDEX] */
#define vec_remove(VECTOR, INDEX, COUNT) \
    ((void)(SDL_memmove((VECTOR) + (INDEX), (VECTOR) + (INDEX) + (COUNT), \
        sizeof(*(VECTOR)) * (vec_meta(VECTOR)->used - (INDEX) - (COUNT))), \
            vec_meta(VECTOR)->used -= (COUNT)))

#define vec_init( VECTOR, N ) \
    vec_alloc(((void **)&(VECTOR)), (N), sizeof(*(VECTOR)))


#define vec_copy( DST, SRC ) \
    vec_init( DST, vec_size( SRC ) ); \
    for (int i = 0; i < vec_size( SRC ); ++i ){ DST[i] = SRC[i]; }


void vec_alloc( void **vector, int n, size_t s);
void vec_grow(void **vector, size_t i, size_t s);
void vec_delete(void *vector);

#endif
