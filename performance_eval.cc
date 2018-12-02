#include <iostream>
#include <cmath>
#include <string>
#include <cstdlib>
#include <ctime>
#include <string.h>
#include <chrono>
#include <algorithm>
#include <cstdlib>

#include "cache.h"


using namespace std; 
using namespace std::chrono; 

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
const index_type LARGEVAL_SIZE = 30;
index_type valsize1_for_get = 0;
index_type valsize2_for_get = 0;

int TRIALS = 1000000;

////////////////////
// Test Functions //
////////////////////

// Test to ensure that cache creation and destruction work with different cache sizes and presence or absence of user-input hash functions, to ensure that any errors which may arise from doing so arise
int test_create_cache() {
    index_type sizeArray[] = {SMALL_CACHE_SIZE, CACHE_SIZE, LARGE_CACHE_SIZE};

    for(int i = 0; i < 3; i++) {
        int count = 0;
        float timer = 0;
        while (count < TRIALS){
            auto start = high_resolution_clock::now();
            cache_type cache = create_cache(sizeArray[i], NULL);
            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<nanoseconds>(stop - start);

            timer += duration.count();
            ++count;
            destroy_cache(cache);
        }
        timer = (timer / TRIALS);
        cout << "\tAverage time to create a cache of size "<< sizeArray[i] << ": " << timer << " nanoseconds" << endl;
    }
    return 0;
}

int test_cache_set(cache_type cache) {
    const char* values[] = {SMALLVAL, LARGEVAL};
    index_type valSize[] = {SMALLVAL_SIZE, LARGEVAL_SIZE};

    index_type sizeArray[] = {SMALL_CACHE_SIZE, CACHE_SIZE, LARGE_CACHE_SIZE};
    cache_type cacheArray[] = {create_cache(SMALL_CACHE_SIZE, NULL), create_cache(CACHE_SIZE, NULL), create_cache(LARGE_CACHE_SIZE, NULL)};

    for(int i = 0; i < 2; i++) {
        for(int j=0;j< 3;j++) {
            int count = 0;
            float timer = 0;
            while (count < TRIALS){
                auto start = high_resolution_clock::now();
                cache_set(cacheArray[j], KEY1, values[i], valSize[i]);
                auto stop = high_resolution_clock::now();
                auto duration = duration_cast<nanoseconds>(stop - start);

                timer += duration.count();
                ++count;
                cache_delete(cacheArray[j], KEY1);
            }
            timer = (timer / TRIALS);
            cout << "\tAVG Time to set "<< valSize[i] << " sized val in " << sizeArray[j] << " sized cache: " << timer << " nanoseconds" << endl;
        }
    }
    free(values);
    return 0;
}
int test_cache_delete(cache_type cache) {
    const char* values[] = {SMALLVAL, LARGEVAL};
    index_type valSize[] = {SMALLVAL_SIZE, LARGEVAL_SIZE};

    index_type sizeArray[] = {SMALL_CACHE_SIZE, CACHE_SIZE, LARGE_CACHE_SIZE};
    cache_type cacheArray[] = {create_cache(SMALL_CACHE_SIZE, NULL), create_cache(CACHE_SIZE, NULL), create_cache(LARGE_CACHE_SIZE, NULL)};

    for(int i = 0; i < 2; i++) {
        for(int j=0; j<3; j++) {
            int count = 0;
            float timer = 0;
            while (count < TRIALS){
                cache_set(cacheArray[j], KEY1, values[i], valSize[i]);
                auto start = high_resolution_clock::now();
                cache_delete(cacheArray[j], KEY1);
                auto stop = high_resolution_clock::now();
                auto duration = duration_cast<nanoseconds>(stop - start);

                timer += duration.count();
                ++count;
            }
            timer = (timer / TRIALS);
            cout << "\tAVG Time to delete "<< valSize[i] << " sized val in " << sizeArray[j] << " sized cache: " << timer << " nanoseconds" << endl;        
        }
    }
    int count = 0;
    float timer = 0;
    free(values);

    return 0;
}
int test_cache_get() {
    index_type sizeArray[] = {SMALL_CACHE_SIZE, CACHE_SIZE, LARGE_CACHE_SIZE};
    cache_type cacheArray[] = {create_cache(SMALL_CACHE_SIZE, NULL), create_cache(CACHE_SIZE, NULL), create_cache(LARGE_CACHE_SIZE, NULL)};

    const char* values[] = {SMALLVAL, LARGEVAL};
    index_type valSize[] = {SMALLVAL_SIZE, LARGEVAL_SIZE};

    for(int i = 0; i < 3; i++) {
        int count = 0;
        float timer = 0;
        while (count < TRIALS){
            auto start = high_resolution_clock::now();
            cache_get(cacheArray[i], KEY1, &valsize1_for_get);
            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<nanoseconds>(stop - start);

            timer += duration.count();
            ++count;
        }
        timer = (timer / TRIALS);
        cout << "\tAverage time to get in empty "<< sizeArray[i] << " bit cache: " << timer << " nanoseconds" << endl;
    }

    //fill up caches halfway
    for(int j = 0; j < 3; j++){
        index_type size = sizeArray[j] / 2;
        cache_set(cacheArray[j], KEY1, make_str_of_defined_length(size), size);
    }
    for(int i = 0; i < 3; i++) {
        int count = 0;
        float timer = 0;
        while (count < TRIALS){
            auto start = high_resolution_clock::now();
            cache_get(cacheArray[i], KEY1, &valsize1_for_get);
            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<nanoseconds>(stop - start);

            timer += duration.count();
            ++count;
        }
        timer = (timer / TRIALS);
        cout << "\tAverage time to get in 1/2filled "<< sizeArray[i] << " cache: " << timer << " nanoseconds" << endl;
    }

    for(int j = 0; j < 3; j++){
        index_type size = sizeArray[j];
        cache_set(cacheArray[j], KEY1, make_str_of_defined_length(size), size);
    }

    for(int i = 0; i < 3; i++) {
        int count = 0;
        float timer = 0;
        while (count < TRIALS){
            auto start = high_resolution_clock::now();
            cache_get(cacheArray[i], KEY1, &valsize1_for_get);
            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<nanoseconds>(stop - start);

            timer += duration.count();
            ++count;
        }
        timer = (timer / TRIALS);
        cout << "\tAverage time for get of filled "<< sizeArray[i] << " cache: " << timer << " nanoseconds" << endl;
    }

    for(int j = 0; j < 2; j++){
        index_type size = sizeArray[j];
        for(int numOfValues = 0; numOfValues < size; numOfValues++){
            string numString = to_string(numOfValues);
            key_type numChar = numString.c_str();
            cache_set(cacheArray[j], numChar, make_str_of_defined_length(2), 1);
        }
    }

    // for(int i = 0; i < 2; i++) {
    //     int count = 0;
    //     float timer = 0;
    //     string randk;
    //     key_type randKEY;
    //     cout << "Currently on Cache size " << sizeArray[i] << '\n';
    //     while (count < TRIALS){
    //         randk = to_string(rand() % sizeArray[i]);
    //         randKEY = randk.c_str();
    //         cout << count << "\n";
    //         auto start = high_resolution_clock::now();
    //         cache_get(cacheArray[i], randKEY, &valsize1_for_get);

    //         auto stop = high_resolution_clock::now();
    //         auto duration = duration_cast<nanoseconds>(stop - start);

    //         timer += duration.count();
    //         ++count;
    //     }
    //     timer = (timer / TRIALS);
    //     cout << "\tAverage time for get of discrete filled "<< sizeArray[i] << " cache: " << timer << " nanoseconds" << endl;
    //     destroy_cache(cacheArray[i]);
    // }
    free(values);
    return 0;
}

