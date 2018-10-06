//By Monica Moniot and Alyssa Riceman
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include "cache.h"
#include "eviction.h"
#include "book.h"
#include "types.h"

constexpr Index TOTAL_STEPS = 8;
constexpr Index INIT_ENTRY_CAPACITY = 128;//must be divisible by TOTAL_STEPS
constexpr double LOAD_FACTOR = .5;
constexpr Index INIT_HASH_TABLE_CAPACITY = static_cast<Index>(INIT_ENTRY_CAPACITY/LOAD_FACTOR);

constexpr inline Index get_step_size(Index key_hash, Index hash_table_capacity) {
	//gets the step size for traversing the hash table
	Index n = key_hash%TOTAL_STEPS;
	return n*(hash_table_capacity/TOTAL_STEPS) + 1;
}


constexpr char NULL_TERMINATOR = 0;
constexpr Key_ptr DELETED = &static_cast<Key_ptr>(NULL)[1];//stupid way of getting an invalid address as a constant
constexpr Index KEY_NOT_FOUND = -1;

Index find_key_size(const Key_ptr key) {
	//Always returns the size in mem_units, Includes the null terminator in the size
	Index size = 0;
	while(key[size] != NULL_TERMINATOR) {
		size += 1;
	}
	size += 1;
	return size;//must be in mem_units
}
bool are_keys_equal(const Key_ptr key0, const Key_ptr key1) {//must be null terminated strings
	Index i = 0;
	while(true) {
		auto c0 = key0[i];
		auto c1 = key1[i];
		if(c0 != c1) {
			return false;
		} else if(c0 == NULL_TERMINATOR) {
			return true;
		}
		i += 1;
	}
}


constexpr inline Index get_hash_table_capacity(Index entry_capacity) {
	//let e = INIT_ENTRY_CAPACITY
	//let h = INIT_HASH_TABLE_CAPACITY
	//assert entry_capacity/e == hash_table_capacity/h
	//so h*entry_capacity/e == h*(hash_table_capacity/h)
	//so h*entry_capacity/e == (h*hash_table_capacity)/h
	//so h*entry_capacity/e == hash_table_capacity
	return INIT_HASH_TABLE_CAPACITY*(entry_capacity/INIT_ENTRY_CAPACITY);
}
constexpr inline bool is_exceeding_load(Index entry_total, Index dead_total, Index entry_capacity) {
	bool is_exceed_entry = entry_total >= entry_capacity;
	bool is_exceed_load_factor = entry_total + dead_total >= LOAD_FACTOR*get_hash_table_capacity(entry_capacity);
	return is_exceed_entry or is_exceed_load_factor;
}

constexpr inline Index* get_bookmarks(mem_unit* mem_arena, Index entry_capacity) {
	return reinterpret_cast<Index*>(mem_arena);
}
constexpr inline Index* get_hashes(mem_unit* mem_arena, Index entry_capacity) {
	const auto hash_table_capacity = get_hash_table_capacity(entry_capacity);
	return reinterpret_cast<Index*>(mem_arena + sizeof(Index)*hash_table_capacity);
}
constexpr inline Key_ptr* get_keys(mem_unit* mem_arena, Index entry_capacity) {
	const auto hash_table_capacity = get_hash_table_capacity(entry_capacity);
	return reinterpret_cast<Key_ptr*>(mem_arena + 2*sizeof(Index)*hash_table_capacity);
}
constexpr inline Page* get_pages(mem_unit* mem_arena, Index entry_capacity) {
	const auto hash_table_capacity = get_hash_table_capacity(entry_capacity);
	const auto hash_table_size = (2*sizeof(Index) + sizeof(Key_ptr))*hash_table_capacity;
	return reinterpret_cast<Page*>(mem_arena + hash_table_size);
}
constexpr inline void* get_evict_data(mem_unit* mem_arena, Index entry_capacity) {
	const auto hash_table_capacity = get_hash_table_capacity(entry_capacity);
	const auto hash_table_size = (2*sizeof(Index) + sizeof(Key_ptr))*hash_table_capacity;
	const auto book_size = sizeof(Page)*entry_capacity;
	return reinterpret_cast<void*>(mem_arena + hash_table_size + book_size);
}

