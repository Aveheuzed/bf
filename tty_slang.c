/* tty_slang.c
******************************************************************************
   This file is part of `bf', Yet another Brainfuck interpreter.

   Author: Stephan Beyer

   Description of this file:
       tty handling routines using S-Lang library
   
   Copyright (C) GPL, 2004 Stephan Beyer - s-beyer@gmx.net
  
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

#include <slang.h> /* S-Lang library */

#include "tty.h"
#include "bf.h"
#include "errors.h"

void ttyRestore(void)
{
	SLang_reset_tty();
}

/* implementations of the ',' operation */
char ttyGetInput(void)
{
	char c;
	/* We init and restore the tty on each input (and not
	 * once at start and end of program) to prevent bugs with
	 * bf output... */
	/* init slang terminal */
	DoAndErr(SLang_init_tty(-1, 0, 1) == -1, SLang_init_tty,
							errSLANG);
	/* get any key */
	if (opt.inputmode <= 2) /* 1 or 2 */
		c = SLang_getkey();
	else /* 3 or 4 */
	{ /* get key but no escape characters */
		int tmp;
		do {
			tmp = SLkp_getkey();
		} while(tmp > 255);
		c = (unsigned char)tmp;
	}
	
	if (c == '\r') /* slang returns \r, make \n */
		c = '\n';
	
	if (!(opt.inputmode % 2)) /* 2 and 4 */
	{
		putchar(c); /* print them */
		fflush(stdout);
	}
	/* and restore */
	ttyRestore();
	return c;
}

void ttyInit(void)
{
	/* init slang kp if mode 3 or 4 */
	if ((opt.inputmode == 3)
	 || (opt.inputmode == 4))
	{
		SLtt_get_terminfo();
		DoAndErr(SLkp_init() == -1,
			SLkp_init, errSLANG);
	}
}
