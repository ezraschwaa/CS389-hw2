#pragma once
//By Monica Moniot
#include "cache.h"

using Cache = cache_obj;
using Key_ptr = key_type;
using Value_ptr = val_type;
using Index = index_type;
using Hash_func = hash_func;

using Evict_pid = index_type;
using DLL_value = Index;
using DLL_index = Index;
struct Node {
	DLL_index next;
	DLL_index pre;
	DLL_value val;
};
struct DLL {
	DLL_index first;
	DLL_index last;
	DLL_index end;
	DLL_index size;
	Node* nodes;
};

struct Evictor {
	evictor_type policy;
	DLL list;
};


void create_evictor(Evictor* evictor, evictor_type policy, Index size);
void delete_evictor(Evictor evictor);

Index evict_item(Evictor evictor);

Evict_pid add_item(Evictor evictor, Index pid);

void remove_item(Evictor evictor, Evict_pid i);

void touch_item(Evictor evictor, Evict_pid i);