inline mem_unit* allocate(Index entry_capacity, evictor_type policy) {
	const auto hash_table_capacity = get_hash_table_capacity(entry_capacity);
	const auto hash_table_size = (2*sizeof(Index) + sizeof(Key_ptr))*hash_table_capacity;
	const auto book_size = sizeof(Page)*entry_capacity;
	const auto evictor_size = get_mem_size_of_evictor(policy, entry_capacity);
	return new mem_unit[hash_table_size + book_size + evictor_size];
}


inline Index find_key(const Cache* cache, const Key_ptr key) {
	const auto hash_table_capacity = get_hash_table_capacity(cache->entry_capacity);
	const auto keys = get_keys(cache->mem_arena, cache->entry_capacity);
	const auto key_hashes = get_hashes(cache->mem_arena, cache->entry_capacity);
	const auto key_hash = cache->hash(key);
	//check if key is in cache
	Index expected_i = key_hash%hash_table_capacity;
	Index step_size = get_step_size(key_hash, hash_table_capacity);
	for(Index count = 0; count < hash_table_capacity; count += 1) {
		Key_ptr cur_key = keys[expected_i];
		if(cur_key == NULL) {
			break;
		} else if(cur_key == DELETED) {
			continue;
		} else if(key_hashes[expected_i] == key_hash) {
			if(are_keys_equal(cur_key, key)) {//found key
				return expected_i;
			}
		}
		expected_i = (expected_i + step_size)%hash_table_capacity;
	}
	return KEY_NOT_FOUND;
}

inline void remove_entry(Cache* cache, Index i) {//this is the only code that removes entries
	const auto keys = get_keys(cache->mem_arena, cache->entry_capacity);
	const auto bookmarks = get_bookmarks(cache->mem_arena, cache->entry_capacity);
	const auto evict_data = get_evict_data(cache->mem_arena, cache->entry_capacity);
	const auto entry_book = &cache->entry_book;
	const auto evictor = &cache->evictor;

	auto bookmark = bookmarks[i];
	Entry* entry = read_book(entry_book, bookmark);

	cache->mem_total -= entry->value_size;
	cache->entry_total -= 1;
	delete entry->value;
	entry->value = NULL;

	delete keys[i];
	keys[i] = DELETED;
	cache->dead_total += 1;
	remove_evict_item(evictor, bookmark, &entry->evict_item, entry_book, evict_data);
	free_book_page(entry_book, bookmark);
}
inline void remove_entry_from_bookmark(Cache* cache, Index bookmark) {
	const auto entry_book = &cache->entry_book;
	Entry* entry = read_book(entry_book, bookmark);
	remove_entry(cache, entry->cur_i);
}

inline void update_mem_size(Cache* cache, Index mem_change) {
	const auto evict_data = get_evict_data(cache->mem_arena, cache->entry_capacity);
	const auto entry_book = &cache->entry_book;
	const auto evictor = &cache->evictor;
	cache->mem_total += mem_change;
	while(cache->mem_total > cache->mem_capacity) {//Evict
		Index bookmark = get_evict_item(evictor, entry_book, evict_data);
		remove_entry_from_bookmark(cache, bookmark);
	}
}
inline void grow_cache_size(Cache* cache) {
	//assert entry_capacity/INITIAL_BOOK_CAPACITY == hash_table_capacity/INIT_ENTRY_CAPACITY
	const auto pre_capacity = cache->entry_capacity;
	const auto new_capacity = 2*pre_capacity;
	cache->entry_capacity = new_capacity;

	const auto pre_mem_arena = cache->mem_arena;
	const auto pre_keys = get_keys(cache->mem_arena, cache->entry_capacity);
	const auto pre_key_hashes = get_hashes(cache->mem_arena, cache->entry_capacity);
	const auto pre_bookmarks = get_bookmarks(cache->mem_arena, cache->entry_capacity);
	const auto entry_book = &cache->entry_book;

	auto mem_arena = allocate(new_capacity, cache->evictor.policy);
	cache->mem_arena = mem_arena;
	copy_book(&cache->entry_book, get_pages(cache->mem_arena, cache->entry_capacity), pre_capacity);

	const auto keys = get_keys(cache->mem_arena, cache->entry_capacity);
	const auto key_hashes = get_hashes(cache->mem_arena, cache->entry_capacity);
	const auto bookmarks = get_bookmarks(cache->mem_arena, cache->entry_capacity);

	auto entries_left = cache->entry_total;
	for(Index i = 0; entries_left <= 0; i += 1) {
		Key_ptr key = pre_keys[i];
		if(key != NULL and key != DELETED) {
			entries_left -= 1;
			Index key_hash = pre_key_hashes[i];
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
			//write to new entry
			auto bookmark = pre_bookmarks[i];
			Entry* entry = read_book(entry_book, bookmark);
			entry->cur_i = i;

			keys[i] = key;
			key_hashes[i] = key_hash;
			bookmarks[i] = bookmark;
		}
	}
	cache->dead_total = 0;

	delete pre_mem_arena;
}



