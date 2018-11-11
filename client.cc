// Alyssa Riceman and Monica Moniot

#include <iostream>
#include <cassert>
#include <cstdio>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include "cache.h"

typedef int socketType;
struct cache_obj {
    uint16_t portNum;
    const char* ipAddress;
};

uint16_t PORTNUM = 33052;
const char* LOCALHOST = "127.0.0.1";

char* makeHttpRequest(const char* method, const char* uriBase, const char* key, const char* value) {
    uint32_t beforeKeyLength = strlen(method) + strlen(uriBase) + 2; //Add 2 for the " /" between the method and the URI
    uint32_t keyLength = 0;
    uint32_t valLength = 0;
    if (key != NULL) {
        keyLength = strlen(key) + 1; //Add 1 for the / before the key
    }
    if (value != NULL) {
        valLength = strlen(value) + 1; //Add 1 for the / before the value
    }
    uint32_t overallLength = beforeKeyLength + keyLength + valLength + 3; //Add 3 for two newlines and one null terminator

    char* request = new char[overallLength];
    assert(request != NULL && "Failed to allocate memory for HTTP request.");
    const char* finalNewlines = "\n\n";
    uint32_t requestFillSuccess = 0;
    if ((key == NULL) && (value == NULL)) {
        requestFillSuccess = sprintf(request, "%s /%s%s", method, uriBase, finalNewlines);
    } else if ((key != NULL) && (value == NULL)) {
        requestFillSuccess = sprintf(request, "%s /%s/%s%s", method, uriBase, key, finalNewlines);
    } else if ((key == NULL) && (value != NULL)) {
        requestFillSuccess = sprintf(request, "%s /%s/%s%s", method, uriBase, value, finalNewlines);
    } else { //(key != NULL) && (value != NULL)
        requestFillSuccess = sprintf(request, "%s /%s/%s/%s%s", method, uriBase, key, value, finalNewlines);
    }
    assert(requestFillSuccess > 0 && "Failed to stitch together HTTP request string.");

    return request;
}

int charsBeforeNewline(char* cStr) {
    int stringLength = strlen(cStr);
    for (int i = 0; i < stringLength; i++) {
        if (cStr[i] == '\n') {
            return i; 
        }
    }
    return stringLength;
}

char* getStatusCode(char* sourceBuffer, int sourceBufferLength) {

    int statusCodeStart = 0;
    while (statusCodeStart < sourceBufferLength) {
        if (sourceBuffer[statusCodeStart] == ' '){
            statusCodeStart++;
            break;
        }
        statusCodeStart++;
    }
    int statusCodeEnd = charsBeforeNewline(sourceBuffer);
    assert(statusCodeStart < statusCodeEnd && "Status code does not start where expected.");

    char* statusCodeStr = new char[statusCodeEnd - statusCodeStart];
    for (int i = 0; i < (statusCodeEnd - statusCodeStart); i++) {
        statusCodeStr[i] = sourceBuffer[i + statusCodeStart];
    }
    return statusCodeStr;
}

int handleStatusCode(char* sourceBuffer, int sourceBufferLength) {
    char* statusCode;

    int success = 0;
    bool codeNeeded = true;
    while (codeNeeded) { // Loop waiting for status code
        statusCode = getStatusCode(sourceBuffer, sourceBufferLength);
        switch(statusCode[0]) {
            case '1':
                delete[] statusCode;
                continue;
            case '2':
                codeNeeded = false;
            case '3':
                delete[] statusCode;
                continue;
            case '4':
                success = -1;
                codeNeeded = false;
            case '5':
                success = -1;
                codeNeeded = false;
        }
    }

    delete[] statusCode;
    return success;
}

// char* readMessage(char* sourceBuffer, int sourceBufferLength) {
//     int messageStart = 0;
//     bool hitOneNewline = false;
//     while (messageStart < sourceBufferLength) {
//         if (sourceBuffer[messageStart] == '\n' &&){
//             statusCodeStart++;
//             break;
//         }
//         statusCodeStart++;
//     }
// }

socketType start_socket(int commType, uint16_t portNum, const char* ipAddress) {
    socketType newSocket = socket(AF_INET, commType, 0);
    assert(newSocket >= 0 && "Socket creation failed.");

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNum);
    int addressSuccess = inet_pton(AF_INET, ipAddress, &serverAddress.sin_addr.s_addr);
    assert(addressSuccess == 1 && "Socket address set failed.");

    const sockaddr* addressPointer = reinterpret_cast<const sockaddr *>(&serverAddress);
    int connectionSuccess = connect(newSocket, addressPointer, sizeof(serverAddress));
    assert(connectionSuccess == 0 && "Socket connection failed.");

    return newSocket;
}

