#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

typedef enum {
    NONE = 0,
    HEX = 1 << 0,
    DEC = 1 << 1,
    OCT = 1 << 2,
    BIN = 1 << 3,
} format_t;

void usage(char *cmd) {
    fprintf(stderr, "Usage: %s <from>\n", cmd);
    fprintf(stderr, "<from> is an integer that may be specified as one of the following formats: 0x1f 31. b11111 037\n");
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
        fmt = OCT;
        input += 1;
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
            //fprintf(stderr, "c=%c\n", *c);
            if (*c != '0' || *c != '1') {
                //fprintf(stderr, "not bin\n");
                possible &= ~BIN;
            }
            if (!(*c <= '7' && *c >= '0')) {
                //fprintf(stderr, "not oct\n");
                possible &= ~OCT;
            }
            if (!(*c <= '9' && *c >= '0')) {
                //fprintf(stderr, "not dec\n");
                possible &= ~DEC;
            }
            if (!((*c <= 'f' && *c >= 'a') || (*c <= 'F' && *c >= 'A') || (*c <= '9' && *c >= '0'))) {
                //fprintf(stderr, "not hex\n");
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

void printnum(long num) {

    char str[1024];
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

}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        usage(argv[0]);
        return -1;
    }

    long parsed = 0;
    if (parse(argv[1], &parsed) != 0) {
        return -1;
    }

    printnum(parsed);

    return 0;
}
