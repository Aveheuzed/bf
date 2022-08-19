/*
Wrapper around the Arduino functionalities
to ease the integration into the main project.

The symbols declared here are defined in wrapper.cpp.
That is the only meaningful C++ file of the project.

License and documentation are at the top of wrapper.cpp.
*/

#ifndef wrapper_h
#define wrapper_h

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void ttyInit(void);
char ttyGetInput(void);

void SDopen(void);
void SDclose(void);
size_t SDread(void *ptr, size_t size, size_t nmemb);
int SDeof(void);

#ifdef putchar
#undef putchar
#endif
int putchar(int c);

int puts(const char* s);

#ifdef __cplusplus
}
#endif
#endif /* end of include guard: wrapper_h */
