//By Monica Moniot and Alyssa Riceman
#pragma once
#include "cache.h"


using mem_unit = uint8_t;//this must have the size of a unit of memory (a byte)

using Cache = cache_obj;
using Key_ptr = key_type;
using Value_ptr = val_type;
using Index = index_type;
using Hash_func = hash_func;


//Book is a data structure for allocating memory of a fixed size in constant time
//Page is the unit of fixed size memory that book allocates
//Bookmark is a relative pointer to a page in book
//Page_data defines the type and size of memory that book allocates
struct Node {
	Index next;
	Index pre;
};
union Evict_item {
	Index rand_i;
	Node node;
};
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


struct DLL {
	Index last;
	Index first;
};

struct Rand_data {
	Index total_items;
};
union Evictor_data {
	DLL list;
	Rand_data rand_data;
};

struct Evictor {
	evictor_type policy;
	Evictor_data data;
};


struct cache_obj {//Definition of Cache
	Index mem_capacity;
	Index mem_total;
	Index entry_capacity;
	Index entry_total;
	Index dead_total;//records deleted entries
	mem_unit* mem_arena;//joint allocation of: {hash_table {Bookmark* bookmarks, Index* key_hashes, Key_ptr keys}, Page* pages, void* evict_data}; these fields have functions for retrieving them
	Book entry_book;
	Hash_func hash;
	Evictor evictor;
};
