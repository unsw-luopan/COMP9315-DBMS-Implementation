// query.c ... query scan functions
// part of Multi-attribute Linear-hashed Files
// Manage creating and using Query objects
// This is group work, aims for task2
// Last modified by
// Pan Luo: z5192086,
// Zhidong Luo: z5181142
// Shuxiang Zou: z5187969
// Aug, 2019

#include "defs.h"
#include "query.h"
#include "reln.h"
#include "tuple.h"

#include "hash.h"

// A suggestion ... you can change however you like

struct QueryRep {
	Reln    rel;       // need to remember Relation info
	Bits    known;     // the hash value from MAH
	Bits    unknown;   // the unknown bits from MAH
	PageID  curpage;   // current page in scan
	int     is_ovflow; // are we in the overflow pages?
	Offset  curtup;    // offset of current tuple within page
	Bits stbucket; // start bucket value
	Tuple qstring;
	int depth;
	Bits nBits;
	Bits unBits;
	Count count;
	Count ntup;
	Bits option;
};

// check whether the querry is valid, for example, whether the number of attributes is equal to that in db
int checkQuery(Reln r, char *q) {
    if (*q == '\0') return 0;
    char *c;
    int nattr = 1;
    for (c = q; *c != '\0'; c++)
        if (*c == ',') nattr++;
    return (nattr == nattrs(r));
}

//check the bits in querry, whether they are known or unknown
void checkBits(Reln r,char *q,Count nattributes,Bits knownBits,Bits unknownBits,Query new){
  int attrknow[nattributes];
  char *attrpt[nattributes];
  tupleVals(q, attrpt);
  ChVecItem *cv = chvec(r);
  Bits hash[nattributes];
  Bits m = 0x00000000;
  for (int i = 0; i < nattributes; i++) {
  attrknow[i] = strcmp(attrpt[i], "?");
  hash[i] = 0x00000000;
  if (attrknow[i])
  hash[i] = hash_any((unsigned char *) attrpt[i], strlen(attrpt[i]));
}
  for (int i = 0; i < 32; ++i) {
  m = 0x00000000;
  if (!attrknow[cv[i].att]) unknownBits = setBit(unknownBits, i);
  m = setBit(m, cv[i].bit);
  if ((hash[cv[i].att] & m) == 0) knownBits = unsetBit(knownBits, i);
 }
 //printf("%u\n\n\n",knownBits);
  new->nBits=knownBits;
  new->unBits=unknownBits;
  //printf("%u\n\n\n",unknownBits);
}

// take a query string (e.g. "1234,?,abc,?")
// set up a QueryRep object for the scan
Query startQuery(Reln r, char *q)
{
  Query new = malloc(sizeof(struct QueryRep));
  assert(new != NULL);
  new->rel = r;
  new->is_ovflow = 0;
  new->qstring = copyString(q);
  new->curtup = 0;
  new->ntup = 0;
  int min=0x00000000;
  int max=0xFFFFFFFF;
  new->option = min;
  Count nvals = nattrs(r);
  Bits nknow = min;
  Bits qhash = max;
  Bits mask = min;
  if (!checkQuery(r, q)) return NULL;
  checkBits(r,q,nvals,qhash,nknow,new);
  qhash=new->nBits;

  nknow=new->unBits;
  //printf("%u\n\n\n",qhash);
  new->known = qhash;
  new->unknown = nknow;
  mask = min;
  int d = depth(r);
  for (int i = 0; i < d; ++i) mask = setBit(mask, i);

  PageID id = qhash & mask;
  new->depth = depth(r);
  if (id < splitp(r)) {
    mask = setBit(mask, d);
    id = qhash & mask;
    new->depth++;

}
  int count = 0;
  for (int i = 0; i < new->depth; i++) if (nknow & (1 << i)) count++;
  new->count = count;
  new->stbucket = id;
  new->curpage = id;
  return new;
}

//check whether cur page has overflow pages
Bool hasOverFlow(Query q,Page p){
	if (pageOvflow(p) != NO_PAGE) {
		return TRUE;
	}else{ return FALSE;}
}


Tuple getNextTuple(Query q) {
  // Partial algorithm:
	// if (more tuples in current page)
	//    get next matching tuple from current page
	// else if (current page has overflow)
	//    move to overflow page
	//    grab first matching tuple from page
	// else
	//    move to "next" bucket
	//    grab first matching tuple from data page
	// endif
	// if (current page has no matching tuples)
	//    go to next page (try again)
	// endif
  Reln re = q->rel;
  while (TRUE) {
    PageID pid = q->curpage;
    FILE *file;

    // initialise the page, file, and state of page overflow
        Count numberOfTuples = q->ntup;
    numberOfTuples = numberOfTuples;
    Offset offsetOfTuples = q->curtup;
    offsetOfTuples = offsetOfTuples;
	int judgeOverFlow = q->is_ovflow;
    if (judgeOverFlow) file = ovflowFile(re);
    else file = dataFile(re);
    Page page = getPage(file, pid);
   //judge if there are more tuples in the page, get next matching tuple from current page
    while (q->ntup < pageNTuples(page)) {
  Tuple tmp = read_next_tuple(file, q->curpage, q->curtup);
q->ntup++;
q->curtup += strlen(tmp) + 1;
if (tupleMatch(re, q->qstring, tmp)) return tmp;
free(tmp);
}
    Bool po_rel = hasOverFlow(q,page);

    if (po_rel == TRUE) { // if (current page has overflow)      move to overflow page
      q->curpage = pageOvflow(page);
      numberOfTuples = 0;
      offsetOfTuples = 0;
      judgeOverFlow = 1;
      continue;
    } else { // go to next page
      Bits unKnownBits = q->unknown;
      Bits optionBits = q->option;
      optionBits++;      
      int countBits = q->count;
      if (optionBits >= (1 << countBits)) break; //move left for 1
      q->option = optionBits;
      int currentOffset = 0;
      Bits mask = 0;
      int tmp;
      for (tmp=0 ; tmp < countBits; ++tmp) {
        while (!(unKnownBits & (setBit(0, currentOffset)))) currentOffset++;
        if (optionBits & setBit(0, tmp)) mask = setBit(mask, currentOffset);
        currentOffset++;
      }
      PageID id = q->stbucket | mask;
      if(id > npages(re)-1) break;
      q->curpage = id;
      q->ntup = 0;
      q->curtup = 0;
      judgeOverFlow = 0;
    }
  }
	return NULL;
}
void closeQuery(Query q)
{
  free(q); //free the query memory
}

