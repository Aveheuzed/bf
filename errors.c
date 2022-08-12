/* errors.c
******************************************************************************
   Author: Stephan Beyer
   
   Description of this file:
       error handling

   Copyright (C) GPL, 2000,2001,2002,2003,2004 Stephan Beyer - s-beyer@gmx.net
  
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************************
*/

#include <errno.h> /* errno ... */
#include <stdio.h> /* fprintf... */
#include <stdlib.h> /* exit ... */
#include <string.h> /* strerror ... */
#include "errors.h"

/* prints an error message to stderr and exits with given nr,
 * doesn't exit when nr==0 */
void ErrorMsg(signed int nr, const char *msg)
{
	fprintf(stderr, "Error: %s\n", msg);
	if (nr)
	{
		puts(ERROCCMSG);
		exit(nr);
	}
}

/* Usage: HandleError("function", __FILE__, __LINE__, 0)
 * - nr specifies the exit code (if nonzero)
 *   if zero, it won't exit... */
void HandleError(const char *def, char *file, unsigned int line, signed int nr)
{ 
	char *separator;

	if (def == NULL || *def == '\0')
		def = separator = "";
	else
		separator = " - ";
	
	fprintf(stderr, "%s%s%s (in file %s, line %d)\n", def, separator, 
			strerror(errno), file, line);
	
	if (nr)
	{
		puts(ERROCCMSG);
		exit(nr);
	}
}
