#include <stdio.h>
#include <stdlib.h>

#ifndef PARAMS
    #define STATES 7
    #define SYMBOLS 2
    #define DEBUG
#endif

#include "asserts.h"
#ifndef DEFINITIONS
    #include "defs.h"
#endif

#include "arena_allocator.c"


_Static_assert(SYMBOLS <= 10, "Only up to 10 symbols is currently supported.");
_Static_assert(STATES <= 26, "Only up to 26 states is currently supported.");

typedef Instruction Machine[STATES][SYMBOLS];
typedef struct {
    char* tape;
    usize count;
    int state;
    usize current_position;
    usize min_visited;
    usize max_visited;
} TapeState;


TapeState tape_state_init(usize tape_width) {
    TapeState tape_state = {0};
    char* tape = calloc(tape_width, sizeof(typeof(*tape_state.tape)));
    tape_state.tape = tape;
    tape_state.count = tape_width;
    tape_state.current_position = tape_state.count / 2;
    tape_state.state = 0;
    tape_state.max_visited = tape_state.current_position;
    tape_state.min_visited = tape_state.current_position;
    return tape_state;
}

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
#define HALT_STATE (char)255

#define FLUSH fflush(stdout)

void subroutine_decompose(const Machine input_machine, usize depth) {
    assert("Only depth 1 supported", depth == 1);
    bool connection_field[STATES * SYMBOLS] = {0};
    
}

typedef enum TMSimulationResult {
    SIMULATION_HALTED,        // TM simulation reached the halt state
    SIMULATION_MAX_STEPS,     // TM simulation reached the maximum step count without halting
    SIMULATION_OUT_OF_MEMORY, // TM simulation went past the maximum amount of memory allocated to it
} TMSimulationResult;

