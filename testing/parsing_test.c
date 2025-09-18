#define STATES 7
#define SYMBOLS 2
#include "../parsing.c"

/// Reads in the entirety of the contents of a file into memory, returning it as one long malloc string.
/// This is UNBUFFERED. Don't read in a several gigabyte file or you'll use way more memory than necessary.
string read_file_unbuffered(FILE* file) {
    string return_string = {0};
    fseek(file, 0, SEEK_END);
    usize len = ftell(file);

    return_string.str = calloc(len, sizeof(u8));
    fread(return_string.str, sizeof(u8), len, file);

    return_string.length = len;
    return return_string;
}

int main(int argc, char* argv[]) {
    assert(argc > 1);
    FILE* in = fopen(argv[1], "r");

    string file = read_file_unbuffered(in);
    fclose(in);
}