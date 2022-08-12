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
*/

#include <stdio.h>
#include <stdlib.h> /* malloc, atoi */
#include <string.h> /* strlen, strchr */
#include <signal.h> /* signal */

#include "bf.h"
#include "errors.h"
#include "tty.h"

#define	ShowCode(c)	if (opt.showinput) fputc(c, stderr);

typedef struct __progr {
	char op;
	int plus; /* +- */
	int step; /* <> */
	int match; /* position of matching bracket (if [ or ]) */
} Progr;

/* SIGINT, SIGTERM, SIGQUIT handler */
void ForceExit(int signum)
{
	ttyRestore(); /* if signal during "," */
	switch(signum)
	{
		case SIGINT:
			puts("Interrupted...");
			break;
		case SIGTERM:
			puts("Terminated...");
			break;
		case SIGQUIT:
			puts("Quit...");
			break;
	}
	exit(errSIGNAL);
}

#if 0
/* efficienter(?) FindMatchingBrackets, but some are missing, 
 * so it's #if 0'ed */
void FindMatchingBrackets(void)
{
	typedef struct __loops {
		size_t begin, level, end;
	} Loops;

	int level = 0, /* bracket level */
	    b = 0, /* bracket index */
	    i = 0; /* program index */
	Progr *px;
	Loops *loops; /* helping structure */
	loops = (Loops*)calloc(brackets, sizeof(Loops));
	DoAndErr(!loops, calloc, errMEMORY);
	
	for (i = 0; i < psize; i++)
	{
		px = program + i;
		if (px->op == '[')
		{
			level++;
			(loops+b)->begin = i;
			(loops+b)->level = level;
			b++;
		} else
		if (px->op == ']')
		{
			level--;
			if (level < 0)
				ErrorMsg(errBRACKET, "Unmatching ] bracket.");
		}
	}
	if (level)
		ErrorMsg(errBRACKET, "Unbalanced brackets. Too many [s.");

	for (b=brackets-1 /* equal --b */; b>=0; b--)
	{
		Loops *lx = loops+b;

		if ((b == brackets-1) || (lx->level >= (lx+1)->level))
		{
			i = lx->begin;
		} else
		if (lx->level < (lx+1)->level)
		{
			int j;
			for (j = 2; (lx->level < (lx+j)->level) && (b+j < brackets); j++);
			i = (lx+j-1)->end;
		}
		for (px = program+i+1; px->op != ']' ; px++);
		lx->end = px - program;
	}

	/* now write it into program */
	for (b = 0; b < brackets; b++)
	{
		(program+((loops+b)->begin))->match = (loops+b)->end;
		(program+((loops+b)->end))->match = (loops+b)->begin;
	}

	free(loops);
}
#endif

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
				ErrorMsg(errBRACKET, "Unbalanced brackets.");
			px->match = j;
			(program+j)->match = i;
		}
	}
}


/* read input file and write it into <program>, return size of program */
size_t ReadProgram(Progr **program)
{
	size_t psize = 0; 
	FILE *fp;
	int i = 0, read = 0;
	char fbuf[BUFSIZ], *vbuf = NULL, *c;
	char oldc = 0;

	/* file opening banana */
	fp = fopen(opt.filename, "rb");
	DoAndErr(!fp, fopen, errFILE); 

	/* read file */
	while (!feof(fp))
	{
		i = fread((char*)fbuf, 1, sizeof(fbuf), fp);
		vbuf = realloc(vbuf, read+=i);
		DoAndErr(!vbuf, realloc, errMEMORY);
		memcpy(vbuf+read-i, fbuf, i);
	}
	fclose(fp);
	
	/* determine program size */
	for(c = vbuf; c < vbuf+read; c++)
	{
		if (strchr("+-<>.,[]", *c))
		{
			if ((!oldc) ||
			    (!strchr("<>", oldc) && !strchr("+-<>", *c)) ||
			    (strchr("<>", oldc) && !strchr("<>", *c)))
				psize++;
#if 0
			if (*c=='[')
				brackets++;
#endif
			oldc = *c;
		} 
	}
	
	/* alloc *zeroed* memory for program */
	*program = (Progr*)calloc(psize, sizeof(Progr));
	DoAndErr(!*program, calloc, errMEMORY);

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

char GetInput(void)
{
	char c;
	if (!opt.inputmode)
		c = getchar();
	else
		c = ttyGetInput();
	if (opt.null && (c == '\n')) /* translate \n to \0? */
		c = 0;
	return c;
}

void Interprete(Progr *program, size_t psize)
{
	register char *array; /* the Brainfuck array */
	register char *p; /* and the array pointer */
	register int i = 0; /* program position */
	
	/* reset array and pointer */
	array = calloc(opt.cells+1, sizeof(char));
	DoAndErr(!array, malloc, errMEMORY);
	p = array;
	
	while (i < psize)
	{
		register Progr px = *(program + i);
		if (px.op)
		{
			ShowCode(px.op);
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
					fflush(stdout);
					break;
				case ',':
					*p = GetInput();
					break;
			}
		}
		if (px.plus) /* do +- */
		{
			ShowCode(px.plus > 0 ?'+':'-');
			if (!opt.wraparound)
			{
				if (*p+px.plus > 255)
					ErrorMsg(errWRAPAROUND, "Out of range! You wanted to '+' a 0xFF byte. See -w option.");
				else 
				if(*p+px.plus < 0)
					ErrorMsg(errWRAPAROUND, "Out of range! You wanted to '-' a 0x00 byte. See -w option.");
			}
			*p += px.plus;
		}
		if (px.step) /* do <> */
		{
			ShowCode(px.step > 0 ?'>':'<');
			if (p+px.step > array+opt.cells)
				ErrorMsg(errOUTOFRANGE, "Out of range! You "
				       "wanted to '>' beyond the last cell."
				       "See -c option.");
			else
			if (p+px.step < array)
				ErrorMsg(errOUTOFRANGE, "Out of range! You"
				    "wanted to '<' below the first cell.\n"
				    "To solve, add some '>'s at the "
				    "beginning, for example.");
			p += px.step;
		}
		i++;
	}
	free(array); /* cleanup */
}