cache_type create_cache(index_type maxmem, hash_func hasher) {
    // socketType tcpSocket = start_socket(SOCK_STREAM, PORTNUM, LOCALHOST);
    // socketType udpSocket = start_socket(SOCK_DGRAM, PORTNUM, LOCALHOST);

    // Ignore maxmem and hasher; the server API doesn't allow them to work any more

    cache_obj* cacheStruct = new cache_obj;
    // cacheStruct->tcpSocket = tcpSocket;
    // cacheStruct->udpSocket = udpSocket;
    cacheStruct->portNum = PORTNUM;
    cacheStruct->ipAddress = LOCALHOST;

    return cacheStruct;
}

int cache_set(cache_type cache, key_type key, val_type val, index_type val_size) {
    socketType setSocket = start_socket(SOCK_STREAM, cache->portNum, cache->ipAddress);

    char *setRequest = makeHttpRequest("PUT", "key", key, static_cast<const char *>(val));
    std::cout << setRequest;
    send(setSocket, setRequest, strlen(setRequest), 0);

    char* serverReadBuffer = new char[2048];
    int serverReadLength = read(setSocket, serverReadBuffer, 2048);
    assert(serverReadLength >= 0 && "Failed to read from server.");
    char* statusCode;

    int setSuccess = handleStatusCode(serverReadBuffer, 2048);

    delete[] setRequest;
    delete[] serverReadBuffer;
    return setSuccess;
}

val_type cache_get(cache_type cache, key_type key, index_type *val_size) {
    socketType getSocket = start_socket(SOCK_STREAM, cache->portNum, cache->ipAddress);

    char *getRequest = makeHttpRequest("GET", "key", key, NULL);
    std::cout << getRequest;
    int sendSuccess = send(getSocket, getRequest, strlen(getRequest), 0);
    assert(sendSuccess >= 0 && "Failed to send space used request.");

    char* serverReadBuffer = new char[2048];
    int serverReadLength = read(getSocket, serverReadBuffer, 2048);
    assert(serverReadLength >= 0 && "Failed to read from server.");
    char* statusCode = getStatusCode(serverReadBuffer, 2048);

    int getSuccess = handleStatusCode(serverReadBuffer, 2048);

    //If applicable, read and return reply

    delete[] getRequest;
    delete[] statusCode;
    return NULL; //Placeholder
}

int cache_delete(cache_type cache, key_type key) {
    socketType deleteSocket = start_socket(SOCK_STREAM, cache->portNum, cache->ipAddress);

    char *deleteRequest = makeHttpRequest("DELETE", "key", key, NULL);
    std::cout << deleteRequest;
    int sendSuccess = send(deleteSocket, deleteRequest, strlen(deleteRequest), 0);
    assert(sendSuccess >= 0 && "Failed to send delete request.");

    char* serverReadBuffer = new char[2048];
    int serverReadLength = read(deleteSocket, serverReadBuffer, 2048);
    assert(serverReadLength >= 0 && "Failed to read from server.");
    char* statusCode = getStatusCode(serverReadBuffer, 2048);

    int deleteSuccess = handleStatusCode(serverReadBuffer, 2048);

    delete[] deleteRequest;
    delete[] statusCode;
    return deleteSuccess;
}

index_type cache_space_used(cache_type cache){
    socketType spaceUsedSocket = start_socket(SOCK_STREAM, cache->portNum, cache->ipAddress);

    char* spaceUsedRequest = makeHttpRequest("GET", "memsize", NULL, NULL);
    std::cout << spaceUsedRequest;
    int sendSuccess = send(spaceUsedSocket, spaceUsedRequest, strlen(spaceUsedRequest), 0);
    std::cout << sendSuccess << "\n";
    assert(sendSuccess >= 0 && "Failed to send space used request.");

    // Receive reply from server

    delete[] spaceUsedRequest;
    return 0; //Placeholder
}

void destroy_cache(cache_type cache) {
    socketType destroySocket = start_socket(SOCK_STREAM, cache->portNum, cache->ipAddress);

    char* destroyRequest = makeHttpRequest("POST", "shutdown", NULL, NULL);
    std::cout << destroyRequest;
    int sendSuccess = send(destroySocket, destroyRequest, strlen(destroyRequest), 0);
    assert(sendSuccess >= 0 && "Failed to send destroy request.");

    delete[] destroyRequest;
}

int main() {
    cache_type testCache = create_cache(4096, NULL);
    key_type shortCstring = "bbq";
    char* valueText = "BarbecueLettuce";
    void* placeholderValue = reinterpret_cast<void*>(valueText);
    index_type placeholderValSize = 15;
    cache_set(testCache, shortCstring, placeholderValue, placeholderValSize);
    cache_get(testCache, shortCstring, &placeholderValSize); //Weird segfault here; debug when possible
    cache_space_used(testCache);
    cache_delete(testCache, shortCstring);
    destroy_cache(testCache);
    return 1; // Main is for testing only, will vanish from final program
}