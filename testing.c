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
    free(tape_state.tape);
    
    tape_state = tape_state_init(1);
    assert("This test would probably segfault if it failed", simulate_unaccelerated(test_bb5_champ, &tape_state, 1000) == SIMULATION_OUT_OF_MEMORY);
    free(tape_state.tape);

    fprintf(stderr, "Unaccelerated running tests passed\n");
}

/// Reads in command line arguments. Standard main function stuff.
int main(int argc, char* argv[]) {
    test_parsing();
    test_unaccelerated_running();

    fprintf(stderr, "All tests passing\n");
    return 0;
}