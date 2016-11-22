#ifndef __PRIM__
#define __PRIM__

#include <stdint.h>
#include <stdlib.h>

/***************************
 * _prim_ public interface *
 ***************************/

/* current library version */
#define PRIM_VERSION "0.1"

/*
 * High-level prime sieve. Sieves all primes in [1, _max_] into a new
 * _uint64_t*_, which is returned. The length of the resulting array is
 * stored in _len_.
 *
 * May return _NULL_ or a _uint64_t*_ of garbage on failure, depending
 * on if the failure occurred before or after invoking _segmented_sieve()_.
 * In any failure _len_ will be zero.
 */
uint64_t* prime_sieve(uint64_t max, size_t *len);

/*
 * Sieves all primes in [1, _max_] using the Sieve of Atkin into a new
 * _uint64_t*_, which is returned. The length of the resulting array is
 * stored in _len_.
 *
 * Returns _NULL_ and zero _len_ on failure.
 */
uint64_t* atkin(uint64_t max, size_t *len);

/*
 * Sieves all primes in [1, _max_] using the Sieve of Eratosthenes into a new
 * _uint64_t*_, which is returned. The length of the resulting array is
 * stored in _len_.
 *
 * Returns _NULL_ and zero _len_ on failure.
 */
uint64_t* eratosthenes(uint64_t max, size_t *len);

/*
 * Segmented version of the Sieve of Eratosthenes. Sieves all primes in
 * [1, _max_] and passes each one to the _callback_ function. The callback is a
 * function that takes a single _uint64_t_ and returns void.
 *
 * Does not do anything except stop executing on failure.
 */
void segmented_sieve(uint64_t max, void(*callback)(uint64_t));

/*
 * Set the callback function used for errors. This must be a 
 * function that takes a single argument of _char*_ (representing the
 * error string), and returns void.
 */
void set_prim_error_callback(void (*callback)(char*));

#endif
