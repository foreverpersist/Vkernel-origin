#ifndef __HASHMAP_H
#define __HASHMAP_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OK 0
#define ERR_SYS -1
#define ERR_USER -2
#define ERR_EMPTY -3

/* Table size is powers of 2. */
#define MIN_CAPACITY (1 << 5)
#define DEF_CAPACITY (1 << 8)

#define DEF_FACTOR 0.75f

#define hashmap_log(stream, format, args...) \
	fprintf(stream, "%s: %d: %s: " format, __FILE__, __LINE__, __func__, ##args); 

#define hashmap_error(format, args...) hashmap_log(stderr, format, ##args)

#define hashmap(name) name##_hashmap
#define iter(name) name##_iter

// #define hashmap_create(name) name##_hashmap_create(0, 0)
// #define hashmap_create(name, capacity) name##_hashmap(capacity, 0)
#define hashmap_create(name, capacity, load_factor) \
	name##_hashmap_create(capacity, load_factor)
#define hashmap_destroy(name, map) \
	name##_hashmap_destroy(&map)
#define hashmap_set_hash_func(name, map, hash_func) \
	name##_hashmap_set_hash_func(map, hash_func)
#define hashmap_set_compare_func(name, map, compare_func) \
	name##_hashmap_set_compare_func(map, compare_func)
#define hashmap_set_key_funcs(name, map, key_alloc_func, key_free_func) \
	name##_hashmap_set_key_funcs(map, key_alloc_func, key_free_func)
#define hashmap_has_key(name, map, key) \
	name##_hashmap_has_key(map, key)
#define hashmap_get(name, map, key, pvalue) \
	name##_hashmap_get(map, key, pvalue);
#define hashmap_put(name, map, key, value) \
	name##_hashmap_put(map, key, value);
#define hashmap_remove(name, map, key) \
	name##_hashmap_remove(map, key)
#define hashmap_size(name, map) \
	name##_hashmap_size(map)
#define hashmap_collision_mean(name, map) \
	name##_hashmap_collision_mean(map)
#define hashmap_collision_variance(name, map) \
	name##_hashmap_collision_variance(map)
#define hashmap_iter(name, map) \
	name##_hashmap_iter(map)
#define hashmap_next(name, map, it) \
	name##_hashmap_next(map, it)
#define hashmap_iter_get(name, it, pkey, pvalue) \
	name##_hashmap_iter_get(it, pkey, pvalue)
#define hashmap_iter_put(name, it, value) \
	name##_hahsmap_iter_put(it, value)
#define hashmap_foreach(name, map, func, args) \
	name##_hashmap_foreach(map, func, args)

/* All structures and functions are implemented here, 
   because maybe only `macro` can be used for any data type.
*/
#define HASHMAP_DEFINE(name, key_type, value_type) \
	\
	/* Data Entity, member `next` is a Data Entity pointer. */ \
	typedef struct name##_ENTITY { \
		key_type key; \
		value_type value; \
		struct name##_ENTITY *next; \
	} name##_entity, *name##_iter; \
	\
	/* Table Item, member `entities` is a Data Entity link list. */ \
	typedef struct { \
		size_t collision; \
		name##_iter entities; \
	} name##_item; \
	\
	/* HashMap Entity, member `table` is a Table Item array. */ \
	typedef struct { \
		size_t capacity; \
		size_t size; \
		size_t item_size; \
		float load_factor;	\
		name##_item *table; \
		size_t (*hash_func)(key_type, size_t); \
		int (*compare_func)(key_type, key_type); \
		key_type (*key_alloc_func)(key_type); \
		void (*key_free_func)(key_type); \
	} name##_hashmap_entity, *name##_hashmap; \
	\
	/* Generate inner hash key from raw key and hashmap capacity.
	   If function `hash_func` is given, just use it. Otherwise, use `0`.
	 */ \
	inline static size_t name##_real_hash(size_t (*hash_func)(key_type, size_t), \
		key_type key, size_t capacity) \
	{ \
		if (hash_func) \
			return hash_func(key, capacity) % capacity; \
	\
		/* hashmap_error("Error [%2d]: Can not find valid hash function, use `0` instead\n", ERR_USER) */ \
		return 0; \
	} \
	\
	/* Compare two keys.
	   If function 'compare_func' is given, just use it. Otherwise, use `key1 - key2`.
	 */\
	inline static int name##_real_compare(int (*compare_func)(key_type, key_type), \
		key_type key1, key_type key2) \
	{\
		if (compare_func) \
			return compare_func(key1, key2); \
		/* hashmap_error("Error [%2d]: Can not find valid compare function, use `key1 - key2` instead\n", ERR_USER) */ \
		return key1 - key2; \
	}\
	\
	/* Allocate real key.
	   If function `key_alloc_func` is given, juse use it. Otherwise, do nothing.
	 */\
	inline static key_type name##_real_key_alloc(key_type (*key_alloc_func)(key_type), \
		key_type key) \
	{ \
		if (key_alloc_func) \
		{ \
			return key_alloc_func(key); \
		} \
		return key; \
	} \
	\
	/* Free real key.
	   If function `key_free_func` is given, juse use it. Otherwise, do nothing.
	*/\
	inline static void name##_real_key_free(void (*key_free_func)(key_type), key_type key) \
	{ \
		if (key_free_func) \
			key_free_func(key); \
	} \
	\
	/* Set hash function.
	   It requires hashmap is clean. 
	 */\
	int name##_hashmap_set_hash_func(name##_hashmap map, \
		size_t (*hash_func)(key_type, size_t)) \
	{ \
		if (!map) \
		{ \
			hashmap_error("Error [%2d]: Invalid hashmap\n", ERR_SYS) \
			return ERR_SYS; \
		} \
		if (map->table && map->size > 0) \
		{ \
			hashmap_error("Error [%2d]: Hashmap is not clean\n", ERR_SYS) \
			return ERR_SYS; \
		} \
		map->hash_func = hash_func; \
		return OK; \
	} \
	\
	/* Set compare function.
	   It requires hashmap is clean.
	 */\
	int name##_hashmap_set_compare_func(name##_hashmap map, \
		int (*compare_func)(key_type, key_type)) \
	{ \
		if (!map) \
		{ \
			hashmap_error("Error [%2d]: Invalid hashmap\n", ERR_SYS) \
			return ERR_SYS; \
		} \
		if (map->table && map->size > 0) \
		{ \
			hashmap_error("Error [%2d]: Hashmap is not clean\n", ERR_SYS) \
			return ERR_SYS; \
		} \
		map->compare_func = compare_func; \
		return OK; \
	} \
	\
	/* Set key functions.
	   It requires hashmap is clean.
	 */\
	int name##_hashmap_set_key_funcs(name##_hashmap map, \
		key_type (*key_alloc_func)(key_type), void (*key_free_func)(key_type)) \
	{ \
		if (!map) \
		{ \
			hashmap_error("Error [%2d]: Invalid hashmap\n", ERR_SYS) \
			return ERR_SYS; \
		} \
		if (map->table && map->size > 0) \
		{ \
			hashmap_error("Error [%2d]: Hashmap is not clean\n", ERR_SYS) \
			return ERR_SYS; \
		} \
		map->key_alloc_func = key_alloc_func; \
		map->key_free_func = key_free_func; \
		return OK; \
	} \
	\
	/* Create hashmap with specific capacity and load factor.
	   If `capacity` or `load_factor` is not positive, use default value instead.
	 */\
	name##_hashmap name##_hashmap_create(size_t capacity, float load_factor) \
	{ \
		size_t min_capatity; \
		name##_hashmap map = NULL; \
		map = (name##_hashmap) malloc(sizeof(name##_hashmap_entity)); \
		if (!map) \
		{ \
			hashmap_error("Error [%2d]: Can not allocate memory for new hash map\n", ERR_SYS) \
			return NULL; \
		} \
		if (capacity <= 0) \
			capacity = DEF_CAPACITY; \
		else \
		{ \
			min_capatity = MIN_CAPACITY; \
			while (min_capatity < capacity) \
			{ \
				min_capatity <<= 1; \
			} \
			capacity = min_capatity; \
		} \
	\
		map->table = (name##_item *) malloc(sizeof(name##_item) * capacity); \
		if (!map->table) \
		{ \
			free(map); \
			hashmap_error("Error [%2d]: Can not allocate memory for new hash table\n", ERR_SYS) \
			return NULL; \
		} \
		map->capacity = capacity; \
		map->size = 0; \
		map->item_size = 0; \
		if (load_factor <= 0 || load_factor > 1) \
			load_factor = DEF_FACTOR; \
		map->load_factor = load_factor; \
		memset(map->table, 0, sizeof(name##_item) * capacity); \
		map->hash_func = NULL; \
		map->compare_func = NULL; \
		map->key_alloc_func = NULL; \
		map->key_free_func = NULL; \
		return map; \
	} \
	\
	/* Rehash when item_size * load_facotr >= capacity.
	 */\
	int name##_hashmap_rehash(name##_hashmap map) \
	{ \
		if (!map || !map->table || !map->capacity) \
		{ \
			hashmap_error("Error [%2d]: Invalid hash map\n", ERR_USER) \
			return ERR_USER; \
		} \
		size_t new_capacity = map->capacity << 1; \
		name##_item *new_table = (name##_item *) malloc(sizeof(name##_item) * new_capacity); \
		if (!new_table) \
		{ \
			hashmap_error("Error [%2d]: Can not allocate memory for new hash table\n", ERR_SYS) \
			return ERR_SYS; \
		} \
		memset(new_table, 0, sizeof(name##_item) * new_capacity); \
		size_t i; \
		name##_iter it; \
		name##_iter new_it; \
		name##_iter prev; \
		size_t inner_key; \
		size_t size = map->size; \
		size_t new_size = 0; \
		size_t new_item_size = 0; \
		int status = OK; \
		name##_item *release_table; \
		size_t release_capacity; \
		size_t j; \
		size_t release_size; \
		for (i = 0; !status && new_size < size && i < map->capacity; i++) \
		{ \
			if (map->table[i].entities) \
			{\
				it = map->table[i].entities; \
				while (it) \
				{ \
					inner_key = name##_real_hash(map->hash_func, it->key, new_capacity); \
					new_it = new_table[inner_key].entities; \
					if (!new_it) \
					{ \
						/* Use a new table item */ \
						new_it = (name##_iter) malloc(sizeof(name##_entity)); \
						if (!new_it) \
						{ \
							status = ERR_SYS; \
							break; \
						} \
						new_it->key = name##_real_key_alloc(map->key_alloc_func, it->key); \
						new_it->value = it->value; \
						new_it->next = NULL; \
						new_table[inner_key].entities = new_it; \
						new_table[inner_key].collision++; \
						new_size++; \
						new_item_size++; \
					} \
					else \
					{ \
						while (new_it) \
						{ \
							prev = new_it; \
							new_it = new_it->next; \
						} \
						/* Create a entity in origin table item */\
						new_it = (name##_iter) malloc(sizeof(name##_entity)); \
						if (!new_it) \
						{ \
							status = ERR_SYS; \
							break; \
						} \
						new_it->key = name##_real_key_alloc(map->key_alloc_func, it->key); \
						new_it->value = it->value; \
						new_it->next = NULL; \
						prev->next = new_it; \
						new_table[inner_key].collision++; \
						new_size++; \
					} \
					it = it->next; \
				} \
			} \
		} \
		if (!status) \
		{ \
			/* Succeed to rehash, release old table */ \
			release_table = map->table; \
			release_capacity = map->capacity; \
			release_size = map->size; \
			map->table = new_table; \
			map->item_size = new_item_size; \
		} \
		else \
		{ \
			/* Fail to rehash, release new table */ \
			release_table = new_table; \
			release_capacity = new_capacity; \
			release_size = new_size; \
		} \
		/* Release table */ \
		for (i = 0; j < release_size && i < release_capacity; i++) \
		{ \
			if (release_table[i].entities) \
			{\
				it = release_table[i].entities; \
				while (it) \
				{ \
					prev = it->next; \
					name##_real_key_free(map->key_free_func, it->key); \
					free(it); \
					it = prev; \
					j++; \
				} \
			} \
		} \
		free(release_table); \
		return status; \
	} \
	\
	/* Destroy hashmap.
	   [1] Free data entities of each hash table item.
	   [2] Free hash table.
	   [3] Free hashmap itself.
	*/\
	int name##_hashmap_destroy(name##_hashmap *pmap) \
	{ \
		if (!(*pmap) || !(*pmap)->table) \
		{ \
			hashmap_error("Error [%2d]: Invalid hash map\n", ERR_USER) \
			return ERR_USER; \
		} \
		/* Is transaction protection needed? If so, how to do it? */ \
		name##_iter it, tmp; \
		size_t i; \
		size_t j; \
		for(i = 0; j < (*pmap)->size && i < (*pmap)->capacity; i++) \
		{ \
			it = (*pmap)->table[i].entities; \
			while(it) \
			{ \
				tmp = it->next; \
				name##_real_key_free((*pmap)->key_free_func, it->key); \
				free(it); \
				it = tmp; \
				j++; \
			} \
		} \
		free((*pmap)->table); \
		free(*pmap); \
		*pmap = NULL; \
		return OK; \
	} \
	\
	int name##_hashmap_has_key(name##_hashmap map, key_type key) \
	{ \
		return name##_hashmap_get(map, key, NULL); \
	} \
	/* Get the value of specific key.
	   If `key` exists, value will be written into pvalue. Otherwise, do nothing.  
	*/\
	int name##_hashmap_get(name##_hashmap map, key_type key, value_type *pvalue) \
	{ \
		if (!map || !map->table || !map->capacity) \
		{ \
			hashmap_error("Error [%2d]: Invalid hash map\n", ERR_USER) \
			return ERR_USER; \
		} \
		size_t inner_key = name##_real_hash(map->hash_func, key, map->capacity); \
		name##_iter it = map->table[inner_key].entities; \
		if (!it) \
			return ERR_EMPTY; \
		while (it) \
		{ \
			if (!name##_real_compare(map->compare_func, it->key, key)) \
			{ \
				if (pvalue) \
					*pvalue = it->value; \
				return OK; \
			} \
			it = it->next; \
		} \
		return ERR_EMPTY; \
	} \
	\
	/* Put a pair of (key, value).
	   If `key` has already exists, origin `value` will be overwritten.
	 */\
	int name##_hashmap_put(name##_hashmap map, key_type key, value_type value) \
	{ \
		if (!map || !map->table || !map->capacity) \
		{ \
			hashmap_error("Error [%2d]: Invalid hash map\n", ERR_USER) \
			return ERR_USER; \
		} \
		size_t inner_key = name##_real_hash(map->hash_func, key, map->capacity); \
		name##_iter it = map->table[inner_key].entities; \
		if (!it) \
		{ \
			/* Use a new table item */ \
			it = (name##_iter) malloc(sizeof(name##_entity)); \
			if (!it) \
				return ERR_SYS; \
			it->key = name##_real_key_alloc(map->key_alloc_func, key); \
			it->value = value; \
			it->next = NULL; \
			map->table[inner_key].entities = it; \
			map->table[inner_key].collision++; \
			map->size++; \
			map->item_size++; \
			if (map->item_size * map->load_factor >= map->capacity) \
			{ \
				/* Rehash */ \
				name##_hashmap_rehash(map); \
			} \
			return OK; \
		} \
		else \
		{ \
			name##_iter prev; \
			while (it) \
			{ \
				if (!name##_real_compare(map->compare_func, it->key, key)) \
				{ \
					/* Overwite origin value */ \
					it->value = value; \
					return OK; \
				} \
				prev = it; \
				it = it->next; \
			} \
			/* Create a entity in origin table item */\
			it = (name##_iter) malloc(sizeof(name##_entity)); \
			if (!it) \
				return ERR_SYS; \
			it->key = name##_real_key_alloc(map->key_alloc_func, key); \
			it->value = value; \
			it->next = NULL; \
			prev->next = it; \
			map->table[inner_key].collision++; \
			map->size++; \
			return OK; \
		} \
	} \
	\
	/* Remove a pair of (key, value).
	   If `key` doesn't exists, do nothing.
	 */\
	int name##_hashmap_remove(name##_hashmap map, key_type key) \
	{ \
		if (!map || !map->table || !map->capacity) \
		{ \
			hashmap_error("Error [%2d]: Invalid hash map\n", ERR_USER) \
			return ERR_USER; \
		} \
		size_t inner_key = name##_real_hash(map->hash_func, key, map->capacity); \
		name##_iter it = map->table[inner_key].entities; \
		if (!it) \
		{ \
			hashmap_error("Error [%2d]: No such key exists\n", ERR_USER) \
			return ERR_USER; \
		} \
		name##_iter prev = NULL; \
		while (it) \
		{ \
			if (!name##_real_compare(map->compare_func, it->key, key)) \
			{ \
				if (!prev) \
				{ \
					/* Remove link list head*/ \
					map->table[inner_key].entities = it->next; \
					if (!it->next) \
					{ \
						map->item_size--; \
					} \
				} \
				else \
				{ \
					prev->next = it->next; \
				} \
				map->table[inner_key].collision--; \
				map->size--; \
				name##_real_key_free(map->key_free_func, it->key); \
				free(it); \
				return OK; \
			} \
			prev = it; \
			it = it->next; \
		} \
		hashmap_error("Error [%2d]: No such key exists\n", ERR_USER) \
		return ERR_USER; \
	} \
	\
	/* Get hashmap size.
	 */\
	int name##_hashmap_size(name##_hashmap map) \
	{ \
		if (!map || !map->table || !map->capacity) \
		{ \
			hashmap_error("Error [%2d]: Invalid hash map\n", ERR_USER) \
			return ERR_USER; \
		} \
		return map->size; \
	} \
	\
	/* Get hashmap collision mean.
	 */\
	double name##_hashmap_collision_mean(name##_hashmap map) \
	{ \
		if (!map || !map->table || !map->capacity) \
		{ \
			hashmap_error("Error [%2d]: Invalid hash map\n", ERR_USER) \
			return ERR_USER; \
		} \
		size_t i; \
		long total = 0; \
		for (i = 0; i < map->capacity; i++) \
		{ \
			if (map->table[i].collision > 0) \
				total += map->table[i].collision - 1; \
		} \
		return 1.0 * total / map->item_size; \
	} \
	\
	/* Get hashmap collision variance
	 */\
	double name##_hashmap_collision_variance(name##_hashmap map) \
	{ \
		if (!map || !map->table || !map->capacity) \
		{ \
			hashmap_error("Error [%2d]: Invalid hash map\n", ERR_USER) \
			return ERR_USER; \
		} \
		double mean = name##_hashmap_collision_mean(map); \
		size_t i; \
		double diff; \
		double total = 0; \
		for (i = 0; i < map->capacity; i++) \
		{ \
			if (map->table[i].collision > 0) \
			{ \
				diff = map->table[i].collision - 1 - mean; \
				total += diff * diff; \
			} \
		} \
		return total / map->item_size; \
	} \
	/* Get hash iterator.
	 */\
	name##_iter name##_hashmap_iter(name##_hashmap map) \
	{ \
		if (!map || !map->table || !map->capacity || !map->size) \
		{ \
			hashmap_error("Error [%2d]: Invalid hash map\n", ERR_USER) \
			return NULL; \
		} \
		size_t i; \
		for (i = 0; i < map->capacity; i++) \
		{ \
			if (map->table[i].entities) \
				return map->table[i].entities; \
		} \
		return NULL; \
	} \
	/* Get next iterator.
	   [Warn] If current iterator has been freed, unkown error will happen.
	*/\
	name##_iter name##_hashmap_next(name##_hashmap map, name##_iter it) \
	{ \
		if (!map || !map->table || !map->capacity || !it) \
		{ \
			hashmap_error("Error [%2d]: Invalid hash map\n", ERR_USER) \
			return NULL; \
		} \
		if (it->next) \
			return it->next; \
		size_t inner_key = name##_real_hash(map->hash_func, it->key, map->capacity); \
		size_t i; \
		for (i = inner_key + 1; i < map->capacity; i++) \
		{ \
			if (map->table[i].entities) \
				return map->table[i].entities; \
		} \
		return NULL; \
	} \
	\
	/* Get key and value of current iterator.
	   You can set `pkey` or `pvalue` to NULL if you don't need it. 
	   [Warn] If current iterator has been freed, unkown error will happen.
	*/\
	int name##_hashmap_iter_get(name##_iter it, key_type *pkey, value_type *pvalue) \
	{\
		if (!it) \
		{ \
			hashmap_error("Error [%2d]: Invalid iterator\n", ERR_USER) \
			return ERR_USER; \
		} \
		if (pkey) \
			*pkey = it->key; \
		if (pvalue) \
			*pvalue = it->value; \
		return OK; \
	} \
	\
	/* Set value of current iterator.
	   [Warn] If current iterator has been freed, unkown error will happen.
	*/\
	int name##_hashmap_iter_put(name##_iter it, value_type value) \
	{\
		if (!it) \
		{ \
			hashmap_error("Error [%2d]: Invalid iterator\n", ERR_USER) \
			return ERR_USER; \
		} \
		it->value = value; \
		return OK; \
	}\
	\
	/* Do foreach with specific function `func` and optional extral arguments.
	   Exec `func(key, value, args...)` for each pair of (key, value).
	   If hashmap size is changed during foreach, it will stop immediately.
	*/\
	int name##_hashmap_foreach(name##_hashmap map, \
		int (*func)(key_type, value_type, void *), void *args) \
	{\
		if(!map || !map->table || !map->capacity || !map->size) \
		{ \
			hashmap_error("Error [%2d]: Invalid hash map\n", ERR_USER) \
			return ERR_USER; \
		} \
		if (!func) \
		{ \
			hashmap_error("Error [%2d]: Invalid func\n", ERR_USER) \
			return ERR_USER; \
		} \
		size_t i; \
		name##_iter it; \
		size_t size = map->size; \
		for (i = 0; i < map->capacity; i++) \
		{ \
			if (map->table[i].entities) \
			{\
				it = map->table[i].entities; \
				while (it) \
				{ \
					if (func(it->key, it->value, args) || map->size != size) \
					{ \
						hashmap_error("Error [%2d]: User requests to quit, or hashmap has been changed\n", ERR_USER) \
						return ERR_USER; \
					} \
					it = it->next; \
				} \
			} \
		} \
		return OK; \
	} \

/* Some default functions */

size_t str_hash(const char *key, size_t capacity)
{
	char *p = (char *)key;
	size_t hash = 0;
	for (p; *p; p++)
	{
		hash += *p;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
}

const char * str_key_alloc(const char *key)
{
	return strdup(key);
}

void str_key_free(const char *key)
{
	free((void *)key);
}

#endif
