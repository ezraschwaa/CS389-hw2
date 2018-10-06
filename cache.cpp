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


constexpr Index INIT_ENTRY_CAPACITY = 128;
constexpr double LOAD_FACTOR = .5;
constexpr Index INIT_HASH_TABLE_CAPACITY = static_cast<Index>(INIT_ENTRY_CAPACITY/LOAD_FACTOR);

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
	Index entry_total;
	Index dead_total;//records deleted entries
	mem_unit* mem_arena;//we can do a joint allocation to make the cache serializeable
	Book entry_book;
	Hash_func hash;
	Evictor evictor;
};


inline constexpr Index get_hash_table_capacity(Index entry_capacity) {
	//let e = INIT_ENTRY_CAPACITY
	//let h = INIT_HASH_TABLE_CAPACITY
	//assert entry_capacity/e == hash_table_capacity/h
	//so h*entry_capacity/e == h*(hash_table_capacity/h)
	//so h*entry_capacity/e == (h*hash_table_capacity)/h
	//so h*entry_capacity/e == hash_table_capacity
	return INIT_HASH_TABLE_CAPACITY*(entry_capacity/INIT_ENTRY_CAPACITY);
}
inline constexpr bool is_exceeding_load(Index entry_total, Index dead_total, Index entry_capacity) {
	bool is_exceed_entry = entry_total >= entry_capacity;
	bool is_exceed_load_factor = entry_total + dead_total >= LOAD_FACTOR*get_hash_table_capacity(entry_capacity);
	return is_exceed_entry or is_exceed_load_factor;
}
constexpr Index TOTAL_STEPS = 8;//must be power of 2
inline constexpr Index get_step_size(Index key_hash, Index hash_table_capacity) {
	Index n = key_hash%TOTAL_STEPS;
	return n*(hash_table_capacity/TOTAL_STEPS) + 1;
}


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


