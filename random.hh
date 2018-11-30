
#include <inttypes.h>

struct pcg_state_setseq_64 {    // Internals are *Private*.
    uint64_t state;             // RNG state.  All values are possible.
    uint64_t inc;               // Controls which RNG sequence (stream) is
                                // selected. Must *always* be odd.
};
using PCG = pcg_state_setseq_64;

// If you *must* statically initialize it, here's one.

constexpr PCG PCG_INITIALIZER = { 0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL };


PCG pcg_global = PCG_INITIALIZER;


// pcg_random32()
// pcg_random32(rng)
//     Generate a uniformly distributed 32-bit random number

uint32_t pcg_random32(PCG* rng = &pcg_global) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

//     Generate a uniformly distributed 64-bit random number
inline uint64_t pcg_random64(PCG* rng = &pcg_global) {
	uint64_t r0 = pcg_random32(rng);
	uint64_t r1 = pcg_random32(rng);
	return (r1<<32)|r0;
}

// pcg_seed(initstate, initseq)
// pcg_seed(rng, initstate, initseq):
//     Seed the rng.  Specified in two parts, state initializer and a
//     sequence selection constant (a.k.a. stream id)

void pcg_seed(PCG* rng, uint64_t initstate, uint64_t initseq = 0) {
    rng->state = 0U;
    rng->inc = (initseq << 1u) | 1u;
    pcg_random32(rng);
    rng->state += initstate;
    pcg_random32(rng);
}

inline void pcg_seed(uint64_t seed, uint64_t seq = 0) {
    pcg_seed(&pcg_global, seed, seq);
}

// pcg_random_bound(bound):
// pcg_random_bound(rng, bound):
//     Generate a uniformly distributed number, r, where lower <= r <= upper

inline uint32_t pcg_random_bound(PCG* rng, uint32_t lower, uint32_t upper) {
	uint32_t bound = upper - lower + 1;
    // To avoid bias, we need to make the range of the RNG a multiple of
    // bound, which we do by dropping output less than a threshold.
    // A naive scheme to calculate the threshold would be to do
    //
    //     uint32_t threshold = 0x100000000ull % bound;
    //
    // but 64-bit div/mod is slower than 32-bit div/mod (especially on
    // 32-bit platforms).  In essence, we do
    //
    //     uint32_t threshold = (0x100000000ull-bound) % bound;
    //
    // because this version will calculate the same modulus, but the LHS
    // value is less than 2^32.

    uint32_t threshold = -bound % bound;

    // Uniformity guarantees that this loop will terminate.  In practice, it
    // should usually terminate quickly; on average (assuming all bounds are
    // equally likely), 82.25% of the time, we can expect it to require just
    // one iteration.  In the worst case, someone passes a bound of 2^31 + 1
    // (i.e., 2147483649), which invalidates almost 50% of the range.  In
    // practice, bounds are typically small and only a tiny amount of the range
    // is eliminated.
    for (;;) {
        uint32_t r = pcg_random32(rng);
        if (r >= threshold)
            return (r % bound) + lower;
    }
}

inline uint32_t pcg_random_bound(uint32_t lower, uint32_t upper) {
    return pcg_random_bound(&pcg_global, lower, upper);
}

// pcg32_boundedrand(bound):
// pcg32_boundedrand_r(rng, bound):
//     Generate a uniformly distributed number, r, where 0 <= r < 1
inline double pcg_random_uniform(PCG* rng = &pcg_global) {
	uint32_t r = pcg_random32(rng);
	return ((double)r)/(((double)~0) + 1);
}
//     Generate a uniformly distributed number, r, where 0 <= r <= 1
inline double pcg_random_uniform_in(PCG* rng = &pcg_global) {
	uint32_t r = pcg_random32(rng);
	return ((double)r)/((double)~0);
}
//     Generate a uniformly distributed number, r, where 0 < r < 1
inline double pcg_random_uniform_ex(PCG* rng = &pcg_global) {
	uint32_t r = pcg_random32(rng);
	return (((double)r) + 1)/(((double)~0) + 2);
}
