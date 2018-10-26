// Alyssa Riceman and Monica Moniot

#include <iostream>
#include <cmath>
#include <string>
#include <cstdlib>
#include <ctime>
#include "cache.hh"

using key_type = Cache::key_type;
using index_type = Cache::index_type;
using val_type = Cache::val_type;
using hash_func = Cache::hash_func;
using evictor_type = Cache::evictor_type;

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

int test_create_cache_and_destroy_cache() { // No in-test potential errors, but if create or destroy returns error this will error accordingly
    Cache cache1(CACHE_SIZE, NULL, NULL);
    // Cache cache2(SMALL_CACHE_SIZE, NULL, NULL);
    // Cache cache3(LARGE_CACHE_SIZE, NULL, NULL);

    // delete cache1;
    // destroy_cache(cache2);
    // destroy_cache(cache3);

    return 0;
}

// int test_cache_set_and_get_one_val(cache_type cache1) {
//     cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
//     val_type retrieved_val = cache_get(cache1, KEY1, &SMALLVAL_SIZE);
//     if (read_val(retrieved_val) != read_val(SMALLVAL)) {
//         std::cout << "Value stored or retrieved incorrectly in one-val test. Stored value: " << read_val(SMALLVAL) << "; retrieved value: " << read_val(retrieved_val) << ".\n";
//         return -1;
//     }

//     return 0;
// }

// int test_cache_set_and_get_two_vals_same_key(cache_type cache1) {
//     cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
//     cache_set(cache1, KEY1, LARGEVAL, LARGEVAL_SIZE);
//     val_type retrieved_val = cache_get(cache1, KEY1, &LARGEVAL_SIZE);
//     if (read_val(retrieved_val) != read_val(LARGEVAL)) {
//         std::cout << "Value stored or retrieved incorrectly in two-vals same-key test. Stored value: " << read_val(LARGEVAL) << "; retrieved value: " << read_val(retrieved_val) << ".\n";
//         return -1;
//     }

//     return 0;
// }

// int test_cache_set_and_get_two_vals_diff_keys(cache_type cache1) {
//     cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
//     cache_set(cache1, KEY2, LARGEVAL, LARGEVAL_SIZE);
//     val_type retrieved_val = cache_get(cache1, KEY2, &LARGEVAL_SIZE);
//     if (read_val(retrieved_val) != read_val(LARGEVAL)) {
//         std::cout << "Value stored or retrieved incorrectly in two-vals different-key test. Stored value: " << read_val(LARGEVAL) << "; retrieved value: " << read_val(retrieved_val) << ".\n";
//         return -1;
//     }

//     return 0;
// }

// int test_cache_get_unassignedval(cache_type cache1) {
//     val_type retrieved_val = cache_get(cache1, UNUSEDKEY, &SMALLVAL_SIZE);
//     if (retrieved_val != NULL) {
//         std::cout << "Unassigned key had value initialized already assigned to it. Expected null pointer; received pointer to value " << read_val(retrieved_val) << ".\n";
//         return -1;
//     }

//     return 0;
// }

// int test_cache_delete_one_val(cache_type cache1) {
//     cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
//     cache_delete(cache1, KEY1);
//     retrieved_val = cache_get(cache1, KEY1, &SMALLVAL_SIZE);
//     if (retrieved_val != NULL) {
//         std::cout << "Value was not deleted cleanly in one-val test. Expected null pointer; received pointer to value " << read_val(retrieved_val) << ".\n";
//         return -1;
//     }

//     return 0;
// }

// int test_cache_delete_two_vals_same_key(cache_type cache1) {
//     cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
//     cache_set(cache1, KEY1, LARGEVAL, LARGEVAL_SIZE);
//     cache_delete(cache1, KEY1);
//     retrieved_val = cache_get(cache1, KEY1, &LARGEVAL_SIZE);
//     if (retrieved_val != NULL) {
//         std::cout << "Value was not deleted cleanly in two-val same-key test. Expected null pointer; received pointer to value " << read_val(retrieved_val) << ".\n";
//         return -1;
//     }

//     return 0;
// }

// int test_cache_delete_two_vals_diff_keys(cache_type cache1) {
//     cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
//     cache_set(cache1, KEY2, LARGEVAL, LARGEVAL_SIZE);
//     cache_delete(cache1, KEY2);
//     retrieved_val = cache_get(cache1, KEY1, &LARGEVAL_SIZE);
//     if (retrieved_val != NULL) {
//         std::cout << "Value was not deleted cleanly in two-val different-key test. Expected null pointer; received pointer to value " << read_val(retrieved_val) << ".\n";
//         return -1;
//     }

//     return 0;
// }

// int test_cache_delete_unusedval{ //Makes sure no error arises from deleting something nonexistent
//     cache_delete(cache1, UNUSEDKEY);

//     return 0;
// }

