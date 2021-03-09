#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <assert.h>

#define nil NULL

#if UINTPTR_MAX == 0xFFFFFFFF
#define LISP32
#endif

/* basic data types */
#ifdef LISP32
/* assume we're running on 32 bits!!! */
typedef uintptr_t P;
typedef uint32_t word;
typedef int32_t fixnum;
typedef float flonum;
enum
{
	B2W = 32,	/* bits per word */
	C2W = B2W/8,	/* character per word */
};
#else
/* assume we're running on 64 bits!!! */
typedef uintptr_t P;
typedef uint64_t word;
typedef int64_t fixnum;
typedef double flonum;
enum
{
	B2W = 64,	/* bits per word */
	C2W = B2W/8,	/* character per word */
};
#endif


/* static storage sizes */
enum
{
	NUMCONS = 32*1024,
	NUMFW = 32*1024,
	PDLSZ = 1024,
	MAXARGS = 20,
};


typedef struct C C;
typedef union F F;

/* A cons cell */
struct C
{
	union {
		C *a;
		F *af;
		P ap;
	};
	union {
		C *d;
		F *df;
		P dp;

		fixnum fix;
		flonum flo;
	};
};

/* CAR bits */
enum
{
	CAR_MARK = 1,
	CAR_ATOM = 2,
	CAR_FIX  = 4,
	CAR_FLO  = 8,
	CAR_NUM  = CAR_FIX | CAR_FLO
};


/* A full word */
union F
{
	word fw;
	char c[C2W];
	F *p;
	fixnum n;
	C *(*subr)(void);
};


/* free storage */
extern C *fclist;
extern F *fflist;

/* push down list */
extern C *pdl[PDLSZ];
extern int pdp;

/* Temporary variables automatically saved */
typedef struct Temlis Temlis;
struct Temlis
{
	/* temp */
	void *a, *b, *c;
	/* arguments to cons */
	void *ca;
	void *cd;
	/* pname */
	void *pn;
	/* eval */
	void *e;
};
extern Temlis temlis;
extern C *alist[MAXARGS];
extern int nargs;
extern C *oblist;

extern C *noval;
extern C *t;
extern C *apval;
extern C *expr;
extern C *subr;
extern C *fexpr;
extern C *fsubr;
extern C *funarg;
extern C *cond;
extern C *set;
extern C *setq;
extern C *go;
extern C *retrn;

void err(char *fmt, ...);
void panic(char *fmt, ...);
C **push(C *c);
C *pop(void);

C *cons(void *a, C *d);
F *consw(word fw);
C *mkfix(fixnum fix);
C *mkflo(flonum flo);
C *mksubr(C *(*subr)(void), int n);
int atom(C *c);
int fixnump(C *c);
int flonump(C *c);
int numberp(C *c);
C *sassoc(C *x, C *y);
C *defprop(C *l, C *p, C *ind);
C *pair(C *x, C *y);
C *intern(char *name);
C *readsxp(void);
void print(C *c);
void printatom(C *c);
C *eval(C *form, C *a);
C *evlis(C *m, C *a);
C *apply(C *fn, C *args, C *a);

void gc(void);

void initsubr();
