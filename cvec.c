#include "cvec.h"


void vec_alloc( void **vector, int n, size_t type_size ){
	size_t sz = (n * type_size) + sizeof(vec_t);
	void *data = SDL_malloc( sz );
	SDL_memset( data, 0, sz );
	vec_t *meta = (vec_t *)data;
	meta->allocated = n;
	meta->used = n;
	*vector = meta + 1;
}

void vec_grow(void **vector, size_t more, size_t type_size) {
	vec_t *meta = vec_meta(*vector);
	size_t count = 0;
	void *data = NULL;

	if (*vector) {
		count = 2 * meta->allocated + more;
		data = SDL_realloc(meta, type_size * count + sizeof *meta);
	} else {
		count = more + 1;
		data = SDL_malloc(type_size * count + sizeof *meta);
		((vec_t *)data)->used = 0;
	}

	meta = (vec_t *)data;
	meta->allocated = count;

	SDL_memset( data + sizeof(vec_t) + (meta->used * type_size), 0, 
				(meta->allocated - meta->used) * type_size );

	*vector = meta + 1;
}

void vec_delete(void *vector) {
	if( vector == NULL ) return;
	SDL_free(vec_meta(vector));
}
