#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <getopt.h>

#define VERSION "1.1"

typedef enum {
    NONE = 0,
    HEX = 1 << 0,
    DEC = 1 << 1,
    OCT = 1 << 2,
    BIN = 1 << 3,
} format_t;

void usage(char *cmd) {
    fprintf(stderr, "Usage: %s [OPTIONS] <from>\n", cmd);
    fprintf(stderr, "<from> is an integer that may be specified as one of the following formats: 31. 0x1f 037 b11111\n");
    fprintf(stderr, "OPTIONS\n");
    fprintf(stderr, "-o --output [FORMAT]  Set the output format. Default is to output in all formats. Available formats: (dec|hex|oct|bin).\n");
    fprintf(stderr, "-h --help             Display this help message.\n");
    fprintf(stderr, "-v --version          Display version number.\n");
}

void version(void) {
    printf("Version " VERSION "\n");
}

int parse(char *input, long *output) {

    format_t fmt = NONE;
    size_t len = strlen(input);
    if (len > 2 && strncmp("0x", input, 2) == 0) {
        fmt = HEX;
        input += 2;
    } else if (len > 2 && strncmp("0X", input, 2) == 0) {
        fmt = HEX;
        input += 2;
    } else if (len > 1 && *input == 'x') {
        fmt = HEX;
        input += 1;
    } else if (len > 1 && *input == '0') {
        // avoid accidental confusion with binary not prefixed with 'b'
        const char *c = input;
        while (*c != '\0') {
            if (!(*c == '0' || *c == '1')) { // not a 1 or 0
                fmt = OCT;
                input += 1;
                break;
            }
            c++;
        }
    } else if (len > 1 && *input == 'b') {
        fmt = BIN;
        input += 1;
    } else if (len > 1 && input[len-1] == '.') {
        fmt = DEC;
        input[len-1] = '\0';
    }

    if (fmt == NONE) {
        int possible = HEX | DEC | OCT;
        const char *c = input;
        while (*c != '\0') {
            if (!(*c == '0' || *c == '1')) { // not a 1 or 0
                possible &= ~BIN;
            }
            if (!(*c <= '7' && *c >= '0')) { // not in [0-7]
                possible &= ~OCT;
            }
            if (!(*c <= '9' && *c >= '0')) { // not in [0-9]
                possible &= ~DEC;
            }
            if (!((*c <= 'f' && *c >= 'a') || (*c <= 'F' && *c >= 'A') || (*c <= '9' && *c >= '0'))) { // not in [f-aF-A0-9]
                possible &= ~HEX;
            }
            c++;
        }
        switch (possible) {
            case HEX:
                fmt = HEX;
                break;
            case DEC:
                fmt = DEC;
                break;
            case OCT:
                fmt = OCT;
                break;
            case BIN:
                fmt = BIN;
                break;
            default:
                break;
        }
    }

    long parsed = 0;
    switch (fmt) {
        case HEX:
            fprintf(stderr, "Format detected as hexadecimal\n");
            fmt = HEX;
            parsed = strtol(input, NULL, 16);
            break;
        case DEC:
            fprintf(stderr, "Format detected as decimal\n");
            fmt = DEC;
            parsed = strtol(input, NULL, 10);
            break;
        case OCT:
            fprintf(stderr, "Format detected as octal\n");
            fmt = OCT;
            parsed = strtol(input, NULL, 8);
            break;
        case BIN:
            fprintf(stderr, "Format detected as binary\n");
            fmt = BIN;
            parsed = strtol(input, NULL, 2);
            break;
        default:
            fprintf(stderr, "Unable to determine format for `%s'\n", input);
            return -1;
            break;
    }
    if (parsed == LONG_MAX || parsed == LONG_MIN || errno == ERANGE) {
        perror("Unable to parse");
        return -1;
    }
    *output = parsed;

    return 0;
}

void revstr(char *str) {
    char tmp;
    char *p1 = str;
    char *p2 = str + strlen(str) - 1;

    while (p2 > p1) {
        tmp = *p1;
        *p1++ = *p2;
        *p2-- = tmp;
    }
}

int binstr(long num, char *str, int breaks) {
    char *c = str;
    long v = num;
    for (int i = 0; i < sizeof(long) * 8; i++) {
        *c = !!(v & 0x1) ? '1' : '0';
        v >>= 1;
        c++;
        if (v == 0) {
            break;
        }
        if (breaks && !((i+1) % breaks)) {
            *c++ = ' ';
        }
    }
    *c = '\0';
    revstr(str);
    return c - str;
}

void printnum(long num, format_t format) {
    char str[1024];
    if (format == NONE) {
        int d = 0;
        int h = 0;
        int o = 0;
        int b = 0;

        d = sprintf(str, "%ld     ", num);
        h = sprintf(str + d, "0x%lx     ", num);
        o = sprintf(str + d + h, "0%lo     ", num);
        b = binstr(num, str + d + h + o, 4);

        printf("%-*s%-*s%-*s%-*s\n", d, "dec", h, "hex", o, "oct", b, "bin");
        printf("%s\n", str);
        
        binstr(num, str, 0);
        printf("%*s%s\n", d + h + o, "", str);
    } else if (format == DEC) {
        printf("%ld\n", num);
    } else if (format == HEX) {
        printf("0x%lx\n", num);
    } else if (format == OCT) {
        printf("0%lo\n", num);
    } else if (format == BIN) {
        binstr(num, str, 0);
        printf("%s\n", str);
    }
}

int main(int argc, char *argv[]) {
    static struct option long_options[] = {
        {"output", required_argument, 0, 'o'},
        {"version", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    format_t output_format = NONE;
    while (1) {
        int option_index = 0;
        int c = getopt_long (argc, argv, "hvo:", long_options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
            case 0:
                break;
            case 'o':
                if (strcmp(optarg, "dec") == 0) {
                    output_format = DEC;
                } else if (strcmp(optarg, "hex") == 0) {
                    output_format = HEX;
                } else if (strcmp(optarg, "oct") == 0) {
                    output_format = OCT;
                } else if (strcmp(optarg, "bin") == 0) {
                    output_format = BIN;
                } else {
                    fprintf(stderr, "unknown output format `%s'\n", optarg);
                    return -1;
                }
                break;
            case 'v':
                version();
                return 0;
                break;
            case 'h':
                usage(argv[0]);
                return 0;
                break;
            case '?':
                break;
            default:
                usage(argv[0]);
                return -1;
                break;
        }
    }
    char *cmd = argv[0];
    argc -= optind;
    argv += optind;
   
    if (argc != 1) {
        usage(cmd);
        return -1;
    }
    long parsed = 0;
    if (parse(argv[0], &parsed) != 0) {
        return -1;
    }
    printnum(parsed, output_format);
    return 0;
}
