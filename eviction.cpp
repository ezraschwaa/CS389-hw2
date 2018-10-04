#pragma once
//By Monica Moniot
#include <stdlib.h>
#include <cstring>
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
void grow_DLL(DLL* list) {
	// DLL list;
	Node* new_nodes = new Node[new_size];
	memcpy(new_nodes, list->nodes, pre_size);
	delete list->nodes;
	list->nodes = new_nodes;
}
void reorder(DLL* list, DLL_index i0, DLL_index i1) {//does not set start
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
void remove(DLL* list, DLL_index i) {
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
DLL_index append(DLL* list, DLL_value v) {//may grow
	auto last = list->last;
	auto nodes = list->nodes;
	DLL_index i;
	if(last == INVALID_NODE) {
		i = 0;
		Node* node = &nodes[i];
		node->val = v;
		node->next = INVALID_NODE;
		node->pre = INVALID_NODE;
		list->first = i;
		list->end = 1;
	} else {
		Node* last_node = &nodes[last];
		i = last_node->next;
		Node* node;
		if(i == INVALID_NODE) {
			i = list->end;
			list->end += 1;
			node = &nodes[i];
			node->next = INVALID_NODE;
		} else {
			node = &nodes[i];
		}
		node->pre = last;
	}
	list->last = i;
	return i;
}
DLL_index prepend(DLL* list, DLL_value v) {//may grow
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


void create_evictor(Evictor* evictor, evictor_type policy, Index init_capacity) {
	evictor->policy = policy;
	if(policy == FIFO) {
		auto list = evictor->list;
		create_DLL(list, init_capacity)
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
		// remove(&evictor.list, i);
	} else if(policy == LRU) {
		i = evictor.list.end;
		// remove(&evictor.list, i);
	}
	return i;
}

Evict_pid add_item(Evictor evictor, Index pid) {//item was created
	auto policy = evictor.policy;
	if(policy == FIFO) {

	} else if(policy == LRU) {

	}
}

void remove_item(Evictor evictor, Evict_pid i) {//item was removed
	auto policy = evictor.policy;
	if(policy == FIFO) {

	} else if(policy == LRU) {

	}
}

void touch_item(Evictor evictor, Evict_pid i) {//item was touched
	auto policy = evictor.policy;
	if(policy == FIFO) {

	} else if(policy == LRU) {

	}
}
