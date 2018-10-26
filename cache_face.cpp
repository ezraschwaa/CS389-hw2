#include "cache.hh"
#include "cache.h"


using true_key_type = key_type;

struct Cache::Impl {
};
Cache::Cache(Cache::index_type maxmem, evictor_type evictor, hash_func hasher) : pImpl_(reinterpret_cast<Impl*>(create_cache(maxmem, LRU, NULL))) {

}
Cache::~Cache() {
	auto cache = reinterpret_cast<cache_obj*>(pImpl_.get());
	destroy_cache(cache);
}

void Cache::set(Cache::key_type key, Cache::val_type val, Cache::index_type val_size) {
	auto cache = reinterpret_cast<cache_obj*>(pImpl_.get());
	true_key_type key_str = key.c_str();
	index_type size = key.size();
	cache_set(cache, key_str, size, val, val_size);
}

Cache::val_type Cache::get(key_type key, index_type& val_size) const {
	auto cache = reinterpret_cast<cache_obj*>(pImpl_.get());
	true_key_type key_str = key.c_str();
	index_type size = key.size();
	return cache_get(cache, key_str, size, &val_size);
}

void Cache::del(key_type key) {
	auto cache = reinterpret_cast<cache_obj*>(pImpl_.get());
	true_key_type key_str = key.c_str();
	index_type size = key.size();
	cache_delete(cache, key_str, size);
}

index_type Cache::space_used() const {
	auto cache = reinterpret_cast<cache_obj*>(pImpl_.get());
	return cache_space_used(cache);
}