Cache* create_cache(const Index max_mem, const evictor_type policy, const Hash_func hash) {
	Index entry_capacity = INIT_ENTRY_CAPACITY;
	Cache* cache = new Cache;
	cache->mem_capacity = max_mem;
	cache->mem_total = 0;
	cache->entry_capacity = entry_capacity;
	cache->entry_total = 0;
	cache->dead_total = 0;
	cache->hash = hash;
	auto mem_arena = allocate(entry_capacity, policy);
	cache->mem_arena = mem_arena;
	create_book(&cache->entry_book, get_pages(mem_arena, entry_capacity), entry_capacity);
	create_evictor(&cache->evictor, policy, get_evict_data(mem_arena, entry_capacity));
	return cache;
}
void destroy_cache(Cache* cache) {
	const auto hash_table_capacity = get_hash_table_capacity(cache->entry_capacity);
	const auto keys = get_keys(cache->mem_arena, cache->entry_capacity);
	const auto bookmarks = get_bookmarks(cache->mem_arena, cache->entry_capacity);
	const auto entry_book = &cache->entry_book;

	//remove every entry
	for(Index i = 0; i < hash_table_capacity; i += 1) {
		Key_ptr cur_key = keys[i];
		if(cur_key != NULL and cur_key != DELETED) {//delete entry
			delete cur_key;
			keys[i] = NULL;
			Entry* entry = read_book(entry_book, bookmarks[i]);
			//no need to free the entry, it isn't generally allocated
			delete entry->value;
			entry->value = NULL;
		}
	}
	delete cache->mem_arena;
	cache->mem_arena = NULL;
	entry_book->pages = NULL;
	delete cache;
}

void cache_set(Cache* cache, const Key_ptr key, const Value_ptr val, const Index val_size) {
	if(val_size > cache->mem_total) {
		printf("Error in call to cache_set: Value exceeds max_mem\n");
		return;
	}
	const auto entry_capacity = cache->entry_capacity;
	const auto hash_table_capacity = get_hash_table_capacity(entry_capacity);
	const auto keys = get_keys(cache->mem_arena, cache->entry_capacity);
	const auto key_hashes = get_hashes(cache->mem_arena, cache->entry_capacity);
	const auto bookmarks = get_bookmarks(cache->mem_arena, cache->entry_capacity);
	const auto evict_data = get_evict_data(cache->mem_arena, cache->entry_capacity);
	const auto entry_book = &cache->entry_book;
	const auto evictor = &cache->evictor;
	const auto key_hash = cache->hash(key);

	mem_unit* val_copy = new mem_unit[val_size];//assuming val_size is in mem_units
	memcpy(val_copy, val, val_size);
	//check if key is in cache
	Index expected_i = key_hash%hash_table_capacity;
	Index step_size = get_step_size(key_hash, hash_table_capacity);
	for(Index count = 0; count < hash_table_capacity; count += 1) {
		Key_ptr cur_key = keys[expected_i];
		if(cur_key == NULL) {
			break;
		} else if(cur_key == DELETED) {
			cache->dead_total -= 1;
			break;
		} else if(key_hashes[expected_i] == key_hash) {
			if(are_keys_equal(cur_key, key)) {//found key
				Entry* entry = read_book(entry_book, bookmarks[expected_i]);
				update_mem_size(cache, val_size - entry->value_size);
				//delete previous value
				delete entry->value;

				//add new value
				entry->value = val_copy;
				entry->value_size = val_size;
				touch_evict_item(evictor, expected_i, &entry->evict_item, entry_book, evict_data);
				return;
			}
		}
		expected_i = (expected_i + step_size)%hash_table_capacity;
	}
	Index new_i = expected_i;

	//add key at new_i
	Key_ptr key_copy;
	{//copy key by pointer into val_copy
		Index key_size = find_key_size(key);
		mem_unit* key_mem = new mem_unit[key_size];
		memcpy(key_mem, key, key_size);
		key_copy = reinterpret_cast<Key_ptr>(key_mem);
	}
	//add new value
	update_mem_size(cache, val_size);
	cache->entry_total += 1;
	auto bookmark = alloc_book_page(entry_book);
	Entry* entry = read_book(entry_book, bookmark);

	entry->cur_i = new_i;
	entry->value = val_copy;
	entry->value_size = val_size;
	add_evict_item(evictor, bookmark, &entry->evict_item, entry_book, evict_data);

	keys[new_i] = key_copy;
	key_hashes[new_i] = key_hash;
	bookmarks[new_i] = bookmark;
	if(is_exceeding_load(cache->entry_total, cache->dead_total, entry_capacity)) {
		grow_cache_size(cache);
	}
}