/* display usage information */
void Usage(char *bin)
{
	printf(	"bf - a Brainfuck interpreter       version %s\n", VERSION);
	puts(	"(C) 2003, 2004, Stephan Beyer, GPL, s-beyer@gmx.net\n\n"
		"Usage information: ");
	printf(	"\t%s [-h] [options] inputfile\n\n", bin);
	puts(	"Available options:");
	printf(	"\t-c<num>   specify number of cells [%d]\n", opt.cells);
	printf(	"\t-i        show used code input (stderr)\n"
		"\t-n        translate input: 10 (\\n) to 0\n"
		"\t-w        disallow decrementing 0 and incrementing 255\n"
		"\t-,<mode>  set input mode: 0-4 [%d]\n", opt.inputmode);
	puts(	"\nSee the bf(1) manpage for more information.\n"
		"Have fun!");
}

void HandleOptions(int c, char **v)
/* too lazy to use getopt this time ;) */
{
	int i;
	/* setting defaults */
	opt.showinput = 0;
	opt.cells = CELLS;
	opt.inputmode = 0;
	opt.null = 0;
	opt.wraparound = 1; /* allow by default */
	
	if (c <= 1) /* no filename given */
	{
		Usage(*v);
		ErrorMsg(errOPT, "No input file given.");
	}
        char help[]="-h";
	if (!strcmp(*(v+1), help)) /* is first argument '-h'? */
	{
		Usage(*v);
		exit(0);
	}
	
	for(i = 1; i < c-1; i++) /* options? */
	{
		if ((**(v+i) == '-')
		 && (strlen(*(v+i)) >= 2))
		{
			switch(*(*(v+i)+1))
			{
				case 'c':
					opt.cells = atoi(*(v+i)+2);
					break;
				case 'i':
					opt.showinput = 1;
					break;
				case 'n':
					opt.null = 1;
					break;
				case 'w':
					opt.wraparound = 0;
					break;
				case ',':
					opt.inputmode = atoi(*(v+i)+2);
					break;
				default:
					Usage(*v);
					printf("Unable to handle option %s - ",
						*(v+i));
					ErrorMsg(errOPT, "Unknown option.");
			}
		} else {
			Usage(*v);
			printf("Invalid argument %s - ", *(v+i));
			ErrorMsg(errOPT, "Invalid argument(s).");
		}
	}
	opt.filename = *(v+i); /* last option MUST be filename */
}

#ifdef DEBUGCODE
void debugPrintProgram(Progr *program, size_t psize)
{
	int i,j;
	for (i = 0; i<psize; i++)
	{
		switch((program+i)->op)
		{
			case '[':
				putchar('[');
				printf(" %d ", (program+i)->match);
				break;
			case ']':
				putchar(']');
				printf(" %d ", (program+i)->match);
				break;
			case ',':
				putchar(',');
				break;
			case '.':
				putchar('.');
				break;
		}
		for (j=0; j < abs((program+i)->plus); j++)
			if ((program+i)->plus > 0)
				putchar('+');
			else
				putchar('-');
		for (j=0; j < abs((program+i)->step); j++)
			if ((program+i)->step > 0)
				putchar('>');
			else
				putchar('<');
		putchar('\n');
	}
}
#endif

int main(int argc, char **argv)
{
	Progr* program; /* program structure */
	size_t psize; /* program size */

	HandleOptions(argc, argv);
	psize = ReadProgram(&program);
	FindMatchingBrackets(program, psize);
#ifdef DEBUGCODE
	debugPrintProgram(program, psize);
#endif
	ttyInit();

	/* catch some signals */
	/* TODO use sigaction? */
	signal(SIGINT, ForceExit);
	signal(SIGTERM, ForceExit);
	signal(SIGQUIT, ForceExit);

	/* reading and interpreting */
	Interprete(program, psize);

	/* cleanup */
	free(program);
	return errNO;
}
