// Alyssa Riceman and Monica Moniot

#include <iostream>
#include <cmath>
#include <string>
#include <memory>
#include "cache.h"

// Deleting the resulting pointer is warning-provoking; fix later
// Helper function for generating test strings of at least one "a"
char* make_str_of_defined_length(index_type length) {
    char* newstr = new char[length];
    for(index_type i = 0; i < (length - 1); i++) {
        newstr[i] = 'a';
    }
    newstr[length - 1] = 0;
    return newstr;
}

const index_type CACHE_SIZE = 4096;
const key_type KEY1 = "43";
const key_type KEY2 = "44";
const key_type UNUSEDKEY = "bb";
char* SMALLVAL = make_str_of_defined_length(2);
index_type SMALLVAL_SIZE = 2; //Val sizes can't be const because cache_get requires a non-const val pointer
char* LARGEVAL = make_str_of_defined_length(128);
index_type LARGEVAL_SIZE = 128;

// Helper function for test_hasher and test_create_cache
index_type bad_hash_func_2(const char* message) {
    return message[0];
}

// Helper function for test_hasher
index_type bad_hash_func_1(const char* message) {
    if (sizeof(message) < 5) {
        return message[0];
    } else {
        return message[1];
    }
}

// Helper function for reading values
std::string read_val(val_type value) {
    const char* val_as_cstring = static_cast<const char*>(value);
    std::string val_as_string(val_as_cstring);
    return val_as_string;
}

// Ensure that cache creation works with different cache sizes and presence or absent of user-input hash functions
int test_create_cache() {
    const index_type SMALL_CACHE_SIZE = 64;
    const index_type LARGE_CACHE_SIZE = (pow(2, 24));

    create_cache(CACHE_SIZE, FIFO, NULL);
    create_cache(CACHE_SIZE, FIFO, &bad_hash_func_1);
    create_cache(SMALL_CACHE_SIZE, FIFO, NULL);
    create_cache(LARGE_CACHE_SIZE, FIFO, NULL);

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

int test_cache_set_and_get() {
    cache_type cache1 = create_cache(CACHE_SIZE, FIFO, NULL);

    cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
    val_type retrieved_val = cache_get(cache1, KEY1, &SMALLVAL_SIZE);
    if (read_val(retrieved_val) != read_val(SMALLVAL)) {
        std::cout << "Small value stored or retrieved incorrectly in set/get test.\n";
        return -1;
    }

    cache_set(cache1, KEY1, LARGEVAL, LARGEVAL_SIZE);
    retrieved_val = cache_get(cache1, KEY1, &LARGEVAL_SIZE);
    if (read_val(retrieved_val) != read_val(LARGEVAL)) {
        std::cout << "Large value stored or retrieved incorrectly in set/get test.\n";
        return -1;
    }

    retrieved_val = cache_get(cache1, UNUSEDKEY, &SMALLVAL_SIZE);
    if (retrieved_val != NULL) {
        std::cout << "Value retrieved without being assigned.\n";
        return -1;
    }

    return 0;
}

// Tests for cache_delete
//     Delete everything after getting it to make sure that works
//     Also delete some un-gotten stuff to make sure that it's not purely the "get then delete" order of operations that works

int test_cache_delete() {
    cache_type cache1 = create_cache(CACHE_SIZE, FIFO, NULL);

    cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
    val_type retrieved_val = cache_get(cache1, KEY1, &SMALLVAL_SIZE);
    if (read_val(retrieved_val) != read_val(SMALLVAL)) {
        std::cout << "Small value stored or retrieved incorrectly in delete test.\n";
        return -1;
    }

    cache_delete(cache1, KEY1);
    retrieved_val = cache_get(cache1, KEY1, &SMALLVAL_SIZE);
    if (retrieved_val != NULL) {
        std::cout << "Small value was not deleted cleanly.\n";
        return -1;
    }

    cache_delete(cache1, UNUSEDKEY);

    return 0;
}

// Tests for cache_space_used
//     Seems pretty simple, just dump some stuff in the cache and make sure the numbers are reasonable (plus obligatory "what if inputs are too big" test), then delete them to make sure they go down accordingly

int test_cache_space_used() {
    cache_type cache1 = create_cache(CACHE_SIZE, FIFO, NULL);

    const index_type space1 = cache_space_used(cache1);
    if (space1 != 0) {
        std::cout << "Cache was not initialized with 0 space used. Space filled on initialization: " << space1 << "\n";
        return -1;
    }

    cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
    const index_type space2 = cache_space_used(cache1);
    if (space2 != SMALLVAL_SIZE) {
        std::cout << "Cache failed to add used space from small input value. Previous space used: " << space1 << "; expected space used: " << SMALLVAL_SIZE << "; reported space used: " << space2 << "\n";
        return -1;
    }

    cache_set(cache1, KEY2, LARGEVAL, LARGEVAL_SIZE);
    const index_type space3 = cache_space_used(cache1);
    if (space3 != (SMALLVAL_SIZE + LARGEVAL_SIZE)) {
        std::cout << "Cache failed to add used space from large input value. Previous space used: " << space2 << "; expected space used: " << (SMALLVAL_SIZE + LARGEVAL_SIZE) << "; reported space used: " << space3 << "\n";
        return -1;
    }

    cache_delete(cache1, KEY1);
    const index_type space4 = cache_space_used(cache1);
    if (space4 != LARGEVAL_SIZE) {
        std::cout << "Cache failed to remove space from large deleted value. Previous space used: " << space3 << "; expected new space used: " << LARGEVAL_SIZE << "reported new space used: " << space4 << "\n";
        return -1;
    }

    cache_delete(cache1, KEY2);
    const index_type space5 = cache_space_used(cache1);
    if (space5 != 0) {
        std::cout << "Cache failed to remove space from small deleted value. Previous space used: " << space4 << "; expected new space used: 0; reported new space used: " << space5 << "\n";
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

    delete[] SMALLVAL;
    delete[] LARGEVAL;

    if (error_pile < -1) {
        std::cout << "Errors remain.\n";
        return -1;
    } else if (error_pile == -1 ) {
        std::cout << "One error remains.\n";
        return -1;
    } else {
        std::cout << "No errors detected!\n";
        return 0;
    }
}