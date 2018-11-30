//By Monica Moniot and Alyssa Riceman
#ifndef TYPES_H
#define TYPES_H
#include "cache.h"

using byte = uint8_t;//this must have the size of a unit of memory (a byte)
using uint_ptr = uint64_t;//this must have the size of a pointer

using Cache = cache_obj;
using Key_ptr = key_type;
using Value_ptr = val_type;
using Index = index_type;
using Hash_func = hash_func;

//different evictors want to use memory differently
//we define these different types of memory here and combine them all in a union so that each policy has access to its data
//Evictor goes on the cache itself, Evict_item goes on each individual entry
enum evictor_type {//evictor_types
	FIFO,
	LIFO,
	LRU,
	MRU,
	CLOCK,
	SLRU,
	RR,
};

struct Node {
	Index next;
	Index pre;
	bool rf_bit;
};
union Evict_item {
	Index rand_i;
	Node node;
};
struct DLL {
	Index head;
};
struct SLRU_data {
	DLL protect;
	DLL prohibate;
	Index pp_delta;
};

struct Rand_data {
	Index total_items;
};
union Evictor_data {
	DLL list;
	Rand_data rand_data;
	SLRU_data dlist;
};

struct Evictor {
	evictor_type policy;
	Evictor_data data;
	void* mem_arena;
};



struct Entry {
	Index cur_i;//index to the entry's position in the hash table
	Key_ptr key;
	Index key_size;
	byte* value;
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
	Index entry_capacity;
	Index entry_total;
	Index dead_total;//records deleted entries
	byte* mem_arena;//joint allocation of: {hash_table {Index* key_hashes, Bookmark* bookmarks}, Page* pages, void* evict_data}; these fields have functions for retrieving them
	Book entry_book;
	Hash_func hash;
	Evictor evictor;
};
#endif
