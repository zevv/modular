#include "chip.h"
#include "printd.h"

#define assert(x) if(!(x)) { printd("assert %s\n", #x); for(;;); }

