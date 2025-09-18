typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;
typedef size_t usize;

#define true 1
#define false 0
#define bool char

typedef struct {
    char* str;
    usize length;
} string;

typedef struct TM_Tape {
    int small_index;
    int big_index;

    int head_pos;
    char head_state;

    char* tape;
    int tape_width;
} TM_Tape;

typedef enum direction {
	HALT,
	LEFT,
	RIGHT,
} direction;

typedef struct state {
	char write;
	direction dir;
	char next_state;
} state;