
#include <stdlib.h>
#include <stdio.h>







using Page_index = int;
using Page_data = int;
union Page {
	Page_index next;
	Page_data data;
};
struct Book {
	Page* pages;
	Page_index size;
};

struct Rand_data {
	Book book;
};

void print_address(void* address) {
	printf("%#lx\n", (long unsigned int)address);
}
void print_s(Page_index size) {
	printf("%d\n", size);
}

int main() {
	Rand_data* data = new Rand_data;

	//all equal to "data"
	print_address(data);
	print_address(&data->book);
	print_address(&data->book.pages);
	print_address(&(data->book.pages));
	print_address(&(data->book).pages);
	print_address(&(&data->book)->pages);

	//all unequal
	print_address(data->book.pages);
	Book book = data->book;
	print_address(&book);
	Page* pages = data->book.pages;
	print_address(&pages);

	print_s(data->book.size);//0
	data->book.size = 2;
	print_s(data->book.size);//2
	print_s((&data->book)->size);//2
	book = data->book;
	book.size = 3;
	print_s(data->book.size);//2
	print_s(book.size);//3

	return 0;
}
