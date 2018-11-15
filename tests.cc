// Alyssa Riceman and Monica Moniot

#include <iostream>
#include <cmath>
#include <string>
#include <cstdlib>
#include <ctime>
#include <string.h>
#include "cache.h"

//////////////////////
// Helper Functions //
//////////////////////

// Helper function for generating strings of at least one "a"
char* make_str_of_defined_length(index_type length) {
    char* newstr = new char[length];
    for(index_type i = 0; i < (length - 1); i++) {
        newstr[i] = 'a';
    }
    newstr[length - 1] = 0;
    return newstr;
}

// Helper function for test_create_cache and test_hasher
index_type bad_hash_func(const char* message) {
    return message[0];
}

// // Helper function for reading values
// const char* read_val(val_type value) {
//     const char* val_as_cstr = static_cast<const char*>(value);
// }

//////////////////////
// Global variables //
//////////////////////

const index_type CACHE_SIZE = 4096;
const index_type SMALL_CACHE_SIZE = 64;
const index_type LARGE_CACHE_SIZE = (pow(2, 24));
const key_type KEY1 = "43";
const key_type KEY2 = "44";
const key_type UNUSEDKEY = "bb";
const char* SMALLVAL = make_str_of_defined_length(2); //Vals can't be const because they need to be cast to void*
const index_type SMALLVAL_SIZE = 1; //Val sizes can't be const because cache_get requires a non-const val pointer; size is 1 below size of str length because null terminators aren't part of the val
const char* LARGEVAL = make_str_of_defined_length(128);
const index_type LARGEVAL_SIZE = 127;
index_type valsize1_for_get = 0;
index_type valsize2_for_get = 0;

////////////////////
// Test Functions //
////////////////////

// Test to ensure that cache creation and destruction work with different cache sizes and presence or absence of user-input hash functions, to ensure that any errors which may arise from doing so arise
int test_create_cache_and_destroy_cache() {
    cache_type cache1 = create_cache(CACHE_SIZE, NULL);
    cache_type cache2 = create_cache(CACHE_SIZE, &bad_hash_func);
    cache_type cache3 = create_cache(SMALL_CACHE_SIZE, NULL);
    cache_type cache4 = create_cache(LARGE_CACHE_SIZE, NULL);

    // destroy_cache(cache1);
    // destroy_cache(cache2);
    // destroy_cache(cache3);
    // destroy_cache(cache4);

    return 0;
}

int test_cache_set_and_get(cache_type cache1) {
    cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
    val_type retrieved_val = cache_get(cache1, KEY1, &valsize1_for_get);
    if ((valsize1_for_get != SMALLVAL_SIZE) || memcmp(retrieved_val, SMALLVAL, SMALLVAL_SIZE)) {
        printf("Small value stored or retrieved incorrectly in set/get test. Stored value: %.*s; retrieved value: %.*s; sizes: %d, %d.\n", SMALLVAL_SIZE, SMALLVAL, valsize1_for_get, static_cast<const char*>(retrieved_val), SMALLVAL_SIZE, valsize1_for_get);
        return -1;
    }
    delete[] static_cast<const char*>(retrieved_val);

    cache_set(cache1, KEY1, LARGEVAL, LARGEVAL_SIZE);
    retrieved_val = cache_get(cache1, KEY1, &valsize1_for_get);
    if ((valsize1_for_get != LARGEVAL_SIZE) || memcmp(retrieved_val, LARGEVAL, LARGEVAL_SIZE)) {
        printf("Large value stored or retrieved incorrectly in set/get test. Stored value: %.*s; retrieved value: %.*s; sizes: %d, %d.\n", LARGEVAL_SIZE, LARGEVAL, valsize1_for_get, static_cast<const char*>(retrieved_val), LARGEVAL_SIZE, valsize1_for_get);
        return -1;
    }
    delete[] static_cast<const char*>(retrieved_val);

    retrieved_val = cache_get(cache1, UNUSEDKEY, &valsize1_for_get);
    if (retrieved_val != NULL) {
        printf("Unassigned key had value initialized already assigned to it. Expected null pointer; received pointer to value %.*s.\n", valsize1_for_get, static_cast<const char*>(retrieved_val));
        return -1;
    }
    delete[] static_cast<const char*>(retrieved_val);

    return 0;
}

