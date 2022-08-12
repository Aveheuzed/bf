/* bf.h */
#ifndef BF_H
#define BF_H

#define VERSION "20041219"
#define CELLS 9999

/* options */
typedef struct {
	char *filename;
	int cells;			/* -c */
	unsigned short inputmode;	/* -, */
	unsigned showinput : 1;		/* -i */
	unsigned wraparound : 1;	/* -w */
	unsigned null : 1;		/* -n */
} Opt;
Opt opt; /* bf options */

#endif
