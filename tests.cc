// Alyssa Riceman and Monica Moniot

#include <iostream>
#include <cmath>
#include <string.h>
#include "cache.h"

// Deleting the resulting pointer is warning-provoking; fix later
// Helper function for generating test strings of at least one "a"
char* make_str_of_defined_length(index_type length) {
    char* newstr = new char[length];
    for(index_type i = 0; i < length; i++) {
        newstr[i] = 'a';
    }
    return newstr;
}

const index_type CACHE_SIZE = 4096;
const key_type KEY = "404";
const val_type SMALLVAL = "a";
index_type SMALLVAL_SIZE = 1; //Val sizes can't be const because cache_get requires a non-const val pointer
const val_type LARGEVAL = make_str_of_defined_length(128);
index_type LARGEVAL_SIZE = 128;
const val_type TOOLARGEVAL = make_str_of_defined_length(8192);
index_type TOOLARGEVAL_SIZE = 8192;

// Tests for create_cache
//     Ensure the default hasher works and doesn't return the same number for every input
//     Ensure that the default evictor works (toss in more stuff than max_mem, then check if anything broke and make sure max_mem hasn't been exceeded)

// Helper function for test_hasher
index_type bad_hash_func_1(const char* message) {
    return 1;
}

// Helper function for test_create_cache
index_type bad_hash_func_2(const char* message) {
    if (sizeof(message) < 5) {
        return 1;
    } else {
        return 2;
    }
}

int test_create_cache() {
    const index_type SMALL_CACHE_SIZE = 64;
    const index_type LARGE_CACHE_SIZE = (pow(2, 24));
    cache_type cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
    cache_type cache3 = create_cache(CACHE_SIZE, FIFO, &bad_hash_func_1);
    cache_type cache5 = create_cache(SMALL_CACHE_SIZE, FIFO, NULL);
    cache_type cache6 = create_cache(LARGE_CACHE_SIZE, FIFO, NULL);
    return 0;
}

int test_hasher() {
    cache_type cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
    cache_type cache2 = create_cache(CACHE_SIZE, FIFO, &bad_hash_func_1);
    cache_type cache3 = create_cache(CACHE_SIZE, FIFO, &bad_hash_func_2);
    // test what happens if a hash function has collisions, and make sure that different inputs don't end up keyed together outside of the collision case; return -1 if anything fails
    return 0;
}

int test_evictor() {
    // Create caches with default evictor and a couple me-designed ones, then throw things into the caches to make sure each evictor behaves as expected; specific things to watch out for include max_mem being less than cache_space_used, and wrong items being evicted by the evictors whose workings I understand; return -1 if anything fails
    return 0;
}

// Tests for cache_set
//     Try setting various things, including overlarge ones, large numbers of small ones, weird types, et cetera
//     Figure out what's up with val_size being distinct from val (it seems intuitively weird to have that be user-input, that opens up lots of room for mistakes and accordingly for bugs)

// Tests for cache_get
//     Goes along with cache_set, make sure that my various gettings are good
//     Make sure that it returns null if I try to get something not in there

int test_cache_set_and_get() {
    const index_type WRONG_SIZE = 11;
    cache_type cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
    cache_set(cache1, KEY, SMALLVAL, SMALLVAL_SIZE);
    val_type retrieved_val = cache_get(cache1, KEY, &SMALLVAL_SIZE);
    if (retrieved_val != SMALLVAL) {
        std::cout << "Small value stored or retrieved incorrectly in set/get test.\n";
        return -1;
    }
    // Test largeval, make sure returned array is same as input array (I'm sure there's a C++ function to do that easily)
    // Test toolargeval, make sure the cache doesn't break from it
    // Test smallval with wrong_size to see what happens when input size is too small
    // Test largeval with wrong_size to see what happens when input size is too large
    // Test unusedkey to make sure that it's null
    return 0;
}

// Tests for cache_delete
//     Delete everything after getting it to make sure that works
//     Also delete some un-gotten stuff to make sure that it's not purely the "get then delete" order of operations that works

int test_cache_delete() {
    const key_type UNUSEDKEY = "bb";
    cache_type cache1 = create_cache(CACHE_SIZE, FIFO, NULL);

    cache_set(cache1, KEY, SMALLVAL, SMALLVAL_SIZE);
    val_type retrieved_val = cache_get(cache1, KEY, &SMALLVAL_SIZE);
    if (retrieved_val != SMALLVAL) {
        std::cout << "Small value stored or retrieved incorrectly in delete test.\n";
        return -1;
    }

    cache_delete(cache1, KEY);
    retrieved_val = cache_get(cache1, KEY, &SMALLVAL_SIZE);
    if (retrieved_val != NULL) {
        std::cout << "Small value was not deleted cleanly.\n";
        return -1;
    }

    return 0;
}

// Tests for cache_space_used
//     Seems pretty simple, just dump some stuff in the cache and make sure the numbers are reasonable (plus obligatory "what if inputs are too big" test), then delete them to make sure they go down accordingly

int test_cache_space_used() {
    cache_type cache1 = create_cache(CACHE_SIZE, FIFO, NULL);

    index_type space = cache_space_used(cache1);
    if (space != 0) {
        std::cout << "Cache was not initialized with 0 space used.\n";
        return -1;
    }

    cache_set(cache1, KEY, SMALLVAL, SMALLVAL_SIZE);
    space = cache_space_used(cache1);
    if (space != SMALLVAL_SIZE) {
        std::cout << "Cache failed to add used space from small input value.\n";
        return -1;
    }

    cache_set(cache1, (KEY + 1), LARGEVAL, LARGEVAL_SIZE);
    space = cache_space_used(cache1);
    if (space != (SMALLVAL_SIZE + LARGEVAL_SIZE)) {
        std::cout << "Cache failed to add used space from large input value.\n";
        return -1;
    }

    cache_delete(cache1, KEY);
    space = cache_space_used(cache1);
    if (space != LARGEVAL_SIZE) {
        std::cout << "Cache failed to remove space from large deleted value.\n";
        return -1;
    }

    cache_delete(cache1, (KEY + 1));
    space = cache_space_used(cache1);
    if (space != 0) {
        std::cout << "Cache failed to remove space from small deleted value.\n";
        return -1;
    }

    return 0;
}

int main() {
    int32_t error_pile = 0;
    error_pile += test_create_cache();
    error_pile += test_hasher();
    error_pile += test_evictor();
    error_pile += test_cache_set_and_get();
    error_pile += test_cache_delete();
    error_pile += test_cache_space_used();
    delete[] LARGEVAL;
    delete[] TOOLARGEVAL;
    if (error_pile < 0) {
        std::cout << "Errors remain.\n";
        return -1;
    } else {
        std::cout << "No errors detected!\n";
        return 0;
    }
}