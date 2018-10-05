#pragma once
//By Monica Moniot
#include <stdlib.h>
#include <cstring>
#include <cassert>
#include "eviction.h"

DLL_index INVALID_NODE = -1;
inline void create_DLL(DLL* list, DLL_index size) {
	// DLL list;
	list->first = INVALID_NODE;
	list->last = INVALID_NODE;
	list->end = 0;
	list->size = size;
	list->nodes = new Node[size];
}
inline void delete_DLL(DLL* list) {
	delete list->nodes;
	list->nodes = NULL;
	list->size = 0;
}
void grow_DLL(DLL* list) {
	// DLL list;
	auto pre_size = list->size;
	auto new_size = 2*pre_size;
	Node* new_nodes = new Node[new_size];
	memcpy(new_nodes, list->nodes, pre_size);
	delete list->nodes;
	list->nodes = new_nodes;
}
void reorder(DLL* list, DLL_index i0, DLL_index i1) {//does not set start
	assert(i0 < list->end and i1 < list->end);
	auto nodes = list->nodes;
	auto node0 = &nodes[i0];
	auto node1 = &nodes[i1];
	auto next_i0 = node0->next;
	auto pre_i0 = node0->pre;
	auto next_i1 = node1->next;
	auto pre_i1 = node1->pre;

	node1->next = next_i0;
	node1->pre = pre_i0;
	if(pre_i0 != INVALID_NODE) {
		(&nodes[pre_i0])->next = i1;
	}
	if(next_i0 != INVALID_NODE) {
		(&nodes[next_i0])->pre = i1;
	}

	node0->next = next_i1;
	node0->pre = pre_i1;
	if(pre_i1 != INVALID_NODE) {
		(&nodes[pre_i1])->next = i0;
	}
	if(next_i1 != INVALID_NODE) {
		(&nodes[next_i1])->pre = i0;
	}

	if(i0 == list->first) {
		list->first = i1;
	} else if(i1 == list->first) {
		list->first = i0;
	}
	if(i0 == list->last) {
		list->last = i1;
	} else if(i1 == list->last) {
		list->last = i0;
	}
}
void remove(DLL* list, DLL_index i) {
	assert(i < list->end);
	auto nodes = list->nodes;
	auto node = &nodes[i];
	auto next_i = node->next;
	auto pre_i = node->pre;

	if(pre_i != INVALID_NODE) {
		(&nodes[pre_i])->next = next_i;
	}
	if(next_i != INVALID_NODE) {
		(&nodes[next_i])->pre = pre_i;
	}

	if(list->first == i) {
		list->first = next_i;
	}
	if(list->last == i) {
		list->last = pre_i;
	}
	Node* last_node = &nodes[list->last];
	node->next = last_node->next;
	last_node->next = i;
}
void add_to_empty(DLL* list, DLL_value v) {
	auto nodes = list->nodes;
	Node* node = &nodes[0];
	node->val = v;
	node->next = INVALID_NODE;
	node->pre = INVALID_NODE;
	list->last = 0;
	list->end = 1;
}
DLL_index append(DLL* list, DLL_value v) {//may grow
	auto last = list->last;
	auto nodes = list->nodes;
	DLL_index i;
	if(last == INVALID_NODE) {
		i = 0;
		add_to_empty(list, v);
	} else {
		Node* last_node = &nodes[last];
		i = last_node->next;
		Node* node;
		if(i == INVALID_NODE) {
			i = list->end;
			list->end += 1;
			if(i == list->size) {
				grow_DLL(list);
				nodes = list->nodes;
				node = &nodes[i];
			}
			node = &nodes[i];
			node->next = INVALID_NODE;
		} else {
			node = &nodes[i];
		}
		node->val = v;
		node->pre = last;
		last_node->next = i;
	}
	list->last = i;
	return i;
}
DLL_index prepend(DLL* list, DLL_value v) {//may grow
	auto last = list->last;
	auto nodes = list->nodes;
	DLL_index i;
	if(last == INVALID_NODE) {
		i = 0;
		add_to_empty(list, v);
	} else {
		Node* last_node = &nodes[last];
		i = last_node->next;
		Node* node;
		if(i == INVALID_NODE) {
			i = list->end;
			list->end += 1;
			if(i == list->size) {
				grow_DLL(list);
				nodes = list->nodes;
				node = &nodes[i];
			}
			node = &nodes[i];
		} else {
			node = &nodes[i];
			last_node->next = node->next;
		}
		auto first = list->first;
		node->val = v;
		node->pre = INVALID_NODE;
		node->next = first;
		(&nodes[first])->pre = i;
	}
	list->first = i;
	return i;
}


