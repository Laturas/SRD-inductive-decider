#define MAIN
#define STATES 7
#define SYMBOLS 2

_Static_assert(SYMBOLS <= 10, "Only up to 10 symbols is currently supported.");
_Static_assert(STATES <= 26, "Only up to 26 states is currently supported.");

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "defs.h"

typedef state Machine[STATES][SYMBOLS];

#define LETTER_TO_INDEX(a) (((a) & 31) - 1)
#define NUMBER_TO_INT(a) ((a) & 15)
#define IS_VALID_TM_CODE_CHAR(a) (('0' <= (a) && (a) <= '9') /*|| ('a' <= (a) && (a) <= 'z')*/ || ('A' <= (a) && (a) <= 'Z') || ((a) == '_' || (a) == '-'))
#define IS_WHITESPACE(a) ((a == ' ') || (a == '\n') || (a == '\t'))
#define END_CHAR_OF_STR(_string) (_string.str[_string.length])
#define IS_NUMERIC(a) ('0' <= (a) && (a) <= '9')
#define NUMERIC_TO_INT(a) ((a) - '0')
#define INT_TO_NUMERIC(a) ((a) + '0')
#define IS_DIRECTION(a) ((a) == 'R' || (a) == 'L')
#define CHAR_TO_DIR(a) ((a) == 'R' ? RIGHT : LEFT)
#define IS_CAPITAL_ASCII_CHAR(a) ('A' <= (a) <= 'Z')
#define STATE_TO_CHAR(a) ((a) + 'A')
#define STATE_TO_INT(a) ((a) + '0')

#define FLUSH fflush(stdout)

void subroutine_decompose(const Machine input_machine, usize depth) {

}

void pipeline(const Machine input_machine) {
    subroutine_decompose(input_machine, 1);
}

typedef enum _TMCodeParseError {
    SUCCESS,
    TOO_LONG,
    TOO_SHORT,
    TOO_MANY_SYMBOLS,     // TM code has too many symbols defined for a state
    TOO_FEW_SYMBOLS,      // TM code only has a few symbols defined for a state, and doesn't explicitly undefine them
    ILLEGAL_CHARS,        // TM code contains chars that will never appear in a valid TM code.
    INVALID_FORMAT_WRITE, // TM code is invalid. Writes a non-numeric symbols
    INVALID_FORMAT_DIR,   // TM code is invalid. Says to move in a non L or R direction
    BROKEN_FORMAT_GENERIC,// TM code is invalid. Unknown reason.
} _TMCodeParseError;

/// Parses a TM code from input_string and writes it into the provided machine field.
/// Assumes a valid code. Assumes valid memory address to write to.
_TMCodeParseError parse_machine(Machine OUT_machine, const String input_string) {
    enum Expectation {
        WRITE,
        MOVE,
        NEXT_STATE, 
        END_ROW, // Denotes a break with '_'
        // Sounds not implemented yet.
    };

    enum Expectation current_expectation = WRITE;
    usize current_state = 0;
    usize current_symbol = 0;

    for (int i = 0; i < input_string.length; i++) {
        char char_to_process = input_string.str[i];
        switch (current_expectation) {
            case WRITE: {
                if (IS_NUMERIC(char_to_process)) {
                    OUT_machine[current_state][current_symbol].write = NUMERIC_TO_INT(char_to_process);
                    current_expectation = MOVE;
                } else {
                    if (char_to_process != '-') return INVALID_FORMAT_WRITE;
                    OUT_machine[current_state][current_symbol].write = 1;
                    current_expectation = MOVE;
                }

            } break;
            case MOVE: {
                if (IS_DIRECTION(char_to_process)) {
                    OUT_machine[current_state][current_symbol].dir = CHAR_TO_DIR(char_to_process);
                    current_expectation = NEXT_STATE;
                } else {
                    if (char_to_process != '-') return INVALID_FORMAT_DIR;
                    OUT_machine[current_state][current_symbol].dir = RIGHT;
                    current_expectation = NEXT_STATE;
                }
            } break;
            case NEXT_STATE: {
                // Invalid next states are interpreted as "HALT". This means you could denote it with 'H', '-', or 'Z'
                if (IS_CAPITAL_ASCII_CHAR(char_to_process) && (char_to_process) <= ('A' + STATES)) {
                    OUT_machine[current_state][current_symbol].next_state = (char_to_process) - 'A';
                } else {
                    OUT_machine[current_state][current_symbol].next_state = (char)255;
                }
                if (current_symbol + 1 < SYMBOLS) {
                    current_expectation = WRITE;
                    current_symbol++;
                } else {
                    current_expectation = END_ROW;
                }
            } break;
            case END_ROW: {
                if (char_to_process == '_') {
                    current_expectation = WRITE;
                    current_state++;
                    current_symbol = 0;
                }
            } break;

            default: assert(("Unreachable", 0));
        }
    }
    if (current_state != STATES - 1 || current_symbol != SYMBOLS - 1) {
        return TOO_SHORT;
    }
    if (current_expectation == END_ROW) {
        return SUCCESS;
    } else {
        return BROKEN_FORMAT_GENERIC;
    }
}