int test_cache_delete(cache_type cache1) {
    cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
    val_type retrieved_val = cache_get(cache1, KEY1, &valsize1_for_get);
    if ((valsize1_for_get != SMALLVAL_SIZE) || memcmp(retrieved_val, SMALLVAL, SMALLVAL_SIZE)) {
        printf("Small value stored or retrieved incorrectly in delete test. Stored value: %.*s; retrieved value: %.*s; sizes: %d, %d.\n", SMALLVAL_SIZE, SMALLVAL, valsize1_for_get, static_cast<const char*>(retrieved_val), SMALLVAL_SIZE, valsize1_for_get);
        return -1;
    }
    delete[] static_cast<const char*>(retrieved_val);

    cache_delete(cache1, KEY1);
    retrieved_val = cache_get(cache1, KEY1, &valsize1_for_get);
    if (retrieved_val != NULL) {
        printf("Small value was not deleted cleanly. Expected null pointer; received pointer to value %.*s.\n", valsize1_for_get, static_cast<const char*>(retrieved_val));
        return -1;
    }
    delete[] static_cast<const char*>(retrieved_val);

    cache_set(cache1, KEY1, LARGEVAL, LARGEVAL_SIZE);
    retrieved_val = cache_get(cache1, KEY1, &valsize1_for_get);
    if ((valsize1_for_get != LARGEVAL_SIZE) || memcmp(retrieved_val, LARGEVAL, LARGEVAL_SIZE)) {
        printf("Large value stored or retrieved incorrectly in delete test. Stored value: %.*s; retrieved value: %.*s; sizes: %d, %d.\n", LARGEVAL_SIZE, LARGEVAL, valsize1_for_get, static_cast<const char*>(retrieved_val), LARGEVAL_SIZE, valsize1_for_get);
        return -1;
    }
    delete[] static_cast<const char*>(retrieved_val);

    cache_delete(cache1, KEY1);
    retrieved_val = cache_get(cache1, KEY1, &valsize1_for_get);
    if (retrieved_val != NULL) {
        printf("Large value was not deleted cleanly. Expected null pointer; received pointer to value %.*s.\n", valsize1_for_get, static_cast<const char*>(retrieved_val));
        return -1;
    }
    delete[] static_cast<const char*>(retrieved_val);

    cache_delete(cache1, UNUSEDKEY); //Makes sure no error arises from destroying something nonexistent

    return 0;
}

int test_cache_space_used(cache_type cache1) {
    const index_type space1 = cache_space_used(cache1);
    if (space1 != 0) {
        std::cout << "Cache was not initialized with 0 space used. Space filled on initialization: " << space1 << ".\n";
        return -1;
    }

    cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
    const index_type space2 = cache_space_used(cache1);
    if (space2 != SMALLVAL_SIZE) {
        std::cout << "Cache failed to add used space from small input value. Previous space used: " << space1 << "; expected space used: " << SMALLVAL_SIZE << "; reported space used: " << space2 << ".\n";
        return -1;
    }

    cache_set(cache1, KEY2, LARGEVAL, LARGEVAL_SIZE);
    const index_type space3 = cache_space_used(cache1);
    if (space3 != (SMALLVAL_SIZE + LARGEVAL_SIZE)) {
        std::cout << "Cache failed to add used space from large input value. Previous space used: " << space2 << "; expected space used: " << (SMALLVAL_SIZE + LARGEVAL_SIZE) << "; reported space used: " << space3 << ".\n";
        return -1;
    }

    cache_delete(cache1, KEY1);
    const index_type space4 = cache_space_used(cache1);
    if (space4 != LARGEVAL_SIZE) {
        std::cout << "Cache failed to remove space from large deleted value. Previous space used: " << space3 << "; expected new space used: " << LARGEVAL_SIZE << "reported new space used: " << space4 << ".\n";
        return -1;
    }

    cache_delete(cache1, KEY2);
    const index_type space5 = cache_space_used(cache1);
    if (space5 != 0) {
        std::cout << "Cache failed to remove space from small deleted value. Previous space used: " << space4 << "; expected new space used: 0; reported new space used: " << space5 << ".\n";
        return -1;
    }

    return 0;
}

