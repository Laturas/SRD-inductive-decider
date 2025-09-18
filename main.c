#define MAIN
#define STATES 7
#define SYMBOLS 2

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "defs.h"

typedef state machine[SYMBOLS][STATES];

#define LETTER_TO_INDEX(a) (((a) & 31) - 1)
#define NUMBER_TO_INT(a) ((a) & 15)
#define IS_VALID_TM_CODE_CHAR(a) (('0' <= (a) && (a) <= '9') || ('a' <= (a) && (a) <= 'z') || ('A' <= (a) && (a) <= 'Z') || ((a) == '_' || (a) == '-'))
#define IS_WHITESPACE(a) ((a == ' ') || (a == '\n') || (a == '\t'))
#define END_CHAR_OF_STR(_string) (_string.str[_string.length])

#define FLUSH fflush(stdout)

void subroutine_decompose(const machine input_machine) {

}

void pipeline(const machine input_machine) {
    subroutine_decompose(input_machine);
}

typedef enum _TMCodeParseError {
    SUCCESS,
    TOO_LONG,
    TOO_MANY_SYMBOLS,     // TM code has too many symbols defined for a state
    TOO_FEW_SYMBOLS,      // TM code only has a few symbols defined for a state, and doesn't explicitly undefine them
    ILLEGAL_CHARS,        // TM code contains chars that will never appear in a valid TM code.
    INVALID_FORMAT_WRITE, // TM code is invalid. Writes a non-numeric symbols
    INVALID_FORMAT_DIR,   // TM code is invalid. Says to move in a non L or R direction
} _TMCodeParseError;

/// Parses a TM code from input_string and writes it into the provided machine field.
/// Assumes a valid code. Assumes valid memory address to write to.
_TMCodeParseError parse_machine(machine OUT_machine, const string input_string) {

}

int process_tm_list(const string tm_list) {
    string current_slice = {.str = tm_list.str, .length = 0};
    machine current_machine = {0};
    while (true) {
        while (IS_VALID_TM_CODE_CHAR(END_CHAR_OF_STR(current_slice))) {
            current_slice.length++;
        }
        // printf("char code: %d\n", (END_CHAR_OF_STR(current_slice))); FLUSH;

        assert(current_slice.length);
        // 3 chars per instruction + 1 underscore separator per state.
        // Remove this assertion once proper error handling has been introduced.
        assert(current_slice.length == 3 * (STATES * SYMBOLS) + (STATES-1));

        printf("%.*s\n", current_slice.length, current_slice.str); FLUSH;

        parse_machine(current_machine, current_slice);
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
    
}

/// Reads in the entirety of the contents of a file into memory, returning it as one long malloc string.
/// This is UNBUFFERED. Don't read in a several gigabyte file or you'll use way more memory than necessary.
string read_file_unbuffered(FILE* file) {
    string return_string = {0};
    fseek(file, 0, SEEK_END);
    usize len = ftell(file);
    assert(len);
    fseek(file, 0, SEEK_SET);

    return_string.str = calloc(len, sizeof(u8));
    size_t read_bytes = fread(return_string.str, sizeof(u8), len, file);
    assert(read_bytes == len);

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

    assert(in);
    string file_contents = read_file_unbuffered(in);
    fclose(in);

    assert(file_contents.str > 0);
    assert(file_contents.length);

    process_tm_list(file_contents);

    free(file_contents.str);
    return 0;
}