#include "prim.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

/* size of a segmented_sieve() segment */
#define S_SIEVE_SIZE 65536

/* bitset struct used to reduce sieve memory usage */
typedef struct bitset {
    size_t len;
    uint8_t *data;
} bitset;

/* internal function */
size_t primes_below(uint64_t n);
void* palloc(size_t len);

bitset* bitset_new(size_t size);
void bitset_zero(bitset *bits);
void bitset_one(bitset *bits);
void bitset_flip(bitset *bits, size_t pos);
void bitset_set(bitset *bits, size_t pos, uint8_t val);
int bitset_read(bitset *bits, size_t pos);
void bitset_free(bitset *bits);

/* error callback and setter function */
void (*error_callback)(char*) = NULL;
void set_prim_error_callback(void (*callback)(char*)) {
    error_callback = callback;
}

/* 
 * internal state for using segmented_sieve() in 
 * prime_sieve()
 */
uint64_t* PRIM_INTERNAL_PRIME_BUF = NULL;
size_t PRIM_INTERNAL_PRIME_POS = 0;
void PRIM_SIEVE_CALLBACK(uint64_t prime) {
    PRIM_INTERNAL_PRIME_BUF[PRIM_INTERNAL_PRIME_POS++] = prime;
}

/*
 * If _max_ is less than the segment size, sieve using the 
 * Sieve of Atkin, otherwise sieve using the segmented sieve and 
 * internal buffers.
 */
uint64_t* prime_sieve(uint64_t max, size_t *len) {
    if (max < S_SIEVE_SIZE) {
        return atkin(max, len);
    }

    /* 
     * prime_sieve() is evoked recursively, so 
     * these must be stored to avoid clobbering.
     */
    uint64_t *backup_buf = PRIM_INTERNAL_PRIME_BUF;
    size_t backup_pos = PRIM_INTERNAL_PRIME_POS;

    /* run the segmented sieve */
    PRIM_INTERNAL_PRIME_BUF = palloc(primes_below(max) * sizeof(uint64_t));
    PRIM_INTERNAL_PRIME_POS = 0;
    segmented_sieve(max, &PRIM_SIEVE_CALLBACK);

    /* swap state back to how it was at the start of this routine */
    uint64_t *primes = PRIM_INTERNAL_PRIME_BUF;
    *len = PRIM_INTERNAL_PRIME_POS;
    PRIM_INTERNAL_PRIME_BUF = backup_buf;
    PRIM_INTERNAL_PRIME_POS = backup_pos;

    return primes;
}


/*
 * Modulo-sixty tests for the Sieve of Atkin 
 */

/* 1, 13, 17, 29, 37, 41, 49, 53 */
const uint8_t atkin_test_one[60] = {
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0
};

/* 7, 19, 31, 43 */
const uint8_t atkin_test_two[60] = {
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* 11, 23, 47, 59 */
const uint8_t atkin_test_three[60] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1
};

/* 
 * small primes for the Sieve of Atkin, which will only
 * find primes greater than seven.
 */
const uint64_t small_primes[3] = { 2, 3, 5 };

/*
 * Sieves all primes in [1, _max_] using the Sieve of Atkin into a new
 * _uint64_t*_, which is returned. The length of the resulting array is
 * stored in _len_.
 */
