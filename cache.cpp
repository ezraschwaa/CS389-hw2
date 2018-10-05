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


constexpr Index INITIAL_BOOK_CAPACITY = 128;//must be a power of 2
constexpr double LOAD_FACTOR = 2;//^-1
constexpr Index INITIAL_ENTRY_CAPACITY = LOAD_FACTOR*INITIAL_BOOK_CAPACITY;//must be a power of 2

// Entry* entry = read_book(entry_book, bookmark);
// remove_item (evictor, bookmark, &entry->evict_item, get_locator(cache));
struct Entry {
	Index cur_i;
	mem_unit* value;
	Index value_size;
	Evict_item evict_item;
};

using Bookmark = Index;
using Page_data = Entry;
union Page {
	Bookmark next;
	Page_data data;
};
struct Book {
	Page* pages;
	Bookmark end;
	Bookmark first_unused;
};

struct cache_obj {//Definition of Cache
	Index mem_capacity;
	Index mem_total;
	Index entry_capacity;//this must now be a power of 2
	Index entry_total;//records both alive and deleted entries
	mem_unit* mem_arena;//we can do a joint allocation to make the cache serializeable
	Book entry_book;
	Hash_func hash;
	Evictor evictor;
};


constexpr Bookmark INVALID_PAGE = -1;
inline void create_book(Book* book, Page* pages, Bookmark size) {
	book->first_unused = INVALID_PAGE;
	book->end = 0;
	book->pages = pages;
}
inline void delete_book(Book* book) {
	book->pages = NULL;
}
inline void copy_book(Book* book, Page* new_pages, Bookmark pre_size) {
	memcpy(new_pages, book->pages, pre_size);
	book->pages = new_pages;
}
inline Bookmark alloc_book_page(Book* book) {
	auto pages = book->pages;
	auto bookmark = book->first_unused;
	if(bookmark == INVALID_PAGE) {
		bookmark = book->end;
		book->end += 1;
	} else {
		book->first_unused = pages[bookmark].next;
	}
	return bookmark;
}
inline void free_book_page(Book* book, Bookmark bookmark) {
	auto pages = book->pages;
	auto page = &pages[bookmark];
	page->next = book->first_unused;
	book->first_unused = bookmark;
}
inline Page_data* read_book(Book* book, Bookmark bookmark) {
	return &book->pages[bookmark].data;
}


inline constexpr Index get_book_capacity(Index entry_capacity) {
	//if entry_total >= book_capacity return true
	//assert book_capacity/INITIAL_BOOK_CAPACITY == entry_capacity/INITIAL_ENTRY_CAPACITY
	//so book_capacity == INITIAL_BOOK_CAPACITY*(entry_capacity/INITIAL_ENTRY_CAPACITY)
	//so if entry_total >= INITIAL_BOOK_CAPACITY*(entry_capacity/INITIAL_ENTRY_CAPACITY) return true
	return INITIAL_BOOK_CAPACITY*(entry_capacity/INITIAL_ENTRY_CAPACITY);
}
constexpr bool Is_exceeding_load(Index entry_total, Index entry_capacity) {
	return entry_total >= get_book_capacity(entry_capacity);
}
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


inline Evict_item_locator get_locator(Cache* cache) {
	Evict_item_locator loc;
	loc.abs_first = static_cast<void*>(&read_book(&cache->entry_book, 0)->evict_item);
	loc.step_size = sizeof(Page);
	return loc;
}
inline Index* get_bookmarks(Cache* cache) {
	return reinterpret_cast<Index*>(cache->mem_arena);
}
inline Index* get_hashes(Cache* cache) {
	return reinterpret_cast<Index*>(cache->mem_arena + sizeof(Index)*cache->entry_capacity);
}
inline Key_ptr* get_keys(Cache* cache) {
	return reinterpret_cast<Key_ptr*>(cache->mem_arena + 2*sizeof(Index)*cache->entry_capacity);
}
inline Page* get_pages(Cache* cache) {
	return reinterpret_cast<Page*>(cache->mem_arena + (2*sizeof(Index) + sizeof(Key_ptr))*cache->entry_capacity);
}

inline mem_unit* allocate(Index entry_capacity) {
	auto book_mem_capacity = sizeof(Page)*get_book_capacity(entry_capacity);
	return new mem_unit[(2*sizeof(Index) + sizeof(Key_ptr))*entry_capacity + book_mem_capacity];
}

constexpr Key_ptr DELETED = &(Key_ptr(NULL)[1]);//stupid way of getting an invalid address as a constant
constexpr Index KEY_NOT_FOUND = -1;

inline Index find_key(Cache* cache, Key_ptr key) {
	auto const entry_capacity = cache->entry_capacity;
	auto keys = get_keys(cache);
	auto key_hashes = get_hashes(cache);
	auto const key_hash = cache->hash(key);
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
				return expected_i;
			}
		}
		expected_i = (expected_i + step_size)%entry_capacity;
	}
	return KEY_NOT_FOUND;
}

