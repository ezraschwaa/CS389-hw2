#pragma once
//By Monica Moniot
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include "cache.h"
#include "eviction.h"
using namespace std;

using Cache = cache_obj;
using Key_ptr = key_type;
using Value_ptr = val_type;
using Index = index_type;
using Hash_func = hash_func;

struct Entry {
	Key_ptr key;
	Value_ptr value;
	Index value_size;
	Evict_item evict_item;
};

struct cache_obj {//Definition of Cache
	Index mem_capacity;
	Index mem_total;
	Index entry_capacity;//this must now be a power of 2
	Index entry_total;//records both alive and deleted entries
	Hash_func hash;
	Index* key_hashes;//we can do a joint allocation to make the cache serializeable
	Entry** entries;
	Evictor evictor;
};


constexpr Index TOTAL_STEPS = 8;//must be power of 2
inline constexpr Index get_step_size(Index key_hash, Index entry_capacity) {
	Index n = key_hash%TOTAL_STEPS;
	return n*(entry_capacity/TOTAL_STEPS) + 1;
}

constexpr char NULL_TERMINATOR = 0;
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


constexpr Key_ptr DELETED = &(Key_ptr(NULL)[1]);//stupid way of getting an invalid address as a constant
constexpr Index KEY_NOT_FOUND = -1;
inline Index find_key_i(Cache* cache, Key_ptr key, Index key_hash, bool do_ret_delete = false) {
	auto const entry_capacity = cache->entry_capacity;
	auto keys = cache->keys;
	auto key_hashes = cache->key_hashes;

	Index expected_i = key_hash%entry_capacity;
	Index step_size = get_step_size(key_hash, entry_capacity);
	for(Index count = 0; count < entry_capacity; count += 1) {
		Key_ptr cur_key = keys[expected_i];
		if(cur_key == NULL) {
			break;
		} else if(cur_key == DELETED) {
			if(do_ret_delete) {
				return expected_i;
			}
		} else {
			if(key_hashes[expected_i] == key_hash) {
				if(are_keys_equal(cur_key, key)) {
					return expected_i;
				}
			}
		}
		expected_i = (expected_i + step_size)%entry_capacity;
	}
	return KEY_NOT_FOUND;
}

void grow_cache_size(Cache* cache) {
	auto const pre_capacity = cache->entry_capacity;
	auto pre_values = cache->values;
	auto pre_value_sizes = cache->value_sizes;
	auto pre_keys = cache->keys;
	auto pre_key_hashes = cache->key_hashes;
	auto pre_evict_pids = cache->evict_pids;
	// auto const evictor = cache->evictor;

	auto const new_capacity = 2*pre_capacity;

	Value_ptr* values = new Value_ptr[new_capacity];
	Index* value_sizes = new Index[new_capacity];
	Key_ptr* keys = new Key_ptr[new_capacity];
	Index* key_hashes = new Index[new_capacity];
	Evict_pid* evict_pids = new Evict_pid[new_capacity];

	cache->values = values;
	cache->value_sizes = value_sizes;
	cache->keys = keys;
	cache->key_hashes = key_hashes;


	Index new_total = 0;
	for(Index i = 0; i < pre_capacity; i += 1) {
		Key_ptr key = pre_keys[i];
		if(key != NULL and key != DELETED) {
			new_total += 1;
			Value_ptr val = pre_values[i];
			Index val_size = pre_value_sizes[i];
			Index key_hash = pre_key_hashes[i];
			Evict_pid evict_pid = pre_evict_pids[i];
			//find empty index
			Index i = key_hash%new_capacity;
			Index step_size = get_step_size(key_hash, new_capacity);
			while(true) {
				Key_ptr cur_key = keys[i];
				if(cur_key == NULL) {
					break;
				}
				i = (i + step_size)%new_capacity;
			}
			//write to new cache
			values[i] = val;
			value_sizes[i] = val_size;
			keys[i] = key;
			key_hashes[i] = key_hash;
			evict_pids[i] = evict_pid;
		}
	}
	cache->entry_total = new_total;

	delete pre_values;
	delete pre_value_sizes;
	delete pre_keys;
	delete pre_key_hashes;
	delete pre_evict_pids;
}

