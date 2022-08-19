/* errors.h
 * (C) Stephan Beyer, 2003, 2004, GPL
 * (C) Alexis Masson, 2022, GPL */
#ifndef ERRORS_H
#define ERRORS_H

#define DoAndErr(COND,STR)	if (COND) ErrorMsg(STR);

#include "wrapper.h"

#ifdef __cplusplus
extern "C"
#endif
__attribute__((noreturn))
void ErrorMsg(const char *msg);

#endif
