#define PARAMS

#define STATES 7
#define SYMBOLS 2

// Comment this line out to remove all assertions
// This makes the code run faster, but removes all safety checks.
#define DEBUG

#include "inductive_decider.c"

void test_parsing() {
    Machine result = {0};
    char* normal_parse_error = "Failed to parse a normal TM";
    String test_string = {"1RB0RA_1LC1LF_1RD0LB_1RA1LE_---0LC_1RG1LD_0RG0RF", sizeof("1RB0RA_1LC1LF_1RD0LB_1RA1LE_---0LC_1RG1LD_0RG0RF")};
    
    assert(normal_parse_error, parse_machine(result, test_string) == SUCCESS);
    assert(normal_parse_error, result[0][0].write == 1);
    assert(normal_parse_error, result[0][0].dir == RIGHT);
    assert(normal_parse_error, STATE_TO_CHAR(result[0][0].next_state) == 'B');
    
    assert(normal_parse_error, result[4][0].write == 1);
    assert(normal_parse_error, result[4][0].dir == RIGHT);
    assert(normal_parse_error, result[4][0].next_state == HALT_STATE);
    
    assert(normal_parse_error, result[6][1].write == 0);
    assert(normal_parse_error, result[6][1].dir == RIGHT);
    assert(normal_parse_error, STATE_TO_CHAR(result[6][1].next_state) == 'F');

    char* illegal_parse_error = "Failed to parse an illegal TM";
    
    String machine_too_long = {"1RB0RA_1LC1LF_1RD0LB_1RA1LE_---0LC_1RG1LD_0RG0RF_------", sizeof("1RB0RA_1LC1LF_1RD0LB_1RA1LE_---0LC_1RG1LD_0RG0RF_------")};
    assert(illegal_parse_error, parse_machine(result, machine_too_long) == TOO_LONG);

    String machine_illegal_symbols = {"!RB1RB", sizeof("!RB1RB")};
    assert(illegal_parse_error, parse_machine(result, machine_illegal_symbols) == INVALID_FORMAT_WRITE);
    
    String machine_illegal_direction = {"1SB1SB", sizeof("1SB1SB")};
    assert(illegal_parse_error, parse_machine(result, machine_illegal_direction) == INVALID_FORMAT_DIR);
    
    String test_1RZ = {"0LA0RA_1LC1LF_1RD0LB_1RA1LE_1RZ0LC_1RG1LD_0RG0RF", sizeof("0LA0RA_1LC1LF_1RD0LB_1RA1LE_1RZ0LC_1RG1LD_0RG0RF")};
    assert(illegal_parse_error, parse_machine(result, test_1RZ) == SUCCESS);

    assert(normal_parse_error, result[4][0].write == 1);
    assert(normal_parse_error, result[4][0].dir == RIGHT);
    assert(normal_parse_error, result[4][0].next_state == HALT_STATE);

    assert(normal_parse_error, result[0][0].write == 0);
    assert(normal_parse_error, result[0][0].dir == LEFT);
    assert(normal_parse_error, STATE_TO_CHAR(result[0][0].next_state) == 'A');
    
    fprintf(stderr, "Parsing tests passed\n");
}

void test_unaccelerated_running() {
    String test_string = {"1RA0RA_1LC1LF_1RD0LB_1RA1LE_---0LC_1RG1LD_0RG0RF", sizeof("1RA0RA_1LC1LF_1RD0LB_1RA1LE_---0LC_1RG1LD_0RG0RF")};
    Machine tc_machine = {0};
    
    assert("Parsing failed", parse_machine(tc_machine, test_string) == SUCCESS);
    TapeState tape_state = tape_state_init(2048);
    assert("Test erroneously reported that a TC halted (or ran out of memory)", simulate_unaccelerated(tc_machine, &tape_state, 1000) == SIMULATION_MAX_STEPS);
    assert("", tape_state.state == 0);
    assert("", tape_state.current_position == (tape_state.count / 2 + 1000));
    
    free(tape_state.tape);
    
    String bb5_champ_string = {"1RB1LC_1RC1RB_1RD0LE_1LA1LD_1RZ0LA_------_------", sizeof("1RB1LC_1RC1RB_1RD0LE_1LA1LD_1RZ0LA_------_------")};
    Machine test_bb5_champ = {0};
    assert("Parsing BB5 champ failed", parse_machine(test_bb5_champ, bb5_champ_string) == SUCCESS);
    tape_state = tape_state_init(65565);
    assert("Test erroneously reported that the BB5 champ halts early", simulate_unaccelerated(test_bb5_champ, &tape_state, 47176869) == SIMULATION_MAX_STEPS);
    assert("Test erroneously reported that a halting TM didn't halt", simulate_unaccelerated(test_bb5_champ, &tape_state, 1) == SIMULATION_HALTED);
    
    // RLBlock zero_block = {0, 1};
    // RLBlock one_block = {1, 1};
    // // Collapse zeros
    // RLBlock* rle_tape = calloc( 65565, sizeof(RLBlock));
    // RLETapeState rlets = run_length_collapse_raw_to_rle(tape_state, zero_block, rle_tape);
    // Collapse ones
    // run_length_collapse(&rlets, one_block);

    // print_rletape(rlets, stdout);
    
    // free(rle_tape);
    free(tape_state.tape);
    
    tape_state = tape_state_init(1);
    assert("This test would probably segfault if it failed", simulate_unaccelerated(test_bb5_champ, &tape_state, 1000) == SIMULATION_OUT_OF_MEMORY);
    free(tape_state.tape);

    fprintf(stderr, "Unaccelerated running tests passed\n");
}