// int test_cache_space_used_on_initialization() {
//     cache_type cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
//     const index_type space = cache_space_used(cache1);
//     if (space != 0) {
//         std::cout << "Cache was not initialized with 0 space used. Space filled on initialization: " << space1 << ".\n";
//         return -1;
//     }
//     destroy_cache(cache1);

//     return 0;
// }

// int test_cache_space_used_one_val() {
//     cache_type cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
//     cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
//     const index_type space = cache_space_used(cache1);
//     if (space != SMALLVAL_SIZE) {
//         std::cout << "Cache failed to add used space from one-val input. Previous space used: " << space1 << "; expected space used: " << SMALLVAL_SIZE << "; reported space used: " << space2 << ".\n";
//         return -1;
//     }
//     destroy_cache(cache1);

//     return 0;
// }

// int test_cache_space_used_two_vals_same_key() {
//     cache_type cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
//     cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
//     cache_set(cache1, KEY1, LARGEVAL, LARGEVAL_SIZE);
//     const index_type space = cache_space_used(cache1);
//     if (space != (LARGEVAL_SIZE)) {
//         std::cout << "Cache failed to add used space from two-val same-key input. Previous space used: " << space2 << "; expected space used: " << (SMALLVAL_SIZE + LARGEVAL_SIZE) << "; reported space used: " << space3 << ".\n";
//         return -1;
//     }
//     destroy_cache(cache1);

//     return 0;
// }

// int test_cache_space_used_two_vals_diff_keys() {
//     cache_type cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
//     cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
//     cache_set(cache1, KEY2, LARGEVAL, LARGEVAL_SIZE);
//     const index_type space = cache_space_used(cache1);
//     if (space != (SMALLVAL_SIZE + LARGEVAL_SIZE)) {
//         std::cout << "Cache failed to add used space from two-val different-key input. Previous space used: " << space2 << "; expected space used: " << (SMALLVAL_SIZE + LARGEVAL_SIZE) << "; reported space used: " << space3 << ".\n";
//         return -1;
//     }
//     destroy_cache(cache1);

//     return 0;
// }

// int test_cache_space_used_deletion_one_val() {
//     cache_type cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
//     cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
//     cache_delete(cache1, KEY1);
//     const index_type space = cache_space_used(cache1);
//     if (space != 0) {
//         std::cout << "Cache failed to remove space from deletion of single value. Previous space used: " << space3 << "; expected new space used: " << LARGEVAL_SIZE << "reported new space used: " << space4 << ".\n";
//         return -1;
//     }
//     destroy_cache(cache1);

//     return 0;
// }

// int test_cache_space_used_deletion_two_vals_diff_keys() {
//     cache_type cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
//     cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
//     cache_set(cache1, KEY2, LARGEVAL, LARGEVAL_SIZE);
//     cache_delete(cache1, KEY2);
//     const index_type space = cache_space_used(cache1);
//     if (space != SMALLVAL_SIZE) {
//         std::cout << "Cache failed to remove space from small deleted value. Previous space used: " << space4 << "; expected new space used: 0; reported new space used: " << space5 << ".\n";
//         return -1;
//     }
//     destroy_cache(cache1);

//     return 0;
// }

// int test_hasher(cache_type cache1) {
//     cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);
//     cache_set(cache1, KEY2, LARGEVAL, LARGEVAL_SIZE);
//     val_type retrieved_val_1 = cache_get(cache1, KEY1, &SMALLVAL_SIZE);
//     val_type retrieved_val_2 = cache_get(cache1, KEY2, &LARGEVAL_SIZE);
//     if (read_val(retrieved_val_1) == read_val(retrieved_val_2)) {
//         std::cout << "Non-identical stored values are read as identical on retrieval. Stored values: " << read_val(SMALLVAL) << ", " << read_val(LARGEVAL) << "; retrieved values: " << read_val(retrieved_val_1) << ", " << read_val(retrieved_val_2) << ".\n";
//         return -1;
//     }

//     return 0;
// }

// // int test_evictor_expected_deletion(cache_type cache1) {
// //     index_type largevals_per_cache = CACHE_SIZE / LARGEVAL_SIZE;
// //     key_type activekey;
// //     for (index_type i = 0; i <= largevals_per_cache; i++) {
// //         activekey = make_str_of_defined_length(i + 2);
// //         cache_set(cache1, activekey, LARGEVAL, LARGEVAL_SIZE);
// //         delete[] activekey;
// //     }

// //     activekey = make_str_of_defined_length(2);
// //     val_type retrieved_val = cache_get(cache1, activekey, &LARGEVAL_SIZE);
// //     if (retrieved_val != NULL) {
// //         std::cout << "Cache did not evict expected piece of memory.\n";
// //         delete[] activekey;
// //         return -1;
// //     }
// //     delete[] activekey;

// //     return 0;
// // }

