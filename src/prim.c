#include "prim.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define S_SIEVE_SIZE 65536

typedef struct bitset {
    size_t len;
    uint8_t *data;
} bitset;

size_t primes_below(uint64_t n);
void* palloc(size_t len);

bitset* bitset_new(size_t size);
void bitset_zero(bitset *bits);
void bitset_one(bitset *bits);
void bitset_flip(bitset *bits, size_t pos);
void bitset_set(bitset *bits, size_t pos, uint8_t val);
int bitset_read(bitset *bits, size_t pos);
void bitset_free(bitset *bits);

void (*error_callback)(char*) = NULL;
void set_prim_error_callback(void (*callback)(char*)) {
    error_callback = callback;
}

uint64_t* PRIM_INTERNAL_PRIME_BUF = NULL;
size_t PRIM_INTERNAL_PRIME_POS = 0;
void PRIM_SIEVE_CALLBACK(uint64_t prime) {
    PRIM_INTERNAL_PRIME_BUF[PRIM_INTERNAL_PRIME_POS++] = prime;
}
uint64_t* prime_sieve(uint64_t max, size_t *len) {
    if (max < S_SIEVE_SIZE) {
        return atkin(max, len);
    }

    uint64_t *backup_buf = PRIM_INTERNAL_PRIME_BUF;
    size_t backup_pos = PRIM_INTERNAL_PRIME_POS;

    PRIM_INTERNAL_PRIME_BUF = palloc(primes_below(max) * sizeof(uint64_t));
    PRIM_INTERNAL_PRIME_POS = 0;
    segmented_sieve(max, &PRIM_SIEVE_CALLBACK);

    uint64_t *primes = PRIM_INTERNAL_PRIME_BUF;
    *len = PRIM_INTERNAL_PRIME_POS;
    PRIM_INTERNAL_PRIME_BUF = backup_buf;
    PRIM_INTERNAL_PRIME_POS = backup_pos;
    return primes;
}

const uint8_t atkin_test_one[60] = {
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0
};

const uint8_t atkin_test_two[60] = {
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const uint8_t atkin_test_three[60] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1
};

const uint64_t small_primes[3] = { 2, 3, 5 };

uint64_t* atkin(uint64_t max, size_t *len) {
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

    uint64_t *primes = palloc(*len * sizeof(uint64_t));
    if (!primes) {
        *len = 0;
        return NULL;
    }

    memcpy(primes, small_primes, (*len < 4 ? *len : 3) * sizeof(uint64_t));
    if (max < 6) {
        return primes;
    }

    size_t limit = sqrt((double) max) + 1;
    bitset *sieve = bitset_new(max);
    if (!sieve) {
        free(primes);
        *len = 0;
        return NULL;
    }
    bitset_zero(sieve);

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

uint64_t* eratosthenes(uint64_t max, size_t *len) {
    if (max < 2) {
        *len = 0;
        return palloc(0);
    }

    bitset *sieve = bitset_new(max + 1);
    if (!sieve) {
        *len = 0;
        return NULL;
    }
    bitset_one(sieve);

    uint64_t *primes = palloc(primes_below(max + 1) * sizeof(uint64_t));
    if (!primes) {
        *len = 0;
        goto end;
    }

    primes[0] = 2;
    size_t ind = 1;

    size_t pos = 3;
    while (pos <= max) {
        if (bitset_read(sieve, pos)) {
            primes[ind] = pos;
            ind += 1;

            for (size_t j = pos * 2; j <= max; j += pos) {
                bitset_set(sieve, j, 0);
            }
        }
        pos += 2;
    }

    *len = ind;

end:
    bitset_free(sieve);
    return primes;
}

void segmented_sieve(uint64_t max, void(*callback)(uint64_t)) {
    size_t limit = sqrt((double) max) + 1;
    size_t len_sp = 0;
    uint64_t *small_primes = prime_sieve(limit, &len_sp);

    bitset *sieve = bitset_new(S_SIEVE_SIZE + 1);

    uint64_t *sieve_primes = palloc(limit * sizeof(uint64_t));
    uint64_t *offsets = palloc(limit * sizeof(uint64_t));
    size_t num_sieve = 0;

    uint64_t small = 2;
    uint64_t candidate = 3; 

    (*callback)(2);

    // calculate sieve end condition
    uint64_t end = ceil(((double) max / S_SIEVE_SIZE)) * S_SIEVE_SIZE;
    for (size_t pos = 0; pos < end; pos += S_SIEVE_SIZE) {
        bitset_one(sieve);

        // calculate the upper boundary
        uint64_t pos_h = pos + S_SIEVE_SIZE - 1;
        if (pos_h > max) pos_h = max;

        // add any new small primes to the sieve vec
        while (small * small <= pos_h) {
            for (size_t i = num_sieve; i < len_sp; ++i) {
                if (small_primes[i] == small) {
                    sieve_primes[num_sieve] = small;
                    offsets[num_sieve] = small * small - pos;
                    ++num_sieve;
                } else if (small_primes[i] > small) {
                    break;
                }
            }

            ++small;
        }

        // preform the sieve
        for (size_t i = 1; i < num_sieve; ++i) {
            uint64_t j = offsets[i];
            uint64_t k = sieve_primes[i] * 2;

            while (j < S_SIEVE_SIZE) {
                bitset_set(sieve, j, 0);
                j += k;
            }
            offsets[i] = j - S_SIEVE_SIZE;
        }

        // collect primes, call the callback expression
        while (candidate <= pos_h) {
            if (bitset_read(sieve, candidate - pos)) {
                (*callback)(candidate);
            }

            candidate += 2;
        }

    }
}

/*
 * Internal Functions
 */

size_t primes_below(uint64_t n) {
    return (size_t) ceil(1.25506 * (double) n / log(n));
}

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
    bits->data[pos / 8] &= ~(0x01 << (pos % 8));
    bits->data[pos / 8] |= val << (pos % 8);
}

int bitset_read(bitset *bits, size_t pos) {
    return bits->data[pos / 8] & (0x01 << (pos % 8));
}

void bitset_free(bitset *bits) {
    free(bits->data);
    free(bits);
}