int test_cache_space_used(){
    index_type sizeArray[] = {SMALL_CACHE_SIZE, CACHE_SIZE, LARGE_CACHE_SIZE};
    cache_type cacheArray[] = {create_cache(SMALL_CACHE_SIZE, NULL), create_cache(CACHE_SIZE, NULL), create_cache(LARGE_CACHE_SIZE, NULL)};
    
    index_type space;
    for(int i = 0; i < 3; i++) {
        int count = 0;
        float timer = 0;
        while (count < TRIALS){
            auto start = high_resolution_clock::now();
            space = cache_space_used(cacheArray[i]);
            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<nanoseconds>(stop - start);

            timer += duration.count();
            ++count;
        }
        timer = (timer / TRIALS);
        cout << "\tAverage time for space used of empty "<< sizeArray[i] << " bit cache: " << timer << " nanoseconds" << endl;
    }

    //fill up caches halfway
    for(int j = 0; j < 3; j++){
        index_type size = sizeArray[j] / 2;
        cache_set(cacheArray[j], KEY1, make_str_of_defined_length(size), size);
    }

    for(int i = 0; i < 3; i++) {
        int count = 0;
        float timer = 0;
        while (count < TRIALS){
            auto start = high_resolution_clock::now();
            space = cache_space_used(cacheArray[i]);
            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<nanoseconds>(stop - start);

            timer += duration.count();
            ++count;
        }
        timer = (timer / TRIALS);
        cout << "\tAverage time for space used of 1/2filled "<< sizeArray[i] << " cache: " << timer << " nanoseconds" << endl;
    }

    for(int j = 0; j < 3; j++){
        index_type size = sizeArray[j];
        cache_set(cacheArray[j], KEY1, make_str_of_defined_length(size), size);
    }

    for(int i = 0; i < 3; i++) {
        int count = 0;
        float timer = 0;
        while (count < TRIALS){
            auto start = high_resolution_clock::now();
            space = cache_space_used(cacheArray[i]);
            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<nanoseconds>(stop - start);

            timer += duration.count();
            ++count;
        }
        timer = (timer / TRIALS);
        cout << "\tAverage time for space used of filled "<< sizeArray[i] << " cache: " << timer << " nanoseconds" << endl;
    }

    return 0;
}






int 
main()
{
	cache_type cache1 = create_cache(CACHE_SIZE, NULL);

    //Times the tests itself.  Not great for measuring the actual metrics. 
	cout << "Testing creating cache: " << endl;
    test_create_cache();

	cout << "Testing set: " << endl;
	test_cache_set(cache1);

	cout << "Testing delete: " << endl;
	test_cache_delete(cache1);

	cout << "Testing check space_used: " << endl;
	test_cache_space_used();

    cout << "Testing get: " << endl;
    test_cache_get();

	destroy_cache(cache1);

    delete[] SMALLVAL;
    delete[] LARGEVAL;
}