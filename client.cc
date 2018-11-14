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
const char* IPADDRESS = "127.0.0.1";

char* makeHttpRequest(const char* method, const char* uriBase, const char* key, val_type value, uint32_t valLength) {
    uint32_t beforeKeyLength = strlen(method) + strlen(uriBase) + 2; //Add 2 for the " /" between the method and the URI
    uint32_t keyLength = 0;
    const char* valAsStr;
    if (key != NULL) {
        keyLength = strlen(key) + 1; //Add 1 for the / before the key
    }
    if (value != NULL) {
        valAsStr = static_cast<const char*>(value);
        valLength += 1; //Add 1 for the / before the value
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
        requestFillSuccess = sprintf(request, "%s /%s/%.*s%s", method, uriBase, valLength, valAsStr, finalNewlines);
    } else { //(key != NULL) && (value != NULL)
        requestFillSuccess = sprintf(request, "%s /%s/%s/%.*s%s", method, uriBase, key, valLength, valAsStr, finalNewlines);
    }
    assert(requestFillSuccess > 0 && "Failed to stitch together HTTP request string.");

    return request;
}

int charsBeforeNewline(char* cStr, int cStrLength) {
    for (int i = 0; i < cStrLength; i++) {
        if (cStr[i] == '\n') {
            return i; 
        }
    }
    return cStrLength;
}

uint32_t getStatusCodeIndex(char* sourceBuffer, int sourceBufferLength) {

    int statusCodeStart = 0;
    while (statusCodeStart < sourceBufferLength) {
        if (sourceBuffer[statusCodeStart] == ' '){
            statusCodeStart++;
            break;
        }
        statusCodeStart++;
    }
    return statusCodeStart;
}

int handleStatusCode(char* sourceBuffer, int sourceBufferLength) {
    char* statusCode;

    int success = -1;
    uint32_t statusCodeIndex = getStatusCodeIndex(sourceBuffer, sourceBufferLength);
    if (sourceBuffer[statusCodeIndex] == '2') {
        success = 0;
    }

    return success;
}

char* readMessage(char* sourceBuffer, int sourceBufferLength) {
    int messageStart = 0;
    bool hitOneNewline = false;
    while (messageStart < sourceBufferLength) { // Find index of message body start
        if (sourceBuffer[messageStart] == '\n'){
            if (hitOneNewline) {
                messageStart++;
                break;
            } else {
                hitOneNewline = true;
            }
        }
        messageStart++;
    }

    char* messageStr = new char[sourceBufferLength - messageStart];
    for (int i = 0; i < (sourceBufferLength - messageStart); i++) {
        messageStr[i] = sourceBuffer[i + messageStart];
    }
    return messageStr;
}

char* parseMessageForGet(char* message) {
    uint32_t keySize = *reinterpret_cast<uint*>(message);
    uint32_t valIndex = keySize + 4; // +4 for the initial 4 bytes representing key size
    uint32_t valSize = *reinterpret_cast<uint*>(&message[valIndex]);
    valIndex += 4; //+4 for reading val size

    char* parsedMessage = new char[valSize];
    for (int i = 0; i < valSize; i++) {
        parsedMessage[i] = message[i + valIndex];
    }
    return parsedMessage;
}

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
    // Ignore maxmem and hasher; the server API doesn't allow them to work any more

    cache_obj* cacheStruct = new cache_obj;
    cacheStruct->portNum = PORTNUM;
    cacheStruct->ipAddress = IPADDRESS;

    return cacheStruct;
}

int cache_set(cache_type cache, key_type key, val_type val, index_type val_size) {
    socketType setSocket = start_socket(SOCK_STREAM, cache->portNum, cache->ipAddress);

    char *setRequest = makeHttpRequest("PUT", "key", key, val, val_size);
    std::cout << setRequest;
    send(setSocket, setRequest, strlen(setRequest), 0);

    char* serverReadBuffer = new char[2048];
    int serverReadLength = read(setSocket, serverReadBuffer, 2048);
    assert(serverReadLength >= 0 && "Failed to read from server.");

    int setSuccess = handleStatusCode(serverReadBuffer, serverReadLength);

    delete[] setRequest;
    delete[] serverReadBuffer;
    return setSuccess;
}

