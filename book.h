//By Monica Moniot and Alyssa Riceman
#pragma once
#include <cstring>
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