Page_index INVALID_PAGE = -1;
inline void create_book(Book* book, Page_index size) {
	book->first = INVALID_PAGE;
	book->end = 0;
	book->size = size;
	book->pages = new Page[size];
}
inline void delete_book(Book* book) {
	delete book->pages;
	book->pages = NULL;
	book->size = 0;
}
inline void grow_book(Book* book) {
	auto pre_size = book->size;
	auto new_size = 2*pre_size;
	Page* new_pages = new Page[new_size];
	memcpy(new_pages, book->pages, pre_size);
	book->size = new_size;
	book->pages = new_pages;
}
inline Page_index alloc_book_page(Book* book) {
	auto pages = book->pages;
	auto i = book->first;
	if(i == INVALID_PAGE) {
		i = book->end;
		book->end += 1;
		if(i >= book->size) {
			grow_book(book);
		}
	} else {
		book->first = pages[i].next;
	}
	return i;
}
inline void free_book_page(Book* book, Page_index i) {
	auto pages = book->pages;
	(&pages[i])->next = book->first;
	book->first = i;
}
inline Page_data* read_book(Book* book, Page_index i) {
	return &book->pages[i].data;
}


void create_evictor(Evictor* evictor, evictor_type policy, Index init_capacity) {
	evictor->policy = policy;
	if(policy == FIFO) {
		auto list = &evictor->data.list;
		create_DLL(list, init_capacity);
	} else if(policy == LRU) {
		auto list = &evictor->data.list;
		create_DLL(list, init_capacity);
	} else {//RANDOM
		auto data = &evictor->data.rand_data;
		create_book(&data->book, init_capacity);
		data->indices = new Index[init_capacity];
		data->total_i = 0;
	}
}
void delete_evictor(Evictor* evictor) {
	auto policy = evictor->policy;
	if(policy == FIFO) {
		delete_DLL(&evictor->data.list);
	} else if(policy == LRU) {
		delete_DLL(&evictor->data.list);
	} else {//RANDOM
		auto data = &evictor->data.rand_data;
		delete_book(&data->book);
		delete data->indices;
		data->indices = NULL;
	}
}

Index evict_item(Evictor* evictor) {//return item to evict
	auto policy = evictor->policy;
	Index i = 0;
	if(policy == FIFO) {
		auto list = &evictor->data.list;
		i = list->first;
		remove(list, i);
	} else if(policy == LRU) {
		auto list = &evictor->data.list;
		i = list->first;
		remove(list, i);
	} else {//RANDOM
		auto data = &evictor->data.rand_data;
		Evict_pid pid = rand()%data->total_i;
		i = data->indices[pid];
		// data->indices[pid] = da;
	}
	return i;
}

Evict_pid add_item(Evictor* evictor, Index i) {//item was created
	auto policy = evictor->policy;
	Evict_pid pid;
	if(policy == FIFO) {
		pid = append(&evictor->data.list, i);
	} else if(policy == LRU) {
		pid = append(&evictor->data.list, i);
	} else {//RANDOM
		auto data = &evictor->data.rand_data;
		if(data->book.end >= data->book.size) {
			auto pre_size = data->book.size;
			auto new_size = 2*pre_size;
			auto new_indices = new Index[new_size];
			memcpy(new_indices, data->indices, pre_size);
			delete data->indices;
			data->indices = new_indices;
		}
		pid = alloc_book_page(&data->book);
		Index* item = read_book(&data->book, pid);
		*item = data->total_i;
		data->total_i += 1;
		data->indices[*item] = i;
	}
	return pid;
}

void remove_item(Evictor* evictor, Evict_pid pid) {//item was removed
	auto policy = evictor->policy;
	if(policy == FIFO) {
		auto list = &evictor->data.list;
		remove(list, pid);
	} else if(policy == LRU) {
		auto list = &evictor->data.list;
		remove(list, pid);
	}
}

void touch_item(Evictor* evictor, Evict_pid pid) {//item was touched
	auto policy = evictor->policy;
	if(policy == FIFO) {

	} else if(policy == LRU) {

		auto list = &evictor->data.list;
		reorder(list, list->last, pid);
	}
}
