#pragma once
//By Monica Moniot
#include "cache.h"

using Cache = cache_obj;
using Key_ptr = key_type;
using Value_ptr = val_type;
using Index = index_type;
using Hash_func = hash_func;

using Evict_data = index_type;
using DLL_value = Index;
using DLL_index = Index;
struct Node {
	DLL_index next;
	DLL_index pre;
	DLL_value val;
};
struct DLL {
	Node* nodes;
	DLL_index size;
	DLL_index end;
	DLL_index last;
	DLL_index first;
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
	Book book;
	Index* indices;
	Index total_i;
};
union Evictor_data {
	DLL list;
	Rand_data rand_data;
};

struct Evictor {
	evictor_type policy;
	Evictor_data data;
};

union Evict_data {

}


void create_evictor(Evictor* evictor, evictor_type policy, Index size);
void delete_evictor(Evictor* evictor);

Index evict_item(Evictor* evictor);

Evict_data add_item(Evictor* evictor, Index pid);

void remove_item(Evictor* evictor, Evict_data i);

void touch_item(Evictor* evictor, Evict_data i);
