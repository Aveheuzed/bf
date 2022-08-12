/* tty_termios.c
******************************************************************************
   This file is part of `bf', Yet another Brainfuck interpreter.

   Author: Stephan Beyer

   Description of this file:
       tty handling routines using termios.h

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

#include <stdio.h> /* getchar */
#include <termios.h>
#include <unistd.h>

#include "tty.h"
#include "bf.h"

static struct termios *tty = NULL; /* save tty settings */

/* restore tty settings */
void ttyRestore(void)
{
	if (tty)
		tcsetattr(STDIN_FILENO, TCSANOW, tty);
}

/* set handlers and init tty according to input mode */
void ttyInit(void)
{
	static struct termios term;
	if (!tcgetattr(STDIN_FILENO, &term))
		tty = &term;
}

/* ... */
void ttyFoo(void)
{
	static struct termios term;
	term = *tty; /* copy saved tty */
	term.c_lflag &= ~ICANON; /* disable eof */
	term.c_lflag &= ~ICRNL; /* disable CR -> NL */
	if (opt.inputmode == 1 || opt.inputmode == 3)
		term.c_lflag &= ~ECHO; /* disable output of input */
	term.c_cc[VMIN] = 1; /* one character */
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

char ttyGetInput(void)
{
	char c;
	if (tty) ttyFoo();
	c = getchar();
	ttyRestore();
	return c;
}