void process_parsing_error(_TMCodeParseError error) {
    assert(("Parsing of TM code failed", error == SUCCESS));
    // TODO: Handle other cases with better error messages
}

int process_tm_list(const String tm_list) {
    String current_slice = {.str = tm_list.str, .length = 0};
    Machine current_machine = {0};
    while (true) {
        while (IS_VALID_TM_CODE_CHAR(END_CHAR_OF_STR(current_slice))) {
            current_slice.length++;
        }
        // printf("char code: %d\n", (END_CHAR_OF_STR(current_slice))); FLUSH;

        assert(("Empty line encountered", current_slice.length));
        // 3 chars per instruction + 1 underscore separator per state.
        // Remove this assertion once proper error handling has been introduced.
        assert(current_slice.length == 3 * (STATES * SYMBOLS) + (STATES-1));

        printf("%.*s\n", current_slice.length, current_slice.str); FLUSH;

        assert(parse_machine(current_machine, current_slice) == SUCCESS);
        pipeline(current_machine);
        current_slice.str = &END_CHAR_OF_STR(current_slice);
        current_slice.length = 0;
        
        while ((current_slice.str < &END_CHAR_OF_STR(tm_list)) && !IS_VALID_TM_CODE_CHAR(END_CHAR_OF_STR(current_slice))) {
            current_slice.str++;
        }
        if (current_slice.str == &END_CHAR_OF_STR(tm_list)) return 0;
    }
}

void help_menu() {
    // TODO: Implement help menu
}

/// Reads in the entirety of the contents of a file into memory, returning it as one long malloc string.
/// This is UNBUFFERED. Don't read in a several gigabyte file or you'll use way more memory than necessary.
String read_file_unbuffered(FILE* file) {
    String return_string = {0};
    fseek(file, 0, SEEK_END);
    usize len = ftell(file);
    assert(len);
    fseek(file, 0, SEEK_SET);

    return_string.str = calloc(len, sizeof(u8));
    size_t read_bytes = fread(return_string.str, sizeof(u8), len, file);
    assert(("Unknown error encountered when reading in file", read_bytes == len));

    return_string.length = len;
    return return_string;
}

/// Reads in command line arguments. Standard main function stuff.
int main(int argc, char* argv[]) {
    assert(argc > 0);
    if (argc < 2) {
        fprintf(stderr, "Usage: ./<exe> <input file> <params>\n\tOr run ./<exe> -help for a list of parameters\n");
        return 0;
    }
    if (argv[1][0] == '-' && argv[1][1] == 'h') {
        help_menu();
        return 0;
    }

    FILE* in = fopen(argv[1], "rb");

    assert(("Reading input file failed (does the file exist?)", in));
    String file_contents = read_file_unbuffered(in);
    fclose(in);

    assert(file_contents.str > 0);
    assert(("Cannot process empty file - This is usually a bug", file_contents.length));

    process_tm_list(file_contents);

    free(file_contents.str);
    return 0;
}