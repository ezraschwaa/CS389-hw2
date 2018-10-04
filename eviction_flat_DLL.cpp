#pragma once
//By Monica Moniot
#include <stdlib.h>
#include <cstring>
#include "eviction.h"

using Cache = cache_obj;
using Key_ptr = key_type;
using Value_ptr = val_type;
using Index = index_type;
using Hash_func = hash_func;

Index INVALID_NODE = -1;
struct Node {
	// Index pid;
	Index next;
	Index pre;
};
struct DLL {//has no size
	Index start;
	Index end;
	Node* nodes;
};

inline void create_DLL(DLL* list, Index size) {
	// DLL list;
	list->start = INVALID_NODE;
	list->end = INVALID_NODE;
	list->nodes = new Node[size];
}
inline void grow_DLL(DLL* list, Index pre_size, Index new_size) {
	// DLL list;
	Node* new_nodes = new Node[new_size];
	memcpy(new_nodes, list->nodes, pre_size);
	delete list->nodes;
	list->nodes = new_nodes;
}
inline void reorder(DLL* list, Index i0, Index i1) {//does not set start
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
}
inline void remove(DLL* list, Index i) {
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

	if(list->start == i) {
		list->start = next_i;
	}
	if(list->end == i) {
		list->end = pre_i;
	}
}
void prepend(DLL* list, Index i) {
	auto node = &list->nodes[i];
	node->next = INVALID_NODE;
	node->pre = INVALID_NODE;
	if(list->start == INVALID_NODE) {
		list->end = i;
	} else {
		reorder(list, i, list->start);
	}
	list->start = i;
}
void append(DLL* list, Index i) {
	auto node = &list->nodes[i];
	node->next = INVALID_NODE;
	node->pre = INVALID_NODE;
	if(list->end == INVALID_NODE) {
		list->start = i;
	} else {
		reorder(list, i, list->end);
	}
	list->end = i;
}


union Evictor {
	evictor_type policy;
	DLL list;
};

void create_evictor(Evictor* evictor, evictor_type policy, Index pid_capacity) {
	evictor->policy = policy;
	if(policy == FIFO) {

	} else if(policy == LRU) {

	}
}
void delete_evictor(Evictor evictor) {
	auto policy = evictor.policy;
	if(policy == FIFO) {

	} else if(policy == LRU) {

	}
}

Index evict_item(Evictor evictor) {//return item to evict
	auto policy = evictor.policy;
	Index i = 0;
	if(policy == FIFO) {
		i = evictor.list.end;
		remove(&evictor.list, i);
	} else if(policy == LRU) {
		i = evictor.list.end;
		remove(&evictor.list, i);
	}
	return i;
}

void add_item(Evictor evictor, index_type pid) {//item was created
	auto policy = evictor.policy;
	if(policy == FIFO) {

	} else if(policy == LRU) {

	}
}

void update_item(Evictor evictor, index_type pid) {//item was touched
	auto policy = evictor.policy;
	if(policy == FIFO) {

	} else if(policy == LRU) {

	}
}