/// Returns true if the machine halted. False if not.
TMSimulationResult simulate_unaccelerated(const Machine input_machine, TapeState* config, const usize number_of_steps) {
    #define CURRENT_CELL config->tape[config->current_position]
    for (usize step = 0; step < number_of_steps; step++) {
        Instruction current_instruction = input_machine[config->state][CURRENT_CELL];
        
        if (current_instruction.next_state == HALT_STATE) {
            return SIMULATION_HALTED;
        }

        char to_write = current_instruction.write;
        char next_state = current_instruction.next_state;
        Direction next_direction = current_instruction.dir;
        CURRENT_CELL = to_write;
        if ((config->current_position == 0 && next_direction == LEFT) ||
            (config->current_position >= config->count - 1 && next_direction == RIGHT)) {
            return SIMULATION_OUT_OF_MEMORY;
        }
        config->current_position += next_direction;
        config->state = next_state;
    }
    return SIMULATION_MAX_STEPS;
    #undef CURRENT_CELL
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
        if (current_state >= STATES) return TOO_LONG;

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
                if (IS_CAPITAL_ASCII_CHAR(char_to_process) && (char_to_process) <= ('A' + STATES) && (char_to_process) >= 'A') {
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

            default: assert("Unreachable", false);
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
    assert("Parsing of TM code failed", error == SUCCESS);
    // TODO: Handle other cases with better error messages
}

int process_tm_list(const String tm_list) {
    String current_slice = {.str = tm_list.str, .length = 0};
    Machine current_machine = {0};
    while (true) {
        while (IS_VALID_TM_CODE_CHAR(END_CHAR_OF_STR(current_slice))) {
            current_slice.length++;
        }

        assert("Empty line encountered", current_slice.length);
        // 3 chars per instruction + 1 underscore separator per state.
        // Remove this assertion once proper error handling has been introduced.
        assert("3 chars per instruction + 1 underscore separator per state.", current_slice.length == 3 * (STATES * SYMBOLS) + (STATES-1));

        printf("%.*s\n", current_slice.length, current_slice.str); FLUSH;

        assert("Parsing machine failed", parse_machine(current_machine, current_slice) == SUCCESS);
        pipeline(current_machine);
        current_slice.str = &END_CHAR_OF_STR(current_slice);
        current_slice.length = 0;
        
        while ((current_slice.str < &END_CHAR_OF_STR(tm_list)) && !IS_VALID_TM_CODE_CHAR(END_CHAR_OF_STR(current_slice))) {
            current_slice.str++;
        }
        if (current_slice.str == &END_CHAR_OF_STR(tm_list)) return 0;
    }
}

/// Reads in the entirety of the contents of a file into memory, returning it as one long malloc string.
/// This is UNBUFFERED. Don't read in a several gigabyte file or you'll use way more memory than necessary.
String read_file_unbuffered(FILE* file) {
    String return_string = {0};
    fseek(file, 0, SEEK_END);
    usize len = ftell(file);
    assert("Empty file input (likely indicates a bug)", len);
    fseek(file, 0, SEEK_SET);

    return_string.str = calloc(len, sizeof(u8));
    size_t read_bytes = fread(return_string.str, sizeof(u8), len, file);
    assert("Unknown error encountered when reading in file", read_bytes == len);

    return_string.length = len;
    return return_string;
}

typedef enum DecisionStatus {
    HALTS,      // The machine was proven to halt
    INFINITE,   // The machine was proven to run forever
    UNDECIDED,  // The machine could not be decided.
} DecisionStatus;
// InstructionInstance* tape_history;

typedef struct Contract {
    u32 enter_state;
    u32 exit_state;
    u32 block_definition;
} Contract;

/// Block structure:
/// (string of chars)^len(string of chars)^len...
/// Need a count of strings
/// Store them contiguously
typedef struct Block {
    u32 block_hash;
    u32 sub_block_count;
    RelativeArenaPtr first_string;
} Block;
int circular_buffer_size;
u32* circular_buffer;

typedef enum SimulationInterrupt {
    INTERRUPT_HALT,             // Yay! Machine halted!
    INTERRUPT_UNDEFINED_BLOCK,  // Machine reached a transition it doesn't know how to handle
    INTERRUPT_OUT_OF_MEMORY,    // Ran out of memory
} SimulationInterrupt;

Arena scratch_arena;
Arena block_definitions;
Arena run_length_tape_arena;

// Tape: block run_length block run_length

/// Does the actual accelerated simulation of the turing machine. It returns an interrupt when it finishes
SimulationInterrupt accelerated_run(const Machine input_machine, TapeState* config,
    const usize max_number_of_strides, usize* executed_strides) {

    #define CURRENT_CELL config->tape[config->current_position]
    for (usize step = *executed_strides; step < max_number_of_strides; step++) {
        Instruction current_instruction = input_machine[config->state][CURRENT_CELL];
        
        if (current_instruction.next_state == HALT_STATE) {
            return INTERRUPT_HALT;
        }

        char to_write = current_instruction.write;
        char next_state = current_instruction.next_state;
        Direction next_direction = current_instruction.dir;
        CURRENT_CELL = to_write;
        if ((config->current_position == 0 && next_direction == LEFT) ||
            (config->current_position >= config->count - 1 && next_direction == RIGHT)) {
            return SIMULATION_OUT_OF_MEMORY;
        }
        config->current_position += next_direction;
        config->state = next_state;
    }
    return SIMULATION_MAX_STEPS;
    #undef CURRENT_CELL
}

typedef struct RLBlock {
    usize block;
    usize run_length;
} RLBlock;

typedef struct {
    RLBlock* tape;
    usize count;
    int state;
    usize current_position;
    usize min_visited;
    usize max_visited;
} RLETapeState;

void print_rletape(RLETapeState rle_tape_state, FILE* out) {
    fprintf(out, "$ ");
    for (int i = rle_tape_state.min_visited; i <= rle_tape_state.max_visited; i++) {
        if (rle_tape_state.current_position == i) {
            fprintf(out, "%c> ", STATE_TO_CHAR((char)rle_tape_state.state));
        }
        fprintf(out, "(%c)^%llu ", INT_TO_NUMERIC((char)rle_tape_state.tape[i].block), rle_tape_state.tape[i].run_length);
    }
    fprintf(out, "$");
}

RLETapeState run_length_collapse(RLETapeState* config, RLBlock collapse_block) {
    assert("TODO: run_length_collapse(RLETapeState* config, RLBlock collapse_block) Unimplemented", false);
}

RLETapeState run_length_collapse_raw_to_rle(const TapeState config, RLBlock collapse_block, RLBlock* run_tape) {
    RLETapeState final_tape = {run_tape, config.count, config.state, config.current_position, config.min_visited, config.max_visited};
    int current_run_length = 0;
    int final_tape_pos = config.min_visited;
    for (int i = config.min_visited; i <= config.max_visited; i++) {
        if (config.tape[i] == collapse_block.block) {
            final_tape.max_visited = (current_run_length == 0) ? final_tape.max_visited : final_tape.max_visited - 1;
            current_run_length++;
            if (i < final_tape.current_position) {
                final_tape.current_position--;
            }
            continue;
        }
        if (current_run_length > 0) {
            RLBlock run_length_block = {collapse_block.block, current_run_length};
            final_tape.tape[final_tape_pos] = run_length_block;
            final_tape_pos++;
            current_run_length = 0;
        }
        RLBlock non_rl_block = {config.tape[i], 1};
        final_tape.tape[final_tape_pos] = non_rl_block;
        final_tape_pos++;
    }
    if (current_run_length > 0) {
        RLBlock run_length_block = {collapse_block.block, current_run_length};
        final_tape.tape[final_tape_pos] = run_length_block;
        final_tape_pos++;
        current_run_length = 0;
    }
    return final_tape;
}

/// A "stride" is equivalent to one accelerated step.
///
/// A "contract" defines a stride. It includes what needs to be on the tape, what state the machine needs to be when entering
/// the block, and what state it will be when it exits the block
///
/// Example contract: {A>, C(a,b,c), D>}
///
/// Question 1: How do we store a list of contracts to be quickly accessible?
/// Question 2: How do we detect when to make a new contract?
///
TMSimulationResult simulate_accelerated(const Machine input_machine, TapeState* config, const usize max_number_of_strides) {
    scratch_arena         = arena_init(4096);
    block_definitions     = arena_init(4096);
    run_length_tape_arena = arena_init(sizeof(RLBlock) * 4096);

    RelativeArenaPtr rle_tape = relative_pointer(run_length_tape_arena, aalloc_zero(&run_length_tape_arena, sizeof(RLBlock) * 4096));

    // RLETapeState state = {
    //     rle_tape;
    // }
    // // tape_history = aalloc(&scratch_arena, S);

}