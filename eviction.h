//By Monica Moniot
#include "cache.h"

using mem_unit = uint8_t;

using Cache = cache_obj;
using Key_ptr = key_type;
using Value_ptr = val_type;
using Index = index_type;
using Hash_func = hash_func;

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


void create_evictor(Evictor* evictor, evictor_type policy, void* mem_arena);
constexpr Index get_mem_size_of_evictor(evictor_type policy, Index entry_capacity);

Index get_evict_item(Evictor* evictor, Evict_item_locator loc, void* mem_arena);//also removes item

void add_evict_item(Evictor* evictor, Index item_i, Evict_item* item, Evict_item_locator loc, void* mem_arena);

void remove_evict_item(Evictor* evictor, Index item_i, Evict_item* item, Evict_item_locator loc, void* mem_arena);

void touch_evict_item(Evictor* evictor, Index item_i, Evict_item* item, Evict_item_locator loc, void* mem_arena);
