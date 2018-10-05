#pragma once
//By Monica Moniot
#include "cache.h"

using Cache = cache_obj;
using Key_ptr = key_type;
using Value_ptr = val_type;
using Index = index_type;
using Hash_func = hash_func;

struct DLL {
	Index last;
	Index first;
};

using Page_index = Index;
using Page_data = Index;
union Page {
	Page_index next;
	Page_data data;
};
struct Book {
	Page* pages;
	Page_index size;
	Page_index end;
	Page_index first;
};

// struct
struct Rand_data {
	Index* rand_items;
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

struct Node {
	Index next;
	Index pre;
};
union Evict_item {
	Index rand_i;
	Node node;
};

struct Evict_item_locator {
	void* abs_first;
	Index step_size;
};


void create_evictor(Evictor* evictor, evictor_type policy, Index size);
void grow_evictor(Evictor* evictor, Index size);
void delete_evictor(Evictor* evictor);

Index evict_item(Evictor* evictor, Evict_item_locator loc);

void add_item(Evictor* evictor, Index item_i, Evict_item* item, Evict_item_locator loc);

void remove_item(Evictor* evictor, Index item_i, Evict_item* item, Evict_item_locator loc);

void touch_item(Evictor* evictor, Index item_i, Evict_item* item, Evict_item_locator loc);

void update_item(Evictor* evictor, Index item_i, Evict_item* item, Index new_i, Evict_item_locator loc);