// // int test_evictor_expected_nondeletion(cache_type cache1) {
// //     index_type largevals_per_cache = CACHE_SIZE / LARGEVAL_SIZE;
// //     key_type activekey;
// //     for (index_type i = 0; i <= largevals_per_cache; i++) {
// //         activekey = make_str_of_defined_length(i + 2);
// //         cache_set(cache1, activekey, LARGEVAL, LARGEVAL_SIZE);
// //         delete[] activekey;
// //     }

// //     activekey = make_str_of_defined_length(3);
// //     val_type retrieved_val = cache_get(cache1, activekey, &LARGEVAL_SIZE);
// //     if (retrieved_val != read_val(LARGEVAL)) {
// //         std::cout << "Cache evicted an unexpected piece of memory under FIFO policy.\n";
// //         delete[] activekey;
// //         return -1;
// //     }
// //     delete[] activekey;

// //     return 0;
// // }

// // int test_resizing(cache_type cache1) { // Doesn't work when writing tests purely to the .hh
// //     const int ONE_MORE_THAN_CAPACITY = 129; //Should be enough to make the cache resize, or to throw an error if it fails

// //     key_type activekey;
// //     for (index_type i = 0; i < ONE_MORE_THAN_CAPACITY; i++) {
// //         activekey = make_str_of_defined_length(i + 2);
// //         cache_set(cache1, activekey, LARGEVAL, LARGEVAL_SIZE);
// //         delete[] activekey;
// //     }

// //     return 0;
// // }

// // int test_serialize(cache_type cache1) { // Doesn't work when writing tests purely to the .hh
// //     cache_set(cache1, KEY1, SMALLVAL, SMALLVAL_SIZE);

// //     Mem_array serialized = serialize_cache(cache1);
// //     cache_type deserialized = deserialize_cache(serialized);

// //     val_type retrieved_val = cache_get(cache1, KEY1, &SMALLVAL_SIZE);
// //     if (read_val(retrieved_val) != read_val(SMALLVAL)) {
// //         std::cout << "Serialization or deserialization failed. Stored value before serialization: " << read_val(SMALLVAL) << "; retrieved value after deserialization: " << read_val(retrieved_val) << ".\n";
// //         return -1;
// //     }

// //     delete[] serialized.data;
// //     destroy_cache(deserialized);

// //     return 0;
// // }

// int compositional_testing(uint32_t test_iters, uint32_t internal_iters) {
//     int32_t external_error_pile = 0;
//     for (uint32_t i = 0; i < test_iters; i++) {
//         // struct timespec time; //Seed randomizer with computer clock
//         // clock_gettime(CLOCK_MONOTONIC, &time);
//         // srand(time.tv_nsec);

//         evictor_type evictor; //Cycle through evictor types for tested caches
//         if (i % 2 == 0) {
//             evictor = FIFO;
//         } else {
//             evictor = LRU;
//         }

//         cache_type tested_cache; //Make cache, cycling through hash functions
//         if (i % 4 <= 1) {
//             tested_cache = create_cache(CACHE_SIZE, evictor, NULL);
//         } else {
//             tested_cache = create_cache(CACHE_SIZE, evictor, &bad_hash_func);
//         }

//         int32_t internal_error_pile = 0;

//         for (uint32_t j = 0; j < internal_iters; j++) { //Internal loop, runs a series of randomized tests on the cache
//             uint32_t next_test_to_run = (std::rand() % 6);
//             switch(next_test_to_run) {
//                 case 0: 
//                     internal_error_pile += test_cache_set_and_get(tested_cache);
//                     break;
//                 case 1: 
//                     internal_error_pile += test_cache_delete(tested_cache);
//                     break;
//                 case 2:
//                     internal_error_pile += test_hasher(tested_cache);
//                     break;
//                 case 3:
//                     internal_error_pile += test_evictor(tested_cache);
//                     break;
//                 case 4:
//                     internal_error_pile += test_resizing(tested_cache);
//                     break;
//                 case 5:
//                     // internal_error_pile += test_serialize(tested_cache);
//                     break;
//             }
//         }

//         destroy_cache(tested_cache);

//         if (internal_error_pile < 0) { //Report state of cache when bugs came up
//             external_error_pile += internal_error_pile;
//             std::string evictor_debug;
//             std::string hasher_debug;
//             if (evictor == LRU) {
//                 evictor_debug = "LRU";
//             } else {
//                 evictor_debug = "FIFO";
//             }
//             if (i % 4 <= 1) {
//                 hasher_debug = "the default hasher";
//             } else {
//                 hasher_debug = "a user-input hasher";
//             }
//             std::cout << "The above " << (internal_error_pile * -1) << " errors occurred with an " << evictor_debug << "eviction policy and " << hasher_debug << ".\n";
//         }
//     }
//     return external_error_pile;
// }

int main() {
    int32_t error_pile = 0;

    error_pile += test_create_cache_and_destroy_cache();

    // cache_type cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
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

    // // cache1 = create_cache(CACHE_SIZE, FIFO, NULL);
    // // error_pile += test_serialize(cache1);
    // // destroy_cache(cache1);

    // error_pile += compositional_testing(16, 20);

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