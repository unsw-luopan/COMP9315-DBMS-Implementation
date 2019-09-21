// reln.h ... interface to functions on Relations
// part of Multi-attribute Linear-hashed Files
// See reln.c for details on Reln type and functions
// Last modified by John Shepherd, July 2019

#ifndef RELN_H
#define RELN_H 1

typedef struct RelnRep *Reln;

#include "defs.h"
#include "tuple.h"
#include "page.h"
#include "chvec.h"

Status newRelation(char *name, Count nattr, Count npages, Count d, char *cv);
Reln openRelation(char *name, char *mode);
void closeRelation(Reln r);
Bool existsRelation(char *name);
void checkSplit(Reln r);
PageID addToRelation(Reln r,Tuple t);
void splitReln(Reln r);
Tuple read_next_tuple(FILE *in, PageID pid, Offset curtup);
PageID add_to_split_page(Reln r, Tuple t,PageID p);
FILE *dataFile(Reln r);
FILE *ovflowFile(Reln r);
Count nattrs(Reln r);
Count npages(Reln r);
Count depth(Reln r);
Count splitp(Reln r);
ChVecItem *chvec(Reln r);
void relationStats(Reln r);

#endif
