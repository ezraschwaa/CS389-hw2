# Computer Systems Homework 3 (by Alyssa Riceman and Monica Moniot)

All three of the implementations we tested returned errors on attempting to set the cache, after constructing a cache with null inputs to hasher and evictor; apparently none of the implementations specifically handled the null-input edge case for those.

Ezra's implementation printed a bunch of random things as it ran (intended, we think, to alert users when they overwrite values, try to retrieve values which weren't set, or try to delete values which are already gone), but returned no errors and presented no difficulties with linking, with the following table of test results:

Function name                                    |Result
---                                              |---
test_create_cache                                |PASS
test_cache_set_and_get_one_val                   |PASS
test_cache_set_and_get_two_vals_same_key         |PASS
test_cache_set_and_get_two_vals_diff_keys        |PASS
test_cache_get_unassignedval                     |PASS
test_cache_delete_one_val                        |PASS
test_cache_delete_two_vals_same_key              |PASS
test_cache_delete_two_vals_diff_keys             |PASS
test_cache_delete_unusedval                      |PASS
test_cache_space_used_on_initialization          |PASS
test_cache_space_used_one_val                    |PASS
test_cache_space_used_two_vals_same_key          |PASS
test_cache_space_used_two_vals_diff_keys         |PASS
test_cache_space_used_deletion_one_val           |PASS
test_cache_space_used_deletion_two_vals_diff_keys|PASS

Josh's implementation was easy to link and returned no errors, with the following table of test results:

Function name                                    |Result
---                                              |---
test_create_cache                                |PASS
test_cache_set_and_get_one_val                   |PASS
test_cache_set_and_get_two_vals_same_key         |PASS
test_cache_set_and_get_two_vals_diff_keys        |PASS
test_cache_get_unassignedval                     |PASS
test_cache_delete_one_val                        |PASS
test_cache_delete_two_vals_same_key              |PASS
test_cache_delete_two_vals_diff_keys             |PASS
test_cache_delete_unusedval                      |PASS
test_cache_space_used_on_initialization          |PASS
test_cache_space_used_one_val                    |PASS
test_cache_space_used_two_vals_same_key          |PASS
test_cache_space_used_two_vals_diff_keys         |PASS
test_cache_space_used_deletion_one_val           |PASS
test_cache_space_used_deletion_two_vals_diff_keys|PASS

And Robert's implementation was similarly easy-to-link and error-free, also with this table:

Function name                                    |Result
---                                              |---
test_create_cache                                |PASS
test_cache_set_and_get_one_val                   |PASS
test_cache_set_and_get_two_vals_same_key         |PASS
test_cache_set_and_get_two_vals_diff_keys        |PASS
test_cache_get_unassignedval                     |PASS
test_cache_delete_one_val                        |PASS
test_cache_delete_two_vals_same_key              |PASS
test_cache_delete_two_vals_diff_keys             |PASS
test_cache_delete_unusedval                      |PASS
test_cache_space_used_on_initialization          |PASS
test_cache_space_used_one_val                    |PASS
test_cache_space_used_two_vals_same_key          |PASS
test_cache_space_used_two_vals_diff_keys         |PASS
test_cache_space_used_deletion_one_val           |PASS
test_cache_space_used_deletion_two_vals_diff_keys|PASS