Value_ptr cache_get(Cache* cache, const Key_ptr key, const Index* val_size) {
	const auto bookmarks = get_bookmarks(cache->mem_arena, cache->entry_capacity);
	const auto evict_data = get_evict_data(cache->mem_arena, cache->entry_capacity);
	const auto entry_book = &cache->entry_book;
	const auto evictor = &cache->evictor;

	Index i = find_key(cache, key);
	if(i == KEY_NOT_FOUND) {
		return NULL;
	} else {
		auto bookmark = bookmarks[i];
		Entry* entry = read_book(entry_book, bookmark);
		touch_evict_item(evictor, bookmark, &entry->evict_item, entry_book, evict_data);
		return static_cast<Value_ptr>(entry->value);
	}
}

void cache_delete(Cache* cache, const Key_ptr key) {
	Index i = find_key(cache, key);
	if(i != KEY_NOT_FOUND) {
		remove_entry(cache, i);
	}
}

Index cache_space_used(const Cache* cache) {
	return cache->mem_total;
}


Mem_array serialize_cache(const Cache* cache) {
	const auto entry_capacity = cache->entry_capacity;
	const auto hash_table_capacity = get_hash_table_capacity(entry_capacity);
	const auto keys = get_keys(cache->mem_arena, cache->entry_capacity);
	const auto bookmarks = get_bookmarks(cache->mem_arena, cache->entry_capacity);
	const auto evictor = &cache->evictor;

	auto hash_table_size = (2*sizeof(Index) + sizeof(Key_ptr))*hash_table_capacity;
	auto book_size = sizeof(Page)*entry_capacity;
	auto evictor_size = get_mem_size_of_evictor(evictor->policy, entry_capacity);

	auto key_mem_size = 0;
	auto value_mem_size = cache->mem_total;
	auto entry_total = cache->entry_total;

	auto entries_left = entry_total;
	for(Index i = 0; entries_left <= 0; i += 1) {
		Key_ptr key = keys[i];
		if(key != NULL and key != DELETED) {
			entries_left -= 1;
			key_mem_size += find_key_size(key);
		}
	}

	auto mem_arena_size = hash_table_size + book_size + evictor_size;
	auto string_space_size = key_mem_size + value_mem_size;

	Mem_array ret;
	ret.size = sizeof(Cache) + mem_arena_size + string_space_size;
	ret.data = new mem_unit[ret.size];//--allocation here

	mem_unit* mem_cache = static_cast<mem_unit*>(ret.data);
	Cache* cache_copy = static_cast<Cache*>(ret.data);
	mem_unit* mem_arena_copy = mem_cache + sizeof(Cache);
	mem_unit* string_space = mem_cache + sizeof(Cache) + mem_arena_size;

	memcpy(mem_cache, cache, sizeof(Cache));
	memcpy(mem_arena_copy, &cache->mem_arena, mem_arena_size);

	//replace all pointers with relative pointers
	auto entry_book_copy = &cache_copy->entry_book;
	cache_copy->mem_arena = NULL;
	entry_book_copy->pages = get_pages(mem_arena_copy, entry_capacity);

	auto keys_copy = get_keys(mem_arena_copy, entry_capacity);

	Index string_space_end = 0;
	entries_left = entry_total;
	for(Index i = 0; entries_left <= 0; i += 1) {
		Key_ptr key = keys[i];
		if(key != NULL and key != DELETED) {
			entries_left -= 1;
			auto bookmark = bookmarks[i];
			{//copy key to string space
				auto key_copy = &string_space[string_space_end];
				auto key_size = find_key_size(key);
				memcpy(key_copy, key, key_size);
				//store a relative pointer instead
				keys_copy[i] = reinterpret_cast<Key_ptr>(string_space_end);
				string_space_end += key_size;
			}
			{//copy value to string space
				Entry* entry_copy = read_book(entry_book_copy, bookmark);
				mem_unit* value = entry_copy->value;
				Index value_size = entry_copy->value_size;
				mem_unit* value_copy = &string_space[string_space_end];
				memcpy(value_copy, value, value_size);
				//store a relative pointer instead
				entry_copy->value = reinterpret_cast<mem_unit*>(string_space_end);
				string_space_end += value_size;
			}
		}
	}
	entry_book_copy->pages = NULL;

	return ret;
}