val_type cache_get(cache_type cache, key_type key, index_type *val_size) {
    socketType getSocket = start_socket(SOCK_STREAM, cache->portNum, cache->ipAddress);

    char *getRequest = makeHttpRequest("GET", "key", key, NULL, 0);
    std::cout << getRequest;
    int sendSuccess = send(getSocket, getRequest, strlen(getRequest), 0);
    assert(sendSuccess >= 0 && "Failed to send space used request.");

    char* serverReadBuffer = new char[2048 + *val_size];
    int serverReadLength = read(getSocket, serverReadBuffer, 2048 + *val_size);
    assert(serverReadLength >= 0 && "Failed to read from server.");

    int getSuccess = handleStatusCode(serverReadBuffer, serverReadLength);

    val_type retrievedVal = NULL;
    if (getSuccess == 0) {
        char* message = readMessage(serverReadBuffer, serverReadLength);
        char* parsedMessage = parseMessageForGet(message);
        retrievedVal = static_cast<val_type>(parsedMessage);
        delete[] message;
    }

    delete[] getRequest;
    delete[] serverReadBuffer;
    return retrievedVal; //Placeholder
}

int cache_delete(cache_type cache, key_type key) {
    socketType deleteSocket = start_socket(SOCK_STREAM, cache->portNum, cache->ipAddress);

    char *deleteRequest = makeHttpRequest("DELETE", "key", key, NULL, 0);
    std::cout << deleteRequest;
    int sendSuccess = send(deleteSocket, deleteRequest, strlen(deleteRequest), 0);
    assert(sendSuccess >= 0 && "Failed to send delete request.");

    char* serverReadBuffer = new char[2048];
    int serverReadLength = read(deleteSocket, serverReadBuffer, 2048);
    assert(serverReadLength >= 0 && "Failed to read from server.");

    int deleteSuccess = handleStatusCode(serverReadBuffer, serverReadLength);

    delete[] deleteRequest;
    delete[] serverReadBuffer;
    return deleteSuccess;
}

index_type cache_space_used(cache_type cache){
    socketType spaceUsedSocket = start_socket(SOCK_STREAM, cache->portNum, cache->ipAddress);

    char* spaceUsedRequest = makeHttpRequest("GET", "memsize", NULL, NULL, 0);
    std::cout << spaceUsedRequest;
    int sendSuccess = send(spaceUsedSocket, spaceUsedRequest, strlen(spaceUsedRequest), 0);
    assert(sendSuccess >= 0 && "Failed to send space used request.");

    char *serverReadBuffer = new char[2048];
    int serverReadLength = read(spaceUsedSocket, serverReadBuffer, 2048);
    assert(serverReadLength >= 0 && "Failed to read from server.");

    int spaceUsedSuccess = handleStatusCode(serverReadBuffer, serverReadLength);

    uint spaceNumber = 0;
    if (spaceUsedSuccess == 0) {
        char* spaceNumberStr = readMessage(serverReadBuffer, serverReadLength);
        spaceNumber = *reinterpret_cast<uint*>(spaceNumberStr);
        delete[] spaceNumberStr;
    }

    delete[] spaceUsedRequest;
    delete[] serverReadBuffer;
    return spaceNumber;
}

void destroy_cache(cache_type cache) {
    socketType destroySocket = start_socket(SOCK_STREAM, cache->portNum, cache->ipAddress);

    char* destroyRequest = makeHttpRequest("POST", "shutdown", NULL, NULL, 0);
    std::cout << destroyRequest;
    int sendSuccess = send(destroySocket, destroyRequest, strlen(destroyRequest), 0);
    assert(sendSuccess >= 0 && "Failed to send destroy request.");

    delete[] destroyRequest;
}

int main() {
    cache_type testCache = create_cache(4096, NULL);
    key_type shortCstring = "bbq";
    const char* valueText = "BarbecueLettuce";
    const void* placeholderValue = static_cast<const void*>(valueText);
    index_type placeholderValSize = 16;
    cache_set(testCache, shortCstring, placeholderValue, placeholderValSize);
    val_type getTest = cache_get(testCache, shortCstring, &placeholderValSize);
    cache_space_used(testCache);
    cache_delete(testCache, shortCstring);
    destroy_cache(testCache);
    return 1; // Main is for testing only, will vanish from final program
}