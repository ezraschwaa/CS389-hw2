//By Monica Moniot and Alyssa Riceman
#pragma once
#include "cache.h"
#include "book.h"
#include "types.h"


void create_evictor(Evictor* evictor, evictor_type policy, void* mem_arena);
constexpr Index get_mem_size_of_evictor(evictor_type policy, Index entry_capacity) {
   if(policy == FIFO) {
	   return 0;
   } else if(policy == LRU) {
	   return 0;
   } else {//RANDOM
	   return sizeof(Index)*entry_capacity;
   }
}

Index get_evict_item(Evictor* evictor, Book* book, void* mem_arena);//also removes item

void add_evict_item(Evictor* evictor, Index item_i, Evict_item* item, Book* book, void* mem_arena);

void remove_evict_item(Evictor* evictor, Index item_i, Evict_item* item, Book* book, void* mem_arena);

void touch_evict_item(Evictor* evictor, Index item_i, Evict_item* item, Book* book, void* mem_arena);
