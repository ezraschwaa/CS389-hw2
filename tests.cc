// Alyssa Riceman and Monica Moniot

#include <iostream>
#include <cmath>
#include <string>
#include <memory>
#include <cstdlib>
#include "cache.h"
#include "book.h"
#include "eviction.h"
#include "types.h"

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

// Helper function for reading values
std::string read_val(val_type value) {
    const char* val_as_cstring = static_cast<const char*>(value);
    std::string val_as_string(val_as_cstring);
    return val_as_string;
}

//////////////////////
// Global variables //
//////////////////////

const index_type CACHE_SIZE = 4096;
const index_type SMALL_CACHE_SIZE = 64;
const index_type LARGE_CACHE_SIZE = (pow(2, 24));
const key_type KEY1 = "43";
const key_type KEY2 = "44";
const key_type UNUSEDKEY = "bb";
char* SMALLVAL = make_str_of_defined_length(2); //Vals can't be const because they need to be cast to void*
index_type SMALLVAL_SIZE = 2; //Val sizes can't be const because cache_get requires a non-const val pointer
char* LARGEVAL = make_str_of_defined_length(128);
index_type LARGEVAL_SIZE = 128;

////////////////////
// Test Functions //
////////////////////

// Test to ensure that cache creation and destruction work with different cache sizes and presence or absence of user-input hash functions, to ensure that any errors which may arise from doing so arise
int test_create_cache_and_destroy_cache() {
    cache_type cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
    cache_type cache2 = create_cache(CACHE_SIZE, FIFO, &bad_hash_func);
    cache_type cache3 = create_cache(SMALL_CACHE_SIZE, FIFO, NULL);
    cache_type cache4 = create_cache(LARGE_CACHE_SIZE, FIFO, NULL);

    destroy_cache(cache1);
    destroy_cache(cache2);
    destroy_cache(cache3);
    destroy_cache(cache4);

    return 0;
}

int test_cache_set_and_get(cache_type cache1) {
    cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
    val_type retrieved_val = cache_get(cache1, KEY1, &SMALLVAL_SIZE);
    if (read_val(retrieved_val) != read_val(SMALLVAL)) {
        std::cout << "Small value stored or retrieved incorrectly in set/get test. Stored value: " << read_val(SMALLVAL) << "; retrieved value: " << read_val(retrieved_val) << ".\n";
        return -1;
    }

    cache_set(cache1, KEY1, LARGEVAL, LARGEVAL_SIZE);
    retrieved_val = cache_get(cache1, KEY1, &LARGEVAL_SIZE);
    if (read_val(retrieved_val) != read_val(LARGEVAL)) {
        std::cout << "Large value stored or retrieved incorrectly in set/get test. Stored value: " << read_val(LARGEVAL) << "; retrieved value: " << read_val(retrieved_val) << ".\n";
        return -1;
    }

    retrieved_val = cache_get(cache1, UNUSEDKEY, &SMALLVAL_SIZE);
    if (retrieved_val != NULL) {
        std::cout << "Unassigned key had value initialized already assigned to it. Expected null pointer; received pointer to value " << read_val(retrieved_val) << ".\n";
        return -1;
    }

    return 0;
}

int test_cache_delete(cache_type cache1) {
    cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
    val_type retrieved_val = cache_get(cache1, KEY1, &SMALLVAL_SIZE);
    if (read_val(retrieved_val) != read_val(SMALLVAL)) {
        std::cout << "Small value stored or retrieved incorrectly in delete test. Stored value: " << read_val(SMALLVAL) << "; retrieved value: " << read_val(retrieved_val) << ".\n";
        return -1;
    }

    cache_delete(cache1, KEY1);
    retrieved_val = cache_get(cache1, KEY1, &SMALLVAL_SIZE);
    if (retrieved_val != NULL) {
        std::cout << "Small value was not deleted cleanly. Expected null pointer; received pointer to value " << read_val(retrieved_val) << ".\n";
        return -1;
    }

    cache_set(cache1, KEY1, LARGEVAL, LARGEVAL_SIZE);
    retrieved_val = cache_get(cache1, KEY1, &LARGEVAL_SIZE);
    if (read_val(retrieved_val) != read_val(LARGEVAL)) {
        std::cout << "Large value stored or retrieved incorrectly in delete test. Stored value: " << read_val(LARGEVAL) << "; retrieved value: " << read_val(retrieved_val) << ".\n";
        return -1;
    }

    cache_delete(cache1, KEY1);
    retrieved_val = cache_get(cache1, KEY1, &LARGEVAL_SIZE);
    if (retrieved_val != NULL) {
        std::cout << "Large value was not deleted cleanly. Expected null pointer; received pointer to value " << read_val(retrieved_val) << ".\n";
        return -1;
    }

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
    val_type retrieved_val_1 = cache_get(cache1, KEY1, &SMALLVAL_SIZE);
    val_type retrieved_val_2 = cache_get(cache1, KEY2, &LARGEVAL_SIZE);
    if (read_val(retrieved_val_1) == read_val(retrieved_val_2))
    {
        std::cout << "Non-identical stored values are read as identical on retrieval. Stored values: " << read_val(SMALLVAL) << ", " << read_val(LARGEVAL) << "; retrieved values: " << read_val(retrieved_val_1) << ", " << read_val(retrieved_val_2) << ".\n";
        return -1;
    }

    return 0;
}

