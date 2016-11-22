#include <stdio.h>

#include "prim.h"

/*
 * Unit Tests
 */

#define t_arrays(statement, expected, expected_len) do { \
    primes = statement; \
    char *cmp = cmp_arrays(primes, expected, len, expected_len); \
    if (cmp) { \
        printf("\n\x1b[1;31mTest failed\x1b[0m -- "); \
        p_array(primes, len); \
        printf(" != "); \
        p_array(expected, expected_len); \
        printf(":\n - %s\n", cmp); \
        fails++; \
    } else { \
        ++passed; \
    } \
\
    ++tests; \
    free(primes); \
\
    printf("."); \
    fflush(stdout); \
} while(0);

#define t_len(statement, expected) do { \
    primes = statement; \
    if (len != expected) { \
        printf("\n\x1b[1;31mTest failed\x1b[0m -- "); \
        printf("length of %zu != length of %zu!\n", len, (size_t)expected); \
        fails++; \
    } else { \
        ++passed; \
    } \
\
    ++tests; \
    free(primes); \
\
    printf("."); \
    fflush(stdout); \
} while(0);

unsigned tests = 0;
unsigned passed = 0;
unsigned fails = 0;

char *cmp_arrays(uint64_t *a, uint64_t *b, size_t a_len, size_t b_len);
void p_array(uint64_t *data, size_t len);

void callback(char *str) {
    printf("%s\n", str);
}

void new_section(const char *name) {
    fails = 0;
    printf("Running %s tests ", name);
    fflush(stdout);
}

void end_section(void) {
    if (!fails) {
        printf(" \x1b[1;32mOK\x1b[0m\n");
    }
}

int main() {
    set_prim_error_callback(&callback);

    uint64_t two[] = {2};
    uint64_t three[] = {2, 3};
    uint64_t five[] = {2, 3, 5};
    uint64_t ten[] = {2, 3, 5, 7};

    uint64_t *primes = NULL;
    size_t len = 0;

    new_section("eratosthenes");

    t_arrays(eratosthenes(2, &len), two, 1);
    t_arrays(eratosthenes(3, &len), three, 2);
    t_arrays(eratosthenes(5, &len), five, 3);
    t_arrays(eratosthenes(10, &len), ten, 4);
    t_len(eratosthenes(100, &len), 25);
    t_len(eratosthenes(1000, &len), 168);
    t_len(eratosthenes(1000000, &len), 78498);

    end_section();

    new_section("atkin");

    t_arrays(atkin(2, &len), two, 1);
    t_arrays(atkin(3, &len), three, 2);
    t_arrays(atkin(5, &len), five, 3);
    t_arrays(atkin(10, &len), ten, 4);
    t_len(atkin(100, &len), 25);
    t_len(atkin(1000, &len), 168);
    t_len(atkin(1000000, &len), 78498);

    end_section();

    new_section("segmented sieve");

    t_arrays(prime_sieve(2, &len), two, 1);
    t_arrays(prime_sieve(3, &len), three, 2);
    t_arrays(prime_sieve(5, &len), five, 3);
    t_arrays(prime_sieve(10, &len), ten, 4);
    t_len(prime_sieve(100, &len), 25);
    t_len(prime_sieve(1000, &len), 168);
    t_len(prime_sieve(1000000, &len), 78498);
    t_len(prime_sieve(10000000, &len), 664579);

    end_section();

    printf("\nRan %d tests\n", tests);
    if (passed == tests) {
        printf("\x1b[1;32mAll Tests Passed!\x1b[0m\n");
    } else {
        printf("%d failures, %d passes\n", tests - passed, passed);
    }

    return passed == tests ? 0 : 1;
}

char *cmp_arrays(uint64_t *a, uint64_t *b, size_t a_len, size_t b_len) {
    if (a_len != b_len) {
        return "Array lengths are not equal!";
    }

    for (size_t i = 0; i < a_len; ++i)
        if (a[i] != b[i])
            return "Array elements not equal!";

    return NULL;
}

void p_array(uint64_t *data, size_t len) {
    printf("[");
    for (size_t i = 0; i < len; ++i) {
        if (i != 0) printf(", ");
        printf("%zu", data[i]);
    }
    printf("]");
}
