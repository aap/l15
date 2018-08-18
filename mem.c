#include "lisp.h"

C cstore[NUMCONS];
F fstore[NUMFW];
word fmark[NUMFW/B2W];

void
mark(C *c)
{
	C *a;
	F *f;
	int n;

tail:
	if(c == nil)
		return;

	/* Mark full word */
	f = (F*)c;
	if(f >= &fstore[0] && f < &fstore[NUMFW]){
		n = f - fstore;
		fmark[n/B2W] |= 1UL << n%B2W;
		return;
	}

	/* Must be a cons cell */
	if(c >= &cstore[0] && c < &cstore[NUMCONS]){
		if(c->ap & CAR_MARK)
			return;
		a = c->a;
		c->ap |= CAR_MARK;
		if(c->ap & CAR_ATOM){
			if(c->ap & CAR_NUM)
				return;
		}else
			mark(a);
		c = c->d;
		goto tail;
	}

	panic("invalid ptr: %p\n", c);
}

void
gc(void)
{
	int i, j;
	C *c, **cp;
	F *f;
	word m;
	int nc, nf;

	/* Mark */
	mark(oblist);
	for(i = 0; i < pdp; i++)
		mark(pdl[i]);
	for(cp = (C**)&temlis; cp < (C**)(&temlis+1); cp++)
		mark(*cp);
	for(i = 0; i < nargs; i++)
		mark(alist[i]);

	/* Sweep */
	fclist = nil;
	nc = 0;
	for(c = cstore; c < &cstore[NUMCONS]; c++){
		if(c->ap & CAR_MARK)
			c->ap &= ~CAR_MARK;
		else{
			c->a = nil;
			c->d = fclist;
			fclist = c;
			nc++;
		}
	}

	fflist = nil;
	f = fstore;
	nf = 0;
	for(i = 0; i < NUMFW/B2W; i++){
		m = fmark[i];
		fmark[i] = 0;
		for(j = 0; j < B2W; j++){
			if(!(m&1)){
				f->p = fflist;
				fflist = f;
				nf++;
			}
			m >>= 1;
			f++;
		}
	}

//	printf("reclaimed: %d %d\n", nc, nf);
}
