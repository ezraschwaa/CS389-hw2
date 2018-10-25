# Computer Systems Homework 2 (by Alyssa Riceman and Monica Moniot)

## Cache

In order to find the correct entry based on the key in the cache set and get functions, we implemented a hash table. We take the hash of the key using the given hash function (or, if none is given, the default), and we use it to choose which index in our hash table to put the entry in.

To keep the load factor down, every time we add an entry, we check if the new entry total exceeds half of the space in the table, and if it does, we resize the entire table by allocating new memory and rehashing all of the old values into the new memory.

To resolve collisions, we implemented double hashing, taking the hash of the key and using that to create a step size with which to traverse the table. Every time we find a collision, we go [step size] away from that collision and attempt to use the new location instead. If we keep the table's load factor down, this should be a constant-time operation.

To ensure this is as fast as possible, instead of comparing the input key with all of the already-stored keys, we compare the keys' hashes. If they don't match, we know that the entry we're looking at isn't the one to which the key belongs. If they *do* match, we *then* check the keys against each other to ensure that they're actually the same. If so, we've found the entry we need to modify; otherwise, we keep searching. We do all of this to ensure that we only make one memory access per collision and to increase the locality of our hash table (since we only need to store four-byte-long hashes rather than anything bigger). We have a second table parallel to the hash table such that, for each index in the hash table, the second table stores a relative pointer to the data associated with that index's entry.

Because we use relative pointers and all of the dynamic memory is a joint allocation, the caches are easy to serialize simply by copying memory, and we added functionality to that effect. However, for reasons unclear to us, valgrind throws errors when trying to allocate space for the serialization, although no actual memory errors or leaks take place.

We manage the memory of our entries in a data structure called a Book. Book is a memory allocator which allocates each entry as a fixed-sized page, and returns a relative pointer to that page. We can free this memory, and it will be reused by the allocator. We return a relative pointer so that, even when we resize the cache and move it in memory, the relative pointer will still be valid (because it's relative to the beginning of the Book's page table).

In order to implement the eviction policies, we have the user pass in an enum specifying the specific eviction policy they want to use. This policy is stored, and then whenever the cache needs to update the evictor it will pass the policy to the evictor, which will use the policy to determine the correct operation to perform.

Since different eviction policies want to use memory differently, but only one is applicable for any given cache, we define a data structure which is a union of all the different data fields that each eviction policy would want to store. Given the policy, the evictor can determine which part of the union it should use.

Neither Books nor the eviction policies manage their own memory; both are managed by the cache itself.

## Testing

For testing, we first execute a series of unit tests designed to ensure basic functionality of the cache: we test that each of the functions in header.h can be run without error and produce the results we would expect, and then perform some more specific tests: a test to ensure that user-input hashers work correctly, that both the FIFO and LRU eviction policies work correctly, and that the cache's automatic resizing works correctly.

Following this, we then run a compositional testing function which will, for a given number of iterations, create a cache and then run it through a given number of tests in a random order. This ensures that the cache functionality is robust given usage of all of its functionality in a random order, rather than only when each individual part is executed separately from the others.