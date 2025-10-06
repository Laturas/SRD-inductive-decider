#include <assert.h>
#undef assert

#define assert(message, expression) (void)((!!(expression)) || (fprintf(stderr, "Assertion (%s) at line %d failed:\n\t%s\nTerminating program", #expression, __LINE__, message), exit(-1), 0))