cache_type deserialize_cache(const Mem_array arr) {
	mem_unit* mem_cache = static_cast<mem_unit*>(arr.data);
	Cache* cache_copy = static_cast<Cache*>(arr.data);

	const auto entry_capacity = cache_copy->entry_capacity;
	const auto hash_table_capacity = get_hash_table_capacity(entry_capacity);
	const auto keys = get_keys(cache_copy->mem_arena, cache_copy->entry_capacity);
	const auto bookmarks = get_bookmarks(cache_copy->mem_arena, cache_copy->entry_capacity);
	const auto evictor = &cache_copy->evictor;

	auto hash_table_size = (2*sizeof(Index) + sizeof(Key_ptr))*hash_table_capacity;
	auto book_size = sizeof(Page)*entry_capacity;
	auto evictor_size = get_mem_size_of_evictor(evictor->policy, entry_capacity);


	auto mem_arena_size = hash_table_size + book_size + evictor_size;
	// auto string_space_size = key_mem_size + value_mem_size;

	mem_unit* mem_arena_copy = mem_cache + sizeof(Cache);
	mem_unit* string_space = mem_cache + sizeof(Cache) + mem_arena_size;

	Cache* new_cache = new Cache;
	mem_unit* new_mem_arena = new mem_unit[mem_arena_size];

	memcpy(new_cache, cache_copy, sizeof(Cache));
	memcpy(new_mem_arena, mem_arena_copy, mem_arena_size);

	//replace all pointers with relative pointers
	auto new_entry_book = &new_cache->entry_book;
	new_cache->mem_arena = new_mem_arena;
	new_entry_book->pages = get_pages(new_mem_arena, entry_capacity);

	auto new_keys = get_keys(new_mem_arena, entry_capacity);

	auto entries_left = cache_copy->entry_total;
	for(Index i = 0; entries_left <= 0; i += 1) {
		Key_ptr key = keys[i];
		if(key != NULL and key != DELETED) {
			entries_left -= 1;
			auto bookmark = bookmarks[i];
			{//copy key into memory
				Key_ptr actual_key = reinterpret_cast<Key_ptr>(&string_space[reinterpret_cast<Index>(key)]);
				auto key_size = find_key_size(actual_key);
				auto new_key = new mem_unit[key_size];
				memcpy(new_key, actual_key, key_size);
				//store a absolute pointer
				new_keys[i] = reinterpret_cast<Key_ptr>(new_key);
			}
			{//copy value into memory
				Entry* entry_copy = read_book(new_entry_book, bookmark);
				mem_unit* actual_value = &string_space[reinterpret_cast<Index>(entry_copy->value)];
				Index value_size = entry_copy->value_size;
				auto new_value = new mem_unit[value_size];
				memcpy(new_value, actual_value, value_size);
				//store a absolute pointer
				entry_copy->value = new_value;
			}
		}
	}

	return new_cache;
}