inline void remove_entry(Cache* cache, Index i) {
	auto keys = get_keys(cache);
	auto bookmarks = get_bookmarks(cache);
	auto entry_book = &cache->entry_book;
	auto const evictor = &cache->evictor;

	auto bookmark = bookmarks[i];
	Entry* entry = read_book(entry_book, bookmark);

	cache->mem_total -= entry->value_size;
	cache->entry_total -= 1;
	delete entry->value;
	entry->value = NULL;

	delete keys[i];
	keys[i] = DELETED;
	remove_item(evictor, bookmark, &entry->evict_item, get_locator(cache));
	free_book_page(entry_book, bookmark);
}
inline void remove_entry_from_bookmark(Cache* cache, Index bookmark) {
	auto entry_book = &cache->entry_book;
	Entry* entry = read_book(entry_book, bookmark);
	remove_entry(cache, entry->cur_i);
}

inline void update_mem_size(Cache* cache, Index mem_change) {
	auto const evictor = &cache->evictor;
	cache->mem_total += mem_change;
	while(cache->mem_total > cache->mem_capacity) {//Evict
		Index bookmark = evict_item(evictor, get_locator(cache));
		remove_entry_from_bookmark(cache, bookmark);
	}
}

void grow_cache_size(Cache* cache) {
	auto const pre_capacity = cache->entry_capacity;
	auto const new_capacity = 2*pre_capacity;

	auto pre_mem_arena = cache->mem_arena;
	auto pre_keys = get_keys(cache);
	auto pre_key_hashes = get_hashes(cache);
	auto pre_bookmarks = get_bookmarks(cache);
	auto entry_book = &cache->entry_book;

	auto mem_arena = allocate(new_capacity);
	cache->mem_arena = mem_arena;
	copy_book(&cache->entry_book, get_pages(cache), get_book_capacity(pre_capacity));

	auto keys = get_keys(cache);
	auto key_hashes = get_hashes(cache);
	auto bookmarks = get_bookmarks(cache);

	Index new_total = 0;
	for(Index i = 0; i < pre_capacity; i += 1) {
		Key_ptr key = pre_keys[i];
		if(key != NULL and key != DELETED) {
			new_total += 1;
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
	cache->entry_total = new_total;

	delete pre_mem_arena;
}


Cache* create_cache(Index max_mem, evictor_type policy, Hash_func hash) {
	Cache* cache = new Cache;
	cache->mem_capacity = max_mem;
	cache->mem_total = 0;
	cache->entry_capacity = INITIAL_ENTRY_CAPACITY;
	cache->entry_total = 0;
	cache->hash = hash;
	cache->mem_arena = allocate(INITIAL_ENTRY_CAPACITY);
	create_book(&cache->entry_book, get_pages(cache), INITIAL_BOOK_CAPACITY);
	create_evictor(&cache->evictor, policy, INITIAL_BOOK_CAPACITY);
	return cache;
}
void destroy_cache(Cache* cache) {
	auto keys = get_keys(cache);
	auto bookmarks = get_bookmarks(cache);
	auto entry_book = &cache->entry_book;
	//remove every entry
	for(Index i = 0; i < cache->entry_capacity; i += 1) {
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

void cache_set(Cache* cache, Key_ptr key, Value_ptr val, Index val_size) {
	auto const entry_capacity = cache->entry_capacity;
	auto keys = get_keys(cache);
	auto key_hashes = get_hashes(cache);
	auto bookmarks = get_bookmarks(cache);
	auto entry_book = &cache->entry_book;
	auto const evictor = &cache->evictor;
	auto const key_hash = cache->hash(key);
	mem_unit* val_copy = new mem_unit[val_size];
	memcpy(val_copy, val, val_size);
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
				Entry* entry = read_book(entry_book, bookmarks[expected_i]);
				update_mem_size(cache, val_size - entry->value_size);
				//delete previous value
				delete entry->value;

				//add new value
				entry->value = val_copy;
				entry->value_size = val_size;
				touch_item(evictor, expected_i, &entry->evict_item, get_locator(cache));
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
	add_item(evictor, bookmark, &entry->evict_item, get_locator(cache));
	keys[new_i] = key_copy;
	key_hashes[new_i] = key_hash;
	bookmarks[new_i] = bookmark;
	if(Is_exceeding_load(cache->entry_total, entry_capacity)) {
		grow_cache_size(cache);
	}
}

Value_ptr cache_get(Cache* cache, Key_ptr key, Index* val_size) {
	auto bookmarks = get_bookmarks(cache);
	auto entry_book = &cache->entry_book;
	auto const evictor = &cache->evictor;
	Index i = find_key(cache, key);
	if(i == KEY_NOT_FOUND) {
		return NULL;
	} else {
		auto bookmark = bookmarks[i];
		Entry* entry = read_book(entry_book, bookmark);
		touch_item(evictor, bookmark, &entry->evict_item, get_locator(cache));
		return entry->value;
	}
}

void cache_delete(Cache* cache, Key_ptr key) {
	Index i = find_key(cache, key);
	if(i != KEY_NOT_FOUND) {
		remove_entry(cache, i);
	}
}

Index cache_space_used(Cache* cache) {
	return cache->mem_total;
}
