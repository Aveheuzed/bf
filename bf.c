/* bf.c
******************************************************************************

   Yet another Brainfuck interpreter

   Author: Stephan Beyer

   Copyright (C) GPL, 2003, 2004 Stephan Beyer - s-beyer@gmx.net

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

	 Further modified (Arduino port) by Alexis Masson.

******************************************************************************
*/

#include <stdio.h>
#include <stdlib.h> /* malloc, atoi */
#include <string.h> /* strlen, strchr */

#include "errors.h"
#include "tty.h"
#include "wrapper.h"

// adjust this value depending on how much you need
// vs how much RAM the bf text takes (a Mega rev3 has 8Kib RAM overall)
#define CELLS 1024

typedef struct __progr {
	char op;
	int plus; /* +- */
	int step; /* <> */
	int match; /* position of matching bracket (if [ or ]) */
} Progr;

/* find matching brackets (loop) */
void FindMatchingBrackets(Progr *program, size_t psize)
{
	int i;
	for (i = 0; i < psize; i++)
	{
		register Progr *px = program + i;
		if(px->op == '[')
		{
			int j, l=0;
			for(j=i+1; j<psize; j++)
			{
				if((program+j)->op == '[')
					l++;
				if((program+j)->op == ']')
				{
					if (l)
						l--;
					else
						break;
				}
			}
			if (l)
				ErrorMsg("Unbalanced brackets.");
			px->match = j;
			(program+j)->match = i;
		}
	}
}


/* read input file and write it into <program>, return size of program */
size_t ReadProgram(Progr **program)
{
	size_t psize = 0;
	int i = 0, read = 0;
	char fbuf[BUFSIZ], *vbuf = NULL, *c;
	char oldc = 0;

	/* file opening banana */
	SDopen();

	/* read file */
	while (!SDeof())
	{
		i = SDread((char*)fbuf, 1, sizeof(fbuf));
		vbuf = realloc(vbuf, read+=i);
		DoAndErr(!vbuf, "OOM - bf program too big to fit in RAM");
		memcpy(vbuf+read-i, fbuf, i);
	}

	SDclose();
	/* determine program size */
	for(c = vbuf; c < vbuf+read; c++)
	{
		if (strchr("+-<>.,[]", *c))
		{
			if ((!oldc) ||
			    (!strchr("<>", oldc) && !strchr("+-<>", *c)) ||
			    (strchr("<>", oldc) && !strchr("<>", *c)))
				psize++;
			oldc = *c;
		}
	}

	/* alloc *zeroed* memory for program */
	*program = (Progr*)calloc(psize, sizeof(Progr));
	DoAndErr(!*program, "OOM - bf program too big to fit in RAM");

	oldc = 'X';
	i = 0;

	/* read vbuf again and put into program */
	for(c = vbuf; c < vbuf+read; c++)
	{
		if (strchr("+-<>.,[]", *c))
		{
			Progr *x;
			if ((!strchr("<>X", oldc) && !strchr("+-<>", *c)) ||
			    (strchr("<>", oldc) && !strchr("<>", *c)))
				i++;
			x = *program+i;

			switch(*c)
			{
				case '+':
					x->plus++;
					break;
				case '-':
					x->plus--;
					break;
				case '>':
					x->step++;
					break;
				case '<':
					x->step--;
					break;
				default:
					x->op = *c;
					break;
			}
			oldc = *c;
		}
	}
	free(vbuf); /* cleanup */
	return psize;
}

void Interprete(Progr *program, size_t psize)
{
	register char *array; /* the Brainfuck array */
	register char *p; /* and the array pointer */
	register int i = 0; /* program position */

	/* reset array and pointer */
	array = calloc(CELLS+1, sizeof(char));
	DoAndErr(!array, "OOM - RAM allocation failed for the bf tape");
	p = array;

	while (i < psize)
	{
		register Progr px = *(program + i);
		if (px.op)
		{
			switch (px.op)
			{
				case '[':
					if (!*p)
					{
						i = px.match;
						px = *(program+i);
					}
					break;
				case ']':
					if (*p) /* repeat */
					{
						i = px.match;
						px = *(program+i);
					}
					break;
				case '.':
					putchar(*p);
					break;
				case ',':
					*p = ttyGetInput();
					break;
			}
		}
		if (px.plus) /* do +- */
		{
			*p += px.plus;
		}
		if (px.step) /* do <> */
		{
			if (p+px.step > array+CELLS)
				ErrorMsg("Out of range! You "
				       "wanted to '>' beyond the last cell.");
			else
			if (p+px.step < array)
				ErrorMsg("Out of range! You"
				    "wanted to '<' below the first cell.\n"
				    "To solve, add some '>'s at the "
				    "beginning, for example.");
			p += px.step;
		}
		i++;
	}
	free(array); /* cleanup */
}

void setup()
{
	Progr* program; /* program structure */
	size_t psize; /* program size */

  ttyInit();

	psize = ReadProgram(&program);
	FindMatchingBrackets(program, psize);

	/* reading and interpreting */
	Interprete(program, psize);

	/* cleanup */
	free(program);
}

void loop(){
	exit(0);
}
