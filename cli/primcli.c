#include <stdio.h>
#include <stdlib.h>

#include <getopt.h> 

#include "prim.h"

const int EXIT_OK = 0;
const int EXIT_BAD_ARGS = -1;

void help(void);
void version(void);

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("invalid arguments.\n");
        printf("run prim -h for more information.\n");
        return EXIT_BAD_ARGS;
    }

    char *sep = "\n";

    int c = '\0';
    while ((c = getopt(argc, argv, "hvs:")) != -1) {
        switch (c) {
            case 'h':
                help();
                return EXIT_OK;
            case 'v':
                version();
                return EXIT_OK;
            case 's':
               sep = optarg;
               break;
            case '?':
            case ':':
               return EXIT_BAD_ARGS;
            default:
                printf("unknown argument '%c'\n", c);
                return EXIT_BAD_ARGS;
        }
    }

    uint64_t max = strtol(argv[argc - 1], NULL, 10);
    size_t len = 0;
    uint64_t *primes = prime_sieve(max, &len);
    for (size_t i = 0; i < len; ++i)
        printf("%zu%s", primes[i], sep);
    printf("\n");

    return EXIT_OK;
}

void help(void) {
    printf("usage: prim [args] MAX\n");
    printf("\n");
    printf("args:\n");
    printf("-s STR   \tset the separator string\n");
    printf("-h       \tshow program usage\n");
    printf("-v       \tshow program version information\n");
}

void version(void) {
    printf("prim v%s\n", PRIM_VERSION);
    printf("(c) Phillip Heikoop 2016\n");
}
