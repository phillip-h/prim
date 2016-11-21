#ifndef __PRIM__
#define __PRIM__

#include <stdint.h>
#include <stdlib.h>

#define PRIM_VERSION "0.1"

void set_prim_error_callback(void (*callback)(char*));

uint64_t* prime_sieve(uint64_t max, size_t *len);

uint64_t* atkin(uint64_t max, size_t *len);
uint64_t* eratosthenes(uint64_t max, size_t *len);

#endif
