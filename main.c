#define PARAMS

#define STATES 7
#define SYMBOLS 2

// Comment this line out to remove all assertions
// This makes the code run faster, but removes all safety checks.
#define DEBUG

#include "inductive_decider.c"

void help_menu() {
    // TODO: Implement help menu
}

/// Reads in command line arguments. Standard main function stuff.
int main(int argc, char* argv[]) {
    assert("Less than 1 argument (indicates an error)", argc > 0);
    if (argc < 2) {
        fprintf(stderr, "Usage: ./<exe> <input file> <params>\n\tOr run ./<exe> -help for a list of parameters\n");
        return 0;
    }
    if (argv[1][0] == '-' && argv[1][1] == 'h') {
        help_menu();
        return 0;
    }

    FILE* in = fopen(argv[1], "rb");

    assert("Reading input file failed (does the file exist?)", in);
    String file_contents = read_file_unbuffered(in);
    fclose(in);

    assert("Unknown error", file_contents.str > 0);
    assert("Cannot process empty file - This is usually a bug", file_contents.length);

    process_tm_list(file_contents);

    free(file_contents.str);
    return 0;
}