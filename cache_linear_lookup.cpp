//By Monica Moniot
#include <stdlib.h>
#include <cstring>
#include <cassert>
#include <type_traits>
#include "cache.h"

using mem_unit = uint8_t;

using Cache = cache_obj;
using Key_ptr = key_type;
using Value_ptr = val_type;
using Index = index_type;
using Hash_func = hash_func;
using Evictor = evictor_type;

struct cache_obj {//Definition of Cache
	Index mem_capacity;
	Index mem_size;
	Index entry_capacity;
	Index entry_size;
	Hash_func hash;
	Evictor evictor;
	mem_unit** values;//we can do a joint allocation to make the cache serializeable
	Index* value_sizes;
	Key_ptr* keys;
};


char NULL_TERMINATOR = 0;
Index find_key_size(Key_ptr key) {//Includes the null terminator for the size
	Index size = 0;
	while(key[size] != NULL_TERMINATOR) {
		size += 1;
	}
	size += 1;
	return size;
}
bool are_keys_equal(Key_ptr key0, Key_ptr key1) {
	Index i = 0;
	while(true) {
		auto c = key0[i];
		if(c != key1[i]) {
			return false;
		}
		if(c == NULL_TERMINATOR) {
			return true;
		}
		i += 1;
	}
}


void inline remove_entry(Cache* cache, Index i) {
	auto values = cache->values;
	auto value_sizes = cache->value_sizes;
	auto keys = cache->keys;

	cache->mem_size -= value_sizes[i];
	cache->entry_size -= 1;
	delete values[i];

	delete keys[i];
}


Cache* create_cache(Index max_mem, Evictor evictor, Hash_func hash) {
	Cache* cache = new Cache;
	cache->mem_capacity = max_mem;
	cache->mem_size = 0;
	cache->entry_capacity = max_mem;
	cache->entry_size = 0;
	cache->values = new mem_unit*[max_mem];
	cache->value_sizes = new Index[max_mem];
	cache->keys = new Key_ptr[max_mem];
	cache->hash = hash;
	cache->evictor = evictor;
	return cache;
}

void cache_set(Cache* cache, Key_ptr key, Value_ptr val, Index val_size) {
	assert(val_size > 0);
	Index key_size = find_key_size(key);
	auto const mem_capacity = cache->mem_capacity;
	auto values = cache->values;
	auto value_sizes = cache->value_sizes;
	auto keys = cache->keys;
	// auto const hash = cache->hash;
	// auto const evictor = cache->evictor;
	mem_unit* val_copy = new mem_unit[val_size];
	memcpy(val_copy, val, val_size);
	//check if key is in cache
	for(Index i = 0; i < cache->entry_size; i += 1) {//cache hit
		Key_ptr cur_key = keys[i];
		if(are_keys_equal(key, cur_key)) {
			//delete previous value
			cache->mem_size -= value_sizes[i];
			delete values[i];

			//add new value
			values[i] = val_copy;
			value_sizes[i] = val_size;
			cache->mem_size += val_size;
			return;
		}
	}
	//add key
	//make enough space for val
	while(cache->mem_size + val_size > mem_capacity) {
		remove_entry(cache, cache->entry_size - 1);
	}
	Key_ptr key_copy;
	{//copy key by pointer into val_copy
		mem_unit* key_mem = new mem_unit[key_size];
		memcpy(key_mem, key, key_size);
		key_copy = reinterpret_cast<Key_ptr>(key_mem);
	}
	//add new value
	Index new_i = cache->entry_size;
	cache->entry_size += 1;
	values[new_i] = val_copy;
	value_sizes[new_i] = val_size;
	cache->mem_size += val_size;

	keys[new_i] = key_copy;
}

Value_ptr cache_get(Cache* cache, Key_ptr key, Index* val_size) {
	// Index key_size = find_key_size(key);
	// auto const mem_capacity = cache->mem_capacity;
	auto values = cache->values;
	// auto value_sizes = cache->value_sizes;
	auto keys = cache->keys;
	// auto const hash = cache->hash;
	// auto const evictor = cache->evictor;
	//check if key is in cache
	for(Index i = 0; i < cache->entry_size; i += 1) {
		Key_ptr cur_key = keys[i];
		if(are_keys_equal(key, cur_key)) {//cache hit
			return values[i];
		}
	}
	return NULL;
}

void cache_delete(Cache* cache, Key_ptr key) {
	// Index key_size = find_key_size(key);
	// auto const mem_capacity = cache->mem_capacity;
	// auto values = cache->values;
	// auto value_sizes = cache->value_sizes;
	auto keys = cache->keys;
	// auto const hash = cache->hash;
	// auto const evictor = cache->evictor;
	//check if key is in cache
	for(Index i = 0; i < cache->entry_size; i += 1) {
		Key_ptr cur_key = keys[i];
		if(are_keys_equal(key, cur_key)) {//cache hit
			remove_entry(cache, i);
			break;
		}
	}
}

Index cache_space_used(Cache* cache) {
	return cache->mem_size;
}

void destroy_cache(Cache* cache) {
	// auto const mem_capacity = cache->mem_capacity;
	auto values = cache->values;
	auto value_sizes = cache->value_sizes;
	auto keys = cache->keys;
	// auto const hash = cache->hash;
	// auto const evictor = cache->evictor;
	//remove every entry
	for(Index i = 0; i < cache->entry_size; i += 1) {
		remove_entry(cache, i);
	}
	delete values;
	cache->values = NULL;
	delete value_sizes;
	cache->value_sizes = NULL;
	delete keys;
	cache->keys = NULL;
	delete cache;
}
