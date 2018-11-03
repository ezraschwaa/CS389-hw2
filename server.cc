//By Monica Moniot and Alyssa Riceman
#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>

using byte = uint8_t;//this must have the size of a unit of memory (a byte)
using uint_ptr = uint64_t;//this must have the size of a pointer

using Cache = cache_obj;
using Key_ptr = key_type;
using Value_ptr = val_type;
using uint = index_type;
using Hash_func = hash_func;


constexpr uint DEFAULT_PORT = 33052;
constexpr uint DEFAULT_MAX_MEMORY = 1<<12;
constexpr uint MAX_MESSAGE_SIZE = 1<<10;
constexpr uint HIGH_BIT = 1<<(8*sizeof(uint) - 1);
constexpr uint MAX_MAX_MEMORY = ~HIGH_BIT;


// "GET /key/k\n\n"
int main(int argc, char** argv)
{
	uint port = DEFAULT_PORT;
	uint max_mem = DEFAULT_MAX_MEMORY;
	{
		char* port_arg = NULL;
		char* max_mem_arg = NULL;
		opterr = 0;
		auto c = getopt(argc, argv, "mt:");
		while(c != -1) {
			printf("%d\n", c);
			switch (c) {
			case 'm':
				max_mem_arg = optarg;
				break;
			case 't':
				port_arg = optarg;
				break;
			case '?':
				if(optopt == 'm') {
					fprintf(stderr, "Option -%c requires a maxmem value.\n", optopt);
				} else if(optopt == 't') {
					fprintf(stderr, "Option -%c requires a port number.\n", optopt);
				} else if(isprint(optopt)) {
					fprintf(stderr, "Unknown option `-%c'.\n", optopt);
				} else {
					fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
					return 1;
				}
			default:
				perror("bad args");
				return -1;
			}
			c = getopt(argc, argv, "abc:");
		}
		if(max_mem_arg) {
			auto user_max_mem = strtoul(max_mem_arg, NULL, 0);
			if(user_max_mem > 0 and user_max_mem <= MAX_MAX_MEMORY) {
				max_mem = user_max_mem;
				printf("1 %d\n", max_mem);
			} else {
				fprintf(stderr, "Option -m requires a valid memory size.\n");
			}
		}
		if(port_arg) {
			auto user_port = strtoul(port_arg, NULL, 0);
			if(user_port > 0 and user_port < 65535) {
				port = user_port;
				printf("2 %d\n", port);
			} else {
				fprintf(stderr, "Option -t requires a valid port no.\n");
			}
		}
	}

	uint server_fd;
	uint addrlen;
	sockaddr_in address;
	{
		addrlen = sizeof(address);

		// Creating socket file descriptor
		server_fd = socket(AF_INET, SOCK_STREAM, 0);
		if(server_fd == 0) {
			perror("socket failed");
			return -1;
		}

		// Forcefully attaching socket to the port
		uint opt = 1;
		bool failure = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
		if(failure) {
			perror("setsockopt");
			return -1;
		}
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(port);

		// Forcefully attaching socket to the port
		auto error_code = bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address));
		if(error_code < 0) {
			perror("bind failed");
			return -1;
		}
		error_code = listen(server_fd, 3);
		if(error_code < 0) {
			perror("listen");
			return -1;
		}
	}


	byte buffer[MAX_MESSAGE_SIZE];
	while(true) {
		uint new_socket = accept(server_fd, reinterpret_cast<sockaddr*>(&address), reinterpret_cast<socklen_t*>(&addrlen));
		if(new_socket < 0) {
			perror("accept");
			return -1;
		}

		const char* hello = "hello world";

		uint val_read = read(new_socket , buffer, MAX_MESSAGE_SIZE);
		printf("%s\n", buffer);

		send(new_socket, hello, strlen(hello), 0);
		printf("Hello message sent\n");
	}
	return 0;
}
