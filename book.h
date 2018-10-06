//By Monica Moniot and Alyssa Riceman
#pragma once
#include <cstring>
#include "cache.h"
#include "types.h"


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