int test_hasher(cache_type cache1) {
    cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
    cache_set(cache1, KEY2, LARGEVAL, LARGEVAL_SIZE);
    val_type retrieved_val_1 = cache_get(cache1, KEY1, &valsize1_for_get);
    val_type retrieved_val_2 = cache_get(cache1, KEY2, &valsize2_for_get);
    if ((valsize1_for_get == valsize2_for_get) && !memcmp(retrieved_val_1, retrieved_val_2, valsize1_for_get)) {
        printf("Non-identical stored values are read as identical on retrieval. Stored values: %.*s, %.*s; retrieved values: %.*s, %.*s; sizes of retrieved values: %d, %d.\n", SMALLVAL_SIZE, SMALLVAL, LARGEVAL_SIZE, LARGEVAL, valsize1_for_get, static_cast<const char*>(retrieved_val_1), valsize2_for_get, static_cast<const char*>(retrieved_val_2), valsize1_for_get, valsize2_for_get);
        return -1;
    }
    delete[] static_cast<const char*>(retrieved_val_1);
    delete[] static_cast<const char*>(retrieved_val_2);

    return 0;
}

// int test_evictor(cache_type cache1) {
//     index_type largevals_per_cache = CACHE_SIZE / LARGEVAL_SIZE;
//     key_type activekey;
//     for (index_type i = 0; i <= largevals_per_cache; i++)
//     {
//         activekey = make_str_of_defined_length(i + 2);
//         cache_set(cache1, activekey, LARGEVAL, LARGEVAL_SIZE);
//         delete[] activekey;
//     }

//     activekey = make_str_of_defined_length(2);
//     val_type retrieved_val = cache_get(cache1, activekey, &valsize1_for_get);
//     if (retrieved_val != NULL)
//     {
//         std::cout << "Cache did not evict expected piece of memory under FIFO policy.\n";
//         delete[] activekey;
//         return -1;
//     }
//     delete[] activekey;

//     activekey = make_str_of_defined_length(3);
//     retrieved_val = cache_get(cache1, activekey, &valsize1_for_get);
//     if (read_val(retrieved_val) != read_val(LARGEVAL))
//     {
//         std::cout << "Cache evicted an unexpected piece of memory under FIFO policy.\n";
//         delete[] activekey;
//         return -1;
//     }
//     delete[] activekey;

//     return 0;
// }

int test_resizing(cache_type cache1) {
    const int ONE_MORE_THAN_CAPACITY = 129; //Should be enough to make the cache resize, or to throw an error if it fails

    key_type activekey;
    for (index_type i = 0; i < ONE_MORE_THAN_CAPACITY; i++) {
        activekey = make_str_of_defined_length(i + 2);
        cache_set(cache1, activekey, LARGEVAL, LARGEVAL_SIZE);
        delete[] activekey;
    }

    return 0;
}

int compositional_testing(uint32_t test_iters, uint32_t internal_iters) {
    int32_t external_error_pile = 0;
    for (uint32_t i = 0; i < test_iters; i++) {
        // struct timespec time; //Seed randomizer with computer clock
        // clock_gettime(CLOCK_MONOTONIC, &time);
        // srand(time.tv_nsec);

        cache_type tested_cache = create_cache(CACHE_SIZE, NULL);
        int32_t internal_error_pile = 0;

        for (uint32_t j = 0; j < internal_iters; j++) { //Internal loop, runs a series of randomized tests on the cache
            uint32_t next_test_to_run = (std::rand() % 4);
            switch(next_test_to_run) {
                case 0: 
                    internal_error_pile += test_cache_set_and_get(tested_cache);
                    break;
                case 1: 
                    internal_error_pile += test_cache_delete(tested_cache);
                    break;
                case 2:
                    internal_error_pile += test_hasher(tested_cache);
                    break;
                case 4:
                    internal_error_pile += test_resizing(tested_cache);
                    break;
            }
        }

        // destroy_cache(tested_cache);

        if (internal_error_pile < 0) { //Report state of cache when bugs came up
            external_error_pile += internal_error_pile;
            std::string hasher_debug;
            if (i % 4 <= 1) {
                hasher_debug = "the default hasher";
            } else {
                hasher_debug = "a user-input hasher";
            }
            std::cout << "The above " << (internal_error_pile * -1) << " errors occurred with " << hasher_debug << ".\n";
        }
    }

    return external_error_pile;
}

int main() {
    int32_t error_pile = 0;

    // error_pile += test_create_cache_and_destroy_cache();

    cache_type cache1 = create_cache(CACHE_SIZE, NULL);
    error_pile += test_cache_set_and_get(cache1);
    error_pile += test_cache_delete(cache1);
    error_pile += test_hasher(cache1);

    // error_pile += test_resizing(cache1);
    // error_pile += test_cache_space_used(cache1);
    // error_pile += test_evictor(cache1););
    // error_pile += test_serialize(cache1);

    // error_pile += compositional_testing(16, 20);
    destroy_cache(cache1);

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