constexpr char NULL_TERMINATOR = 0;
Index find_key_size(Key_ptr key) {//Includes the null terminator in the size
	Index size = 0;
	while(key[size] != NULL_TERMINATOR) {
		size += 1;
	}
	size += 1;
	return size;
}
bool are_keys_equal(Key_ptr key0, Key_ptr key1) {//must be null terminated
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


inline Evict_item_locator get_locator(Cache* cache) {
	Evict_item_locator loc;
	loc.abs_first = static_cast<void*>(&read_book(&cache->entry_book, 0)->evict_item);
	loc.step_size = sizeof(Page);
	return loc;
}
inline constexpr Index* get_bookmarks(mem_unit* mem_arena, Index entry_capacity) {
	return reinterpret_cast<Index*>(mem_arena);
}
inline constexpr Index* get_hashes(mem_unit* mem_arena, Index entry_capacity) {
	auto hash_table_capacity = get_hash_table_capacity(entry_capacity);
	return reinterpret_cast<Index*>(mem_arena + sizeof(Index)*hash_table_capacity);
}
inline constexpr Key_ptr* get_keys(mem_unit* mem_arena, Index entry_capacity) {
	auto hash_table_capacity = get_hash_table_capacity(entry_capacity);
	return reinterpret_cast<Key_ptr*>(mem_arena + 2*sizeof(Index)*hash_table_capacity);
}
inline constexpr Page* get_pages(mem_unit* mem_arena, Index entry_capacity) {
	auto hash_table_capacity = get_hash_table_capacity(entry_capacity);
	auto size_of_hash_table = (2*sizeof(Index) + sizeof(Key_ptr))*hash_table_capacity;
	return reinterpret_cast<Page*>(mem_arena + size_of_hash_table);
}
inline constexpr void* get_evict_data(mem_unit* mem_arena, Index entry_capacity) {
	auto hash_table_capacity = get_hash_table_capacity(entry_capacity);
	auto size_of_hash_table = (2*sizeof(Index) + sizeof(Key_ptr))*hash_table_capacity;
	auto size_of_book = sizeof(Page)*entry_capacity;
	return reinterpret_cast<void*>(mem_arena + size_of_hash_table + size_of_book);
}

inline mem_unit* allocate(Index entry_capacity, evictor_type policy) {
	auto hash_table_capacity = get_hash_table_capacity(entry_capacity);
	auto size_of_hash_table = (2*sizeof(Index) + sizeof(Key_ptr))*hash_table_capacity;
	auto size_of_book = sizeof(Page)*entry_capacity;
	auto size_of_evictor = get_mem_size_of_evictor(policy, entry_capacity);
	return new mem_unit[size_of_hash_table + size_of_book + size_of_evictor];
}

constexpr Key_ptr DELETED = &static_cast<Key_ptr>(NULL)[1];//stupid way of getting an invalid address as a constant
constexpr Index KEY_NOT_FOUND = -1;

inline Index find_key(Cache* cache, Key_ptr key) {
	auto const hash_table_capacity = get_hash_table_capacity(cache->entry_capacity);
	auto keys = get_keys(cache->mem_arena, cache->entry_capacity);
	auto key_hashes = get_hashes(cache->mem_arena, cache->entry_capacity);
	auto const key_hash = cache->hash(key);
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

inline void remove_entry(Cache* cache, Index i) {//all entries get removed through here
	auto keys = get_keys(cache->mem_arena, cache->entry_capacity);
	auto bookmarks = get_bookmarks(cache->mem_arena, cache->entry_capacity);
	auto evict_data = get_evict_data(cache->mem_arena, cache->entry_capacity);
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
	cache->dead_total += 1;
	remove_evict_item(evictor, bookmark, &entry->evict_item, get_locator(cache), evict_data);
	free_book_page(entry_book, bookmark);
}
inline void remove_entry_from_bookmark(Cache* cache, Index bookmark) {
	auto entry_book = &cache->entry_book;
	Entry* entry = read_book(entry_book, bookmark);
	remove_entry(cache, entry->cur_i);
}

inline void update_mem_size(Cache* cache, Index mem_change) {
	auto evict_data = get_evict_data(cache->mem_arena, cache->entry_capacity);
	auto const evictor = &cache->evictor;
	cache->mem_total += mem_change;
	while(cache->mem_total > cache->mem_capacity) {//Evict
		Index bookmark = get_evict_item(evictor, get_locator(cache), evict_data);
		remove_entry_from_bookmark(cache, bookmark);
	}
}

inline void grow_cache_size(Cache* cache) {
	//assert entry_capacity/INITIAL_BOOK_CAPACITY == hash_table_capacity/INIT_ENTRY_CAPACITY
	auto const pre_capacity = cache->entry_capacity;
	auto const new_capacity = 2*pre_capacity;
	cache->entry_capacity = new_capacity;

	auto pre_mem_arena = cache->mem_arena;
	auto pre_keys = get_keys(cache->mem_arena, cache->entry_capacity);
	auto pre_key_hashes = get_hashes(cache->mem_arena, cache->entry_capacity);
	auto pre_bookmarks = get_bookmarks(cache->mem_arena, cache->entry_capacity);
	auto entry_book = &cache->entry_book;

	auto mem_arena = allocate(new_capacity, cache->evictor.policy);
	cache->mem_arena = mem_arena;
	copy_book(&cache->entry_book, get_pages(cache->mem_arena, cache->entry_capacity), pre_capacity);

	auto keys = get_keys(cache->mem_arena, cache->entry_capacity);
	auto key_hashes = get_hashes(cache->mem_arena, cache->entry_capacity);
	auto bookmarks = get_bookmarks(cache->mem_arena, cache->entry_capacity);

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


Cache* create_cache(Index max_mem, evictor_type policy, Hash_func hash) {
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
	auto const hash_table_capacity = get_hash_table_capacity(cache->entry_capacity);
	auto keys = get_keys(cache->mem_arena, cache->entry_capacity);
	auto bookmarks = get_bookmarks(cache->mem_arena, cache->entry_capacity);
	auto entry_book = &cache->entry_book;

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

void cache_set(Cache* cache, Key_ptr key, Value_ptr val, Index val_size) {
	auto const entry_capacity = cache->entry_capacity;
	auto const hash_table_capacity = get_hash_table_capacity(entry_capacity);
	auto keys = get_keys(cache->mem_arena, cache->entry_capacity);
	auto key_hashes = get_hashes(cache->mem_arena, cache->entry_capacity);
	auto bookmarks = get_bookmarks(cache->mem_arena, cache->entry_capacity);
	auto evict_data = get_evict_data(cache->mem_arena, cache->entry_capacity);
	auto entry_book = &cache->entry_book;
	auto const evictor = &cache->evictor;
	auto const key_hash = cache->hash(key);

	mem_unit* val_copy = new mem_unit[val_size];
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
				touch_evict_item(evictor, expected_i, &entry->evict_item, get_locator(cache), evict_data);
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
	add_evict_item(evictor, bookmark, &entry->evict_item, get_locator(cache), evict_data);

	keys[new_i] = key_copy;
	key_hashes[new_i] = key_hash;
	bookmarks[new_i] = bookmark;
	if(is_exceeding_load(cache->entry_total, cache->dead_total, entry_capacity)) {
		grow_cache_size(cache);
	}
}

Value_ptr cache_get(Cache* cache, Key_ptr key, Index* val_size) {
	auto bookmarks = get_bookmarks(cache->mem_arena, cache->entry_capacity);
	auto evict_data = get_evict_data(cache->mem_arena, cache->entry_capacity);
	auto entry_book = &cache->entry_book;
	auto const evictor = &cache->evictor;

	Index i = find_key(cache, key);
	if(i == KEY_NOT_FOUND) {
		return NULL;
	} else {
		auto bookmark = bookmarks[i];
		Entry* entry = read_book(entry_book, bookmark);
		touch_evict_item(evictor, bookmark, &entry->evict_item, get_locator(cache), evict_data);
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


Mem_array serialize_cache(Cache* cache) {
	auto const entry_capacity = cache->entry_capacity;
	auto const hash_table_capacity = get_hash_table_capacity(entry_capacity);
	auto keys = get_keys(cache->mem_arena, cache->entry_capacity);
	auto key_hashes = get_hashes(cache->mem_arena, cache->entry_capacity);
	auto bookmarks = get_bookmarks(cache->mem_arena, cache->entry_capacity);
	//pages
	auto evict_data = get_evict_data(cache->mem_arena, cache->entry_capacity);
	auto entry_book = &cache->entry_book;
	auto const evictor = &cache->evictor;

	Mem_array ret;
	// ret.data;//--<--
	// ret.size;//--<--

	auto key_mem_size = 0;
	auto value_mem_size = cache->mem_total;
	auto entry_total = cache->entry_total;

	auto entries_left = entry_total;
	for(Index i = 0; entries_left <= 0; i += 1) {
		Key_ptr key = keys[i];
		if(key != NULL and key != DELETED) {
			entries_left -= 1;
			auto key_hash = key_hashes[i];
			auto bookmark = bookmarks[i];
			key_mem_size += find_key_size(key);
		}
	}
	// auto hash_table_capacity = get_hash_table_capacity(entry_capacity);
	// auto size_of_hash_table = (2*sizeof(Index) + sizeof(Key_ptr))*hash_table_capacity;
	auto size_of_book = sizeof(Page)*entry_capacity;
	auto size_of_evictor = get_mem_size_of_evictor(evictor->policy, entry_capacity);

	Index total_size = sizeof(Cache);
}

cache_type deserialize_cache(Mem_array arr) {

}
