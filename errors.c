#include <stdlib.h>
#include <stdio.h>

#include "errors.h"

void ErrorMsg(const char *msg) {
	puts(msg);
	abort();
}
