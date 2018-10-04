// Alyssa Riceman and Monica Moniot

#include <iostream>
#include "cache.h"

const int CACHE_SIZE = 4096
const int KEY = 404
const int SMALLVAL = 8008
const int SMALLVAL_SIZE = 1
// const LARGEVAL which is an array of size 128
const int LARGEVAL_SIZE = 128
// const TOOLARGEVAL which is an array of size 8192
const int TOOLARGEVAL_SIZE = 8192

const int FIFO_EVICTION = 0
const int LRU_EVICTION = 1

// Tests for create_cache
//     Ensure the default hasher works and doesn't return the same number for every input
//     Ensure that the default evictor works (toss in more stuff than max_mem, then check if anything broke and make sure max_mem hasn't been exceeded)

// Helper function for test_hasher
int bad_hash_func(int message) {
    return 1
}

// Helper function for test_create_cache
int lessbad_hash_func(int message) {
    return (message * 3) % 100
}

int test_create_cache() {
    const int SMALL_CACHE_SIZE = 64
    const int LARGE_CACHE_SIZE = (2 ** 24)
    cache_type cache1 = create_cache(CACHE_SIZE)
    // cache2 with evictor but no hasher, once I understand how evictor works
    cache_type cache3 = create_cache(maxmem = CACHE_SIZE, hasher = &lessbad_hash_func)
    // cache4 with evictor and hasher, once I understand how evictor works
    cache_type cache5 = create_cache(SMALL_CACHE_SIZE)
    cache_type cache6 = create_cache(LARGE_CACHE_SIZE)
    return 0
}

int test_hasher() {
    cache_type cache1 = create_cache(CACHE_SIZE)
    cache_type cache2 = create_cache(maxmem = CACHE_SIZE, hasher = &bad_hash_func)
    cache_type cache3 = create_cache(maxmem = CACHE_SIZE, hasher = &lessbad_hash_func)
    // test what happens if a hash function has collisions, and make sure that different inputs don't end up keyed together outside of the collision case; return -1 if anything fails
    return 0
}

int test_evictor() {
    // Create caches with default evictor and a couple me-designed ones, then throw things into the caches to make sure each evictor behaves as expected; specific things to watch out for include max_mem being less than cache_space_used, and wrong items being evicted by the evictors whose workings I understand; return -1 if anything fails
    return 0
}

// Tests for cache_set
//     Try setting various things, including overlarge ones, large numbers of small ones, weird types, et cetera
//     Figure out what's up with val_size being distinct from val (it seems intuitively weird to have that be user-input, that opens up lots of room for mistakes and accordingly for bugs)

// Tests for cache_get
//     Goes along with cache_set, make sure that my various gettings are good
//     Make sure that it returns null if I try to get something not in there

int test_cache_set_and_get() {
    const int WRONG_SIZE = 11
    cache_type cache1 = create_cache(CACHE_SIZE)
    cache_set(cache1, KEY, SMALLVAL, SMALLVAL_SIZE)
    int retrieved_val = cache_get(cache1, KEY, SMALLVAL_SIZE)
    if retrieved_val != SMALLVAL {
        std::cout << "Small value stored or retrieved incorrectly in set/get test.\n"
        return -1
    }
    // Test largeval, make sure returned array is same as input array (I'm sure there's a C++ function to do that easily)
    // Test toolargeval, make sure the cache doesn't break from it
    // Test smallval with wrong_size to see what happens when input size is too small
    // Test largeval with wrong_size to see what happens when input size is too large
    // Test unusedkey to make sure that it's null
    return 0
}

// Tests for cache_delete
//     Delete everything after getting it to make sure that works
//     Also delete some un-gotten stuff to make sure that it's not purely the "get then delete" order of operations that works

int test_cache_delete() {
    const int UNUSEDKEY = 20
    cache_type cache1 = create_cache(CACHE_SIZE)

    cache_set(cache1, KEY, SMALLVAL, SMALLVAL_SIZE)
    int retrieved_val = cache_get(cache1, KEY, SMALLVAL_SIZE)
    if retrieved_val != SMALLVAL {
        std::cout << "Small value stored or retrieved incorrectly in delete test.\n"
        return -1
    }

    cache_delete(cache1, KEY)
    int retrieved_val = cache_get(cache1, KEY, SMALLVAL_SIZE)
    if retrieved_val != NULL {
        std::cout << "Small value was not deleted cleanly.\n"
        return -1
    }

    return 0
}

// Tests for cache_space_used
//     Seems pretty simple, just dump some stuff in the cache and make sure the numbers are reasonable (plus obligatory "what if inputs are too big" test), then delete them to make sure they go down accordingly

int test_cache_space_used() {
    cache_type cache1 = create_cache(CACHE_SIZE)

    int space = cache_space_used(cache1)
    if space != 0 {
        std::cout << "Cache was not initialized with 0 space used.\n"
        return -1
    }

    cache_set(cache1, KEY, SMALLVAL, SMALLVAL_SIZE)
    int space = cache_space_used(cache1)
    if space != SMALLVAL_SIZE {
        std::cout << "Cache failed to add used space from small input value.\n"
        return -1
    }

    cache_set(cache1, (KEY + 1), LARGEVAL, LARGEVAL_SIZE)
    int space = cache_space_used(cache1)
    if space != (SMALLVAL_SIZE + LARGEVAL_SIZE) {
        std::cout << "Cache failed to add used space from large input value.\n"
        return -1
    }

    cache_delete(cache1, KEY)
    int space = cache_space_used(cache1)
    if space != (LARGEVAL_SIZE) {
        std::cout << "Cache failed to remove space from large deleted value.\n"
        return -1
    }

    cache_delete(cache1, (KEY + 1))
    int space = cache_space_used(cache1)
    if space != 0 {
        std::cout << "Cache failed to remove space from small deleted value.\n"
        return -1
    }

    return 0
}

int main() {
    // Call test functions; if they fail toss an error and return -1
    return 0
}