inline void remove_entry(Cache* cache, Index i) {
	auto values = cache->values;
	auto value_sizes = cache->value_sizes;
	auto keys = cache->keys;
	auto evict_pids = cache->evict_pids;

	cache->mem_total -= value_sizes[i];
	// cache->entry_total -= 1;
	delete values[i];

	delete keys[i];
	keys[i] = DELETED;
	remove_item(cache->evictor, evict_pids[i]);
}

inline void update_mem_size(Cache* cache, Index mem_change) {
	auto const entry_capacity = cache->entry_capacity;
	auto keys = cache->keys;
	auto key_hashes = cache->key_hashes;
	cache->mem_total += mem_change;
	while(cache->mem_total > cache->mem_capacity) {//Evict
		Index key_hash = evict_item(cache->evictor);

		//find the item to be evicted
		Index expected_i = key_hash%entry_capacity;
		Index step_size = get_step_size(key_hash, entry_capacity);
		for(Index count = 0; count < entry_capacity; count += 1) {
			Key_ptr cur_key = keys[expected_i];
			if(cur_key == NULL) {
				printf("Panic! Attempt to evict a nonexistant item\n");
				printf("Item had key_hash: %d", key_hash);
				count = entry_capacity;
			} else if(cur_key == DELETED) {
				// continue;
			} else if(key_hashes[expected_i] == key_hash) {
				remove_entry(cache, expected_i);
				count = entry_capacity;
			}
			expected_i = (expected_i + step_size)%entry_capacity;
		}
	}
}


constexpr Index INITIAL_ENTRY_CAPACITY = 128;
Cache* create_cache(Index max_mem, evictor_type policy, Hash_func hash) {
	Index entry_capacity = INITIAL_ENTRY_CAPACITY;
	Cache* cache = new Cache;
	cache->mem_capacity = max_mem;
	cache->mem_total = 0;
	cache->entry_capacity = entry_capacity;
	cache->entry_total = 0;
	cache->hash = hash;
	cache->values = new Value_ptr[entry_capacity];
	cache->value_sizes = new Index[entry_capacity];
	cache->keys = new Key_ptr[entry_capacity];
	cache->key_hashes = new Index[entry_capacity];
	cache->evict_pids = new Evict_pid[entry_capacity];
	create_evictor(&cache->evictor, policy, entry_capacity);
	return cache;
}

void cache_set(Cache* cache, Key_ptr key, Value_ptr val, Index val_size) {
	auto const mem_capacity = cache->mem_capacity;
	auto const entry_capacity = cache->entry_capacity;
	auto values = cache->values;
	auto value_sizes = cache->value_sizes;
	auto keys = cache->keys;
	auto key_hashes = cache->key_hashes;
	auto evict_pids = cache->evict_pids;
	auto const hash = cache->hash;
	auto const evictor = cache->evictor;
	Index key_hash = hash(key);
	Value_ptr val_copy;
	{//copy val by pointer into val_copy
		void* val_mem = malloc(val_size);
		memcpy(val_mem, val, val_size);
		val_copy = static_cast<Value_ptr>(val_mem);
	}
	//check if key is in cache
	Index expected_i = key_hash%entry_capacity;
	Index step_size = get_step_size(key_hash, entry_capacity);
	for(Index count = 0; count < entry_capacity; count += 1) {
		Key_ptr cur_key = keys[expected_i];
		if(cur_key == NULL) {
			break;
		} else if(cur_key == DELETED) {
			break;
		} else if(key_hashes[expected_i] == key_hash) {
			if(are_keys_equal(cur_key, key)) {//found key
				update_mem_size(cache, val_size - value_sizes[expected_i]);
				//delete previous value
				delete values[expected_i];

				//add new value
				values[expected_i] = val_copy;
				value_sizes[expected_i] = val_size;
				touch_item(cache->evictor, evict_pids[expected_i]);
				return;
			}
		}
		expected_i = (expected_i + step_size)%entry_capacity;
	}
	Index new_i = expected_i;
	//add key at new_i
	Key_ptr key_copy;
	{//copy key by pointer into val_copy
		Index key_size = find_key_size(key);
		void* key_mem = malloc(key_size);
		memcpy(key_mem, key, key_size);
		key_copy = static_cast<Key_ptr>(key_mem);
	}
	//add new value
	update_mem_size(cache, val_size);
	cache->entry_total += 1;
	values[new_i] = val_copy;
	value_sizes[new_i] = val_size;
	keys[new_i] = key_copy;
	key_hashes[new_i] = key_hash;
	evict_pids[new_i] = add_item(evictor, key_hash);
	if(2*cache->entry_total > entry_capacity) {
		grow_cache_size(cache);
	}
}

