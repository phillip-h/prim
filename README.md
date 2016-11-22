# prim

Lightweight primes Sieve in C

# about

`prim` is a lightweight library that contains an implementation of several
prime sieves. It also comes with a small example CLI tool that will print
primes to stdout.

# usage

Clone the repository and run `make`. Requires C99. Compiles with `-Ofast`
by default.

This should produce three files:

- `prim.a` -- the static library.

- `prim` -- the CLI tool, described more below.

- `prim-tests` -- unit test executable.

Running `prim-tests` will run a brief series of unit tests that verify that
the library is working properly.

## source layout

```
.
├── cli -- files for the CLI tool
├── src -- files for the actual library
└── test -- files for unit tests
```

## library interface

The makefile is setup to build a static library, but since there are only
two files for the library proper it's easy enough to just drop into your
project and use directly.

`src/prim.h` contains documentation for all library functions. An overview
of interface function signatures is given here.

```
uint64_t* prime_sieve(uint64_t max, size_t *len);
uint64_t* atkin(uint64_t max, size_t *len);
uint64_t* eratosthenes(uint64_t max, size_t *len);
void segmented_sieve(uint64_t max, void(*callback)(uint64_t));
void set_prim_error_callback(void (*callback)(char*));
```

See `src/prim.h` for more information.

## CLI

`prim` is a small CLI tool that links against `prim.a`. It can be used as:

```
prim MAX [args]
```

and will simply print out each prime in `[1, MAX]` followed by a newline.
The separator can be changed with `-s`, and can contain the following
escape characters:

```
\t -- tab
\n -- newline
\\ -- backslash literal
```

You may need to type `\\\\` for a backslash, depending on how your shell
handles escape characters.

You can also run `prim -h` to print a brief help text or `prim -version` to
print out version information.

# license

Licensed under a 2-clause BSD license. See LICENSE for more information.
