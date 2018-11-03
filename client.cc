// Alyssa Riceman and Monica Moniot

#include <iostream>
#include <cassert>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "cache.h"

uint16_t PORTNO = 8765;
const char* ipAddress = "127.0.0.1";

int main() {
    int clientSocket;
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    assert(clientSocket >= 0 && "Socket creation failed.");

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORTNO);
    inet_pton(AF_INET, ipAddress, &serverAddress.sin_addr.s_addr);

    const sockaddr *addressPointer = reinterpret_cast<const sockaddr*>(&serverAddress);
    int connectionSuccess = connect(clientSocket, addressPointer, sizeof(serverAddress));
    assert(connectionSuccess == 0 && "Connection failed.");
}