int test_evictor(cache_type cache1) {
    index_type largevals_per_cache = CACHE_SIZE / LARGEVAL_SIZE;
    key_type activekey;
    for (index_type i = 0; i <= largevals_per_cache; i++)
    {
        activekey = make_str_of_defined_length(i + 2);
        cache_set(cache1, activekey, LARGEVAL, LARGEVAL_SIZE);
        delete[] activekey;
    }

    activekey = make_str_of_defined_length(2);
    val_type retrieved_val = cache_get(cache1, activekey, &LARGEVAL_SIZE);
    if (retrieved_val != NULL)
    {
        std::cout << "Cache did not evict expected piece of memory under FIFO policy.\n";
        delete[] activekey;
        return -1;
    }
    delete[] activekey;

    activekey = make_str_of_defined_length(3);
    retrieved_val = cache_get(cache1, activekey, &LARGEVAL_SIZE);
    if (read_val(retrieved_val) != read_val(LARGEVAL))
    {
        std::cout << "Cache evicted an unexpected piece of memory under FIFO policy.\n";
        delete[] activekey;
        return -1;
    }
    delete[] activekey;

    //Test LRU evictor policy upon working out the bugs in FIFO test

    return 0;
}

int test_resizing(cache_type cache1) {
    const int ONE_MORE_THAN_CAPACITY = 129; //Should be enough to make the cache resize, or to throw an error if it fails

    key_type activekey;
    for (index_type i = 0; i < ONE_MORE_THAN_CAPACITY; i++) {
        activekey = make_str_of_defined_length(i + 2);
        cache_set(cache1, activekey, LARGEVAL, LARGEVAL_SIZE);
        delete[] activekey;
    }
}

int test_serialize(cache_type cache1) {
    cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);

    Mem_array serialized = serialize_cache(cache1);
    cache_type deserialized = deserialize_cache(serialized);

    val_type retrieved_val = cache_get(cache1, KEY1, &SMALLVAL_SIZE);
    if (read_val(retrieved_val) != read_val(SMALLVAL)) {
        std::cout << "Serialization or deserialization failed. Stored value before serialization: " << read_val(SMALLVAL) << "; retrieved value after deserialization: " << read_val(retrieved_val) << ".\n";
        return -1;
    }

    delete[] serialized.data;
    destroy_cache(deserialized);

    return 0;
}

int compositional_testing(uint32_t test_iters, uint32_t internal_iters) {
    int32_t external_error_pile = 0;
    for (uint32_t i = 0; i < test_iters; i++) {
        evictor_type evictor; //Cycle through evictor types for tested caches
        if (i % 2 == 0) {
            evictor = FIFO;
        } else {
            evictor = LRU;
        }

        cache_type tested_cache; //Make cache, cycling through hash functions
        if (i % 4 <= 1) {
            tested_cache = create_cache(CACHE_SIZE, evictor, NULL);
        } else {
            tested_cache = create_cache(CACHE_SIZE, evictor, &bad_hash_func);
        }

        int32_t internal_error_pile = 0;

        for (uint32_t j = 0; j < internal_iters; j++) { //Internal loop, runs a series of randomized tests on the cache
            uint32_t next_test_to_run = (std::rand() % 7);
            switch(j) {
                case 0: 
                    internal_error_pile += test_cache_set_and_get(tested_cache);
                    break;
                case 1: 
                    internal_error_pile += test_cache_delete(tested_cache);
                    break;
                case 2: 
                    internal_error_pile += test_cache_space_used(tested_cache);
                    break;
                case 3:
                    internal_error_pile += test_hasher(tested_cache);
                    break;
                case 4:
                    internal_error_pile += test_evictor(tested_cache);
                    break;
                case 5:
                    internal_error_pile += test_resizing(tested_cache);
                    break;
                case 6:
                    // internal_error_pile += test_serialize(tested_cache);
                    break;
            }
        }

        destroy_cache(tested_cache);

        if (internal_error_pile < 0) { //Report state of cache when bugs came up
            external_error_pile += internal_error_pile;
            std::string evictor_debug;
            std::string hasher_debug;
            if (evictor == LRU) {
                evictor_debug = "LRU";
            } else {
                evictor_debug = "FIFO";
            }
            if (i % 4 <= 1) {
                hasher_debug = "the default hasher";
            } else {
                hasher_debug = "a user-input hasher";
            }
            std::cout << "The above " << (internal_error_pile * -1) << " errors occurred with an " << evictor_debug << "eviction policy and " << hasher_debug << ".";
        }
    }
    return external_error_pile;
}

int main() {
    int32_t error_pile = 0;

    // error_pile += test_create_cache_and_destroy_cache();

    cache_type cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
    // error_pile += test_cache_set_and_get(cache1);
    // destroy_cache(cache1);

    // cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
    // error_pile += test_cache_delete(cache1);
    // destroy_cache(cache1);

    // cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
    // error_pile += test_cache_space_used(cache1);
    // destroy_cache(cache1);

    // cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
    // error_pile += test_hasher(cache1);
    // destroy_cache(cache1);

    // cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
    // error_pile += test_evictor(cache1);
    // destroy_cache(cache1);

    // cache1 = create_cache(CACHE_SIZE, LRU, NULL);
    // error_pile += test_evictor(cache1);
    // destroy_cache(cache1);

    // cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
    // error_pile += test_resizing(cache1);
    // destroy_cache(cache1);

    // cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
    error_pile += test_serialize(cache1);
    destroy_cache(cache1);

    // error_pile += compositional_testing(256, 500);

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