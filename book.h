//By Monica Moniot and Alyssa Riceman
#pragma once
#include "cache.h"
#include "types.h"


//Book is a data structure for allocating memory of a fixed size in constant time
//Page is the unit of fixed size memory that book allocates
//Bookmark is a relative pointer to a page in book
//Page_data defines the type and size of memory that book allocates

constexpr Bookmark INVALID_PAGE = -1;

inline void create_book(Book* book, Page* pages, Bookmark size) {
	book->first_unused = INVALID_PAGE;
	book->end = 0;
	book->pages = pages;
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
constexpr inline Page_data* read_book(const Book* book, Bookmark bookmark) {
	return &book->pages[bookmark].data;
}