Value_ptr cache_get(Cache* cache, Key_ptr key, Index* val_size) {
	// auto const mem_capacity = cache->mem_capacity;
	auto const entry_capacity = cache->entry_capacity;
	auto values = cache->values;
	// auto value_sizes = cache->value_sizes;
	auto keys = cache->keys;
	auto key_hashes = cache->key_hashes;
	auto evict_pids = cache->evict_pids;
	Index key_hash = cache->hash(key);
	//check if key is in cache
	Index expected_i = key_hash%entry_capacity;
	Index step_size = get_step_size(key_hash, entry_capacity);
	for(Index count = 0; count < entry_capacity; count += 1) {
		Key_ptr cur_key = keys[expected_i];
		if(cur_key == NULL) {
			break;
		} else if(cur_key == DELETED) {
			continue;
		} else if(key_hashes[expected_i] == key_hash) {
			if(are_keys_equal(cur_key, key)) {//found key
				touch_item(cache->evictor, evict_pids[expected_i]);
				return values[expected_i];
			}
		}
		expected_i = (expected_i + step_size)%entry_capacity;
	}
	return NULL;
}

void cache_delete(Cache* cache, Key_ptr key) {
	// auto const mem_capacity = cache->mem_capacity;
	auto const entry_capacity = cache->entry_capacity;
	// auto values = cache->values;
	// auto value_sizes = cache->value_sizes;
	auto keys = cache->keys;
	auto key_hashes = cache->key_hashes;
	auto evict_pids = cache->evict_pids;
	auto const hash = cache->hash;
	Index key_hash = hash(key);
	// auto const evictor = cache->evictor;
	//check if key is in cache
	Index expected_i = key_hash%entry_capacity;
	Index step_size = get_step_size(key_hash, entry_capacity);
	for(Index count = 0; count < entry_capacity; count += 1) {
		Key_ptr cur_key = keys[expected_i];
		if(cur_key == NULL) {
			break;
		} else if(cur_key == DELETED) {
			continue;
		} else if(key_hashes[expected_i] == key_hash) {
			if(are_keys_equal(cur_key, key)) {//found key
				remove_entry(cache, expected_i);
			}
		}
		expected_i = (expected_i + step_size)%entry_capacity;
	}
}

Index cache_space_used(Cache* cache) {
	return cache->mem_total;
}

void destroy_cache(Cache* cache) {
	// auto const mem_capacity = cache->mem_capacity;
	auto values = cache->values;
	auto value_sizes = cache->value_sizes;
	auto keys = cache->keys;
	auto key_hashes = cache->key_hashes;
	auto evict_pids = cache->evict_pids;
	//remove every entry
	for(Index i = 0; i < cache->entry_capacity; i += 1) {
		Key_ptr cur_key = keys[i];
		if(cur_key != NULL and cur_key != DELETED) {
			delete values[i];
			delete keys[i];
		}
	}
	delete values;
	cache->values = NULL;
	delete value_sizes;
	cache->value_sizes = NULL;
	delete keys;
	cache->keys = NULL;
	delete key_hashes;
	cache->key_hashes = NULL;
	delete evict_pids;
	cache->evict_pids = NULL;
	delete_evictor(cache->evictor);
	delete cache;
}