uint64_t* atkin(uint64_t max, size_t *len) {
    /* find how large the resulting array will be */
    if (max < 2) {
        *len = 0;
    } else if (max < 3) {
        *len = 1;
    } else if (max < 5) {
        *len = 2;
    } else if (max < 6) {
        *len = 3;
    } else {
        *len = primes_below(max);
    }

    /* create primes array */
    uint64_t *primes = palloc(*len * sizeof(uint64_t));
    if (!primes) {
        *len = 0;
        return NULL;
    }

    /* copy in the small primes the sieve will not hit */
    memcpy(primes, small_primes, (*len < 4 ? *len : 3) * sizeof(uint64_t));
    if (max < 6) {
        return primes;
    }

    /* construct sieve */
    bitset *sieve = bitset_new(max);
    if (!sieve) {
        free(primes);
        *len = 0;
        return NULL;
    }
    bitset_zero(sieve);

    /* run the wheel */
    size_t limit = sqrt((double) max) + 1;
    size_t index = 0;
    for (size_t x = 1; x <= limit; ++x) {
        for (size_t y = 1; y <= limit; ++y) {
            index = 4 * x * x + y * y;
            if (index <= max && atkin_test_one[index % 60]) {
                bitset_flip(sieve, index);
            }

            index = 3 * x * x + y * y;
            if (index <= max && atkin_test_two[index % 60]) {
                bitset_flip(sieve, index);
            }
          
            if (x <= y) {
                continue;
            }

            index = 3 * x * x - y * y;
            if (index <= max && atkin_test_three[index % 60]) {
                bitset_flip(sieve, index);
            }
        }
    }

    /* remove squares of primes and their multiples */
    uint64_t val = 0;
    for (size_t i = 7; i <= limit; ++i) {
        if (bitset_read(sieve, i)) {
            val = i * i;
            size_t k = val;
            while (k <= max) {
                bitset_set(sieve, k, 0);
                k += val;
            }
        }
    }

    /* accumulate results from the sieve into the array */
    index = 3;
    for (size_t i = 0; i <= max; ++i) {
        if (bitset_read(sieve, i)) {
            primes[index] = i;
            index++;
        }
    }

    *len = index;
    bitset_free(sieve);
    return primes;
}

/*
 * Sieves all primes in [1, _max_] using the Sieve of Eratosthenes into a new
 * _uint64_t*_, which is returned. The length of the resulting array is
 * stored in _len_.
 */
uint64_t* eratosthenes(uint64_t max, size_t *len) {
    /* if the sieve will not hit anything, just return */
    if (max < 2) {
        *len = 0;
        return palloc(0);
    }

    /* construct the sieve */
    bitset *sieve = bitset_new(max + 1);
    if (!sieve) {
        *len = 0;
        return NULL;
    }
    bitset_one(sieve);

    /* construct the results array */
    uint64_t *primes = palloc(primes_below(max + 1) * sizeof(uint64_t));
    if (!primes) {
        *len = 0;
        goto end;
    }

    /* 
     * two is the only even prime,
     * so the algorithm saves time by only checking 
     * odd numbers. However it does mean we must add
     * two manually.
     */
    primes[0] = 2;

    /* index in the results array */
    size_t ind = 1;

    size_t pos = 3;
    while (pos <= max) {
        if (bitset_read(sieve, pos)) {
            /* _pos_ is a prime, add it to the results array */
            primes[ind] = pos;
            ind += 1;

            /* mark multiples of this prime as composite */
            for (size_t j = pos * 2; j <= max; j += pos) {
                bitset_set(sieve, j, 0);
            }
        }
        pos += 2;
    }

    *len = ind;

    /* label for cases of failed allocation */
end:
    bitset_free(sieve);
    return primes;
}

/*
 * Segmented version of the Sieve of Eratosthenes. Sieves all primes in
 * [1, _max_] and passes each one to the _callback_ function. The callback is a
 * function that takes a single _uint64_t_ and returns void.
 */