void test_arena_allocator() {
    Arena test_arena = arena_init(1024);
    assert("Bytes is null", test_arena.bytes != NULL);
    assert("Bytes in use isn't 0 on initialization", test_arena.number_of_bytes_in_use == 0);
    assert("Allocated the wrong number of bytes", test_arena.underlying_allocation_amount == 1024);
    u8* allocated_block = aalloc(&test_arena, 1025);
    assert("Invalid memory returned", allocated_block != NULL);

    u8* second_allocated_block = aalloc_zero(&test_arena, 500);
    assert("Relative pointer fail", relative_pointer(test_arena, second_allocated_block) == (RelativeArenaPtr)1025);
    assert("Incorrect number of bytes in use", test_arena.number_of_bytes_in_use == 1525);
    assert("Incorrect underlying byte count", test_arena.underlying_allocation_amount == 2048);
    for (usize i = 0; i < 500; i++) {
        assert("aalloc_zero doesn't zero memory", second_allocated_block[i] == 0);
    }
    u8* old_ptr = test_arena.bytes;
    aclear(&test_arena);
    assert("Pointer changed", test_arena.bytes == old_ptr);
    afree(&test_arena);

    fprintf(stderr, "Arena allocator tests passed\n");
}

void test_rle_collapse() {
    Arena scratch_arena = arena_init(sizeof(RLBlock) * 4096);
    RLBlock* rle_tape = aalloc_zero(&scratch_arena, sizeof(RLBlock) * 1024);

    char* raw_tape = aalloc_zero(&scratch_arena, 1024);
    TapeState config = {raw_tape, 1024, 0, 500, 500, 550};
    config.tape[525] = 1;
    config.tape[526] = 1;
    RLBlock zero_block = {0, 1};
    RLBlock one_block = {1, 1};

    // Collapse zeros
    RLETapeState rlets = run_length_collapse_raw_to_rle(config, zero_block, rle_tape);
    // Collapse ones
    run_length_collapse(&rlets, one_block);

    FILE* test_file = fopen("__testing.txt", "wb+"); // This is the only way I could find to test this in a platform-independent manner.
    assert("File opening during test failed", test_file);
    print_rletape(rlets, test_file);
    String contents = read_file_unbuffered(test_file);

    // The - 1 in the sizeof comes from the fact that the last character wont match (due to platform independence, it may be \n or \r\n.)
    // (the string being compared to will obviously end with \0)
    assert("Simple tape collapse failure",
        strncmp(contents.str, "$ A> (0)^25 (1)^2 (0)^24 $", sizeof("$ A> (0)^25 (1)^2 (0)^24 $") - 1) == 0
    );
    free(contents.str);

    assert("File closing during test failed", !fclose(test_file));
    assert("File removal during test failed", !remove("__testing.txt"));
    afree(&scratch_arena);

    fprintf(stderr, "Simple rle tests passed\n");
}

/// Reads in command line arguments. Standard main function stuff.
int main(int argc, char* argv[]) {
    // These tests are laid out in order of dependency
    // If an earlier one fails, the other ones will (probably) fail
    test_arena_allocator();
    test_parsing();
    test_unaccelerated_running();
    test_rle_collapse();

    fprintf(stderr, "\nAll tests passing\n");
    return 0;
}