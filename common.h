// common state structs, the instance is just in the main.c file.
#include "defines.h"

typedef struct CommonState {
  u8 *is_running; // make this a pointer so that we can pass it down alone.
} CommonState;