void segmented_sieve(uint64_t max, void(*callback)(uint64_t)) {
    /* if the sieve will not hit anything, just return */
    if (max < 2) {
        return;
    }

    /* compute an array of small primes used for sieving */
    size_t limit = sqrt((double) max) + 1;
    size_t len_sp = 0;
    uint64_t *small_primes = prime_sieve(limit, &len_sp);

    /* 
     * construct an array of offsets and a value to keep track of
     * how many of the small primes are currently able to sieve
     * (less than the square root of maximum value 
     *  of the current segment).
     */
    uint64_t *offsets = palloc(limit * sizeof(uint64_t));
    size_t num_sieve = 0;

    /* construct the sieve */
    bitset *sieve = bitset_new(S_SIEVE_SIZE + 1);


    /* the current small prime and the current "candidate" prime */
    uint64_t small = 2;
    uint64_t candidate = 3; 

    /* 
     * two is the only even prime,
     * so the algorithm saves time by only checking 
     * odd numbers. However it does mean we must run
     * the callback for two manually.
     */
    (*callback)(2);

    /* calculate sieve end */
    uint64_t end = ceil(((double) max / S_SIEVE_SIZE)) * S_SIEVE_SIZE;
    for (size_t pos = 0; pos < end; pos += S_SIEVE_SIZE) {
        /* mark all numbers as prime initially */
        bitset_one(sieve);

        /* calculate the upper boundary */
        uint64_t pos_h = pos + S_SIEVE_SIZE - 1;
        if (pos_h > max) pos_h = max;

        /* update current small primes */
        while (small * small <= pos_h) {
            for (size_t i = num_sieve; i < len_sp; ++i) {
                if (small_primes[i] == small) {
                    offsets[num_sieve] = small * small - pos;
                    ++num_sieve;
                } else if (small_primes[i] > small) {
                    /* 
                     * the list is sorted, so (small * small) is not
                     * in the list.
                     */
                    break;
                }
            }

            ++small;
        }

        /* sieve this segment */
        for (size_t i = 1; i < num_sieve; ++i) {
            uint64_t j = offsets[i];
            uint64_t k = small_primes[i] * 2;

            /* mark off all composites */
            while (j < S_SIEVE_SIZE) {
                bitset_set(sieve, j, 0);
                j += k;
            }
            offsets[i] = j - S_SIEVE_SIZE;
        }

        /* call the callback for each prime */
        while (candidate <= pos_h) {
            if (bitset_read(sieve, candidate - pos)) {
                (*callback)(candidate);
            }

            candidate += 2;
        }

    }

    /* cleanup */
    bitset_free(sieve);
    free(small_primes);
    free(offsets);
}

/*
 * Internal Functions
 */

/* return an upper bound on the number primes in [1, n] */
size_t primes_below(uint64_t n) {
    return (size_t) ceil(1.25506 * (double) n / log(n));
}

/* 
 * try to allocate _len_ bytes, calling the error callback if
 * the allocation fails (and the callback function has been defined.
 */
void* palloc(size_t len) {
    void* block = malloc(len);
    if (!block && error_callback) {
        char error[128];
        snprintf(error, 128, "Failed to allocate %zu bytes -- %s", len, strerror(errno));
        (*error_callback)(error);
    }

    return block;
}

/*
 * bitset functions
 */

bitset* bitset_new(size_t size) {
    bitset *bits = palloc(sizeof(*bits));
    if (!bits) return NULL;

    bits->len = (size + 8 - (size % 8)) / 8 + 1;
    bits->data = palloc(bits->len * sizeof(uint8_t));
    if (!bits->data) {
        bitset_free(bits);
        return NULL;
    }

    return bits;
}

void bitset_one(bitset *bits) {
    memset(bits->data, 0xff, bits->len);
}

void bitset_zero(bitset *bits) {
    memset(bits->data, 0x00, bits->len);
}

void bitset_flip(bitset *bits, size_t pos) {
    bits->data[pos / 8] ^= 0x01 << (pos % 8);
}

void bitset_set(bitset *bits, size_t pos, uint8_t val) {
    /* zero the target bit */
    bits->data[pos / 8] &= ~(0x01 << (pos % 8));
    /* set the target bit */
    bits->data[pos / 8] |= val << (pos % 8);
}

int bitset_read(bitset *bits, size_t pos) {
    return bits->data[pos / 8] & (0x01 << (pos % 8));
}

void bitset_free(bitset *bits) {
    free(bits->data);
    free(bits);
}
