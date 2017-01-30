#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h> 

#include "prim.h"

const int EXIT_OK = 0;
const int EXIT_BAD_ARGS = -1;

bool custom_sep = false;
char *sep = "\n";

char* parse_sep(const char *str);
void print_prime(uint64_t prime);

void help(void);
void version(void);

/*
 * CLI interface for generating prime numbers. Parses the first
 * argument as an upper bound, then runs segmented_sieve() with
 * callback to print each prime to stdout. The default separator
 * between primes is a newline '\n' character, this can be changed
 * to an arbitrary string through the '-s' argument. The only escape
 * characters the separator can have are tabs '\t' newlines '\n' and
 * backslashes '\\'.
 */
int main(int argc, char **argv) {
    if (argc < 2) {
        printf("invalid arguments.\n");
        printf("run prim -h for more information.\n");
        return EXIT_BAD_ARGS;
    }

    uint64_t max = strtol(argv[1], NULL, 10);

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
                if (custom_sep) free(sep);
                sep = parse_sep(optarg);
                custom_sep = true;
               break;
            case '?':
            case ':':
            default:
               if (optind != 1) {
                   return EXIT_BAD_ARGS;
               }
        }
    }

    segmented_sieve(max, &print_prime);

    if (custom_sep) free(sep);
    return EXIT_OK;
}

/*
 * parse '\t', '\n', and '\\' escape characters into a new string.
 */
char* parse_sep(const char *str) {
    char *outstr = malloc((strlen(str) + 1) * sizeof(char));
    char *out = outstr;
    bool escape = false;
    char c = 0;
    while ((c = *str++) != '\0') {
        if (escape) {
            escape = false;
            switch (c) {
                case 'n':
                    *out++ = '\n';
                    break;
                case 't':
                    *out++ = '\t';
                    break;
                case '\\':
                    *out++ = '\\';
                    break;
                default:
                    printf("Warning -- unknown escape character '%c'.\n", c);
                    break;
            }
        } else {
            escape = (c == '\\');
            if (!escape) *out++ = c;
        }
    }

    *out = '\0';
    return outstr;
}

/*
 *  callback for segmented_sieve()
 */
void print_prime(uint64_t prime) {
    printf("%lld%s", (long long) prime, sep);
}

void help(void) {
    printf("usage: prim MAX [args]\n");
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
