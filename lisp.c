#include "lisp.h"

C *fclist;
F *fflist;
C *pdl[PDLSZ];
int pdp;
Temlis temlis;
C *alist[MAXARGS];
int nargs;
C *oblist;

int gcen;
int gcdbg;

void *Atom = (void*)CAR_ATOM;
void *Fixnum = (void*)(CAR_ATOM|CAR_FIX);
void *Flonum = (void*)(CAR_ATOM|CAR_FLO);

/* absence of a value */
C *noval = (C*)~0;

/* some important atoms */
C *pname;
C *apval;
C *expr;
C *subr;
C *fexpr;
C *fsubr;
C *t;
C *quote;
C *label;
C *funarg;
C *lambda;
C *cond;
C *set;
C *setq;
C *go;
C *retrn;

C *star;
C *digits[10];

jmp_buf tljmp;

void dbgprint(C *c);

/* print error and jmp back into toplevel */
void
err(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	longjmp(tljmp, 1);
}

void
panic(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(1);
}

C**
push(C *c)
{
	C **p;
	assert(pdp >= 0 && pdp < PDLSZ);
	p = &pdl[pdp++];
	*p = c;
	return p;
}

C*
pop(void)
{
	assert(pdp > 0 && pdp <= PDLSZ);
	return pdl[--pdp];
}

int
pushargs(void)
{
	int n;
	for(n = 0; n < nargs; n++)
		push(alist[n]);
	nargs = 0;
	return n;
}

void
popargs(int n)
{
	nargs = n;
	for(n = nargs-1; n >= 0; n--)
		alist[n] = pop();
}

C*
cons(void *a, C *d)
{
	C *c;
	if(((P)a & CAR_ATOM) == 0)
		temlis.ca = a;
	temlis.cd = d;
	if(gcen && (fclist == nil || gcdbg))
		gc();
	c = fclist;
	assert(c != nil);
	fclist = fclist->d;
	temlis.ca = nil;
	temlis.cd = nil;
	c->a = a;
	c->d = d;
	return c;
}

F*
consw(word fw)
{
	F *f;
	if(gcen && (fflist == nil || gcdbg))
		gc();
	f = fflist;
	assert(f != nil);
	fflist = fflist->p;
	f->fw = fw;
	return f;
}

C*
mkfix(fixnum fix)
{
	C *c;
	c = cons(Fixnum, nil);
	c->fix = fix;
	return c;
}

C*
mkflo(flonum flo)
{
	C *c;
	c = cons(Flonum, nil);
	c->flo = flo;
	return c;
}

C*
mksubr(C *(*subr)(void), int n)
{
	F nf, sf;
	nf.n = n;
	sf.subr = subr;
	temlis.ca = consw(nf.fw);
	temlis.cd = consw(sf.fw);
	return cons(temlis.ca, temlis.cd);
}

int
atom(C *c)
{
	return c == nil || c->ap & CAR_ATOM;
}

int
fixnump(C *c)
{
	return c != nil && c->ap & CAR_ATOM && c->ap & CAR_FIX;
}

int
flonump(C *c)
{
	return c != nil && c->ap & CAR_ATOM && c->ap & CAR_FLO;
}

int
numberp(C *c)
{
	return c != nil && c->ap & CAR_ATOM && c->ap & CAR_NUM;
}

/* functions for handling pnames */
int
matchpname(C *c, char *name)
{
	int i;
	char *s;
	char c1, c2;

	s = name;
	i = 0;
	for(;;){
		c1 = *s++;
		c2 = c ? c->af->c[i++] : '\0';
		if(i == C2W){
			i = 0;
			c = c->d;
		}
		if(c1 != c2)
			return 0;
		if(c1 == '\0')
			return 1;
	}
}

C*
makepname(char *name)
{
	int i;
	F w;
	char *s;
	C *ret, **next;

	/* TODO: maybe do this elsewhere? */
	ret = cons(nil, nil);
	temlis.pn = ret;
	next = &ret->a;

	/* split up name into full words
	 * and build list structure */
	s = name;
	while(*s != '\0'){
		w.fw = 0;
		for(i = 0; i < C2W; i++){
			if(*s == '\0')
				break;
			w.c[i] = *s++;
		}
		*next = cons(consw(w.fw), nil);
		next = &(*next)->d;
	}
	temlis.pn = nil;
	return ret;
}

C*
prop(C *l, C *p)
{
	for(; l != nil; l = l->d)
		if(l->a == p)
			return l->d;
	return nil;
}

/* returns noval instead of evaluating a function */
C*
sassoc(C *x, C *y)
{
	for(; y != nil; y = y->d)
		if(y->a->a == x)
			return y->a;
	return noval;
}

C*
attrib(C *x, C *e)
{
	for(; x->d != nil; x = x->d);
	x->d = e;
	return e;
}

C*
defprop(C *l, C *p, C *ind)
{
	C *tt;
// TODO: must save here?
	temlis.a = l;
	temlis.b = p;
	temlis.c = ind;
	nargs = 3;
	tt = prop(l, ind);
	if(tt)
		tt->a = p;
	else
		attrib(l, cons(ind, cons(p, nil)));
	temlis.a = nil;
	temlis.b = nil;
	temlis.c = nil;
	return l;
}

C*
nconc(C *x, C *e)
{
	C *m;
	if(x == nil) return e;
	m = x;
	for(; x->d != nil; x = x->d);
	x->d = e;
	return m;
}

C*
pair(C *x, C *y)
{
	C *m, **p;
// TODO: must save here?
	temlis.b = x;
	temlis.c = y;
	assert(temlis.a == nil);
	p = (C**)&temlis.a;
	while(x != nil && y != nil){
		*p = cons(cons(x->a, y->a), nil);
		p = &(*p)->d;
		x = x->d;
		y = y->d;
	}
	if(x != nil || y != nil)
		err("error: pair not same length");
	m = temlis.a;
	temlis.a = nil;
	temlis.b = nil;
	temlis.c = nil;
	return m;
}

C*
intern(char *name)
{
	C *c;
	C *pn;
	for(c = oblist; c; c = c->d){
		if(numberp(c->a))
			continue;
		pn = prop(c->a->d, pname);
		if(pn == nil)
			continue;
		if(matchpname(pn->a, name))
			return c->a;
	}
	c = cons(Atom,
		cons(pname,
		makepname(name)));
	oblist = cons(c, oblist);
	return c;
}

/*
 * output
 */

void
printpname(C *c)
{
	word fw;
	int i;
	for(c = c->a; c != nil; c = c->d){
		fw = ((F*)c->a)->fw;
		for(i = 0; i < C2W; i++){
			putchar(fw&0xFF);
			fw >>= 8;
		}
	}
}

void
printatom(C *c)
{
	if(c == nil)
		printf("NIL");
	else if(fixnump(c))
#ifdef LISP32
		printf("%d", c->fix);
#else
		printf("%ld", c->fix);
#endif
	else if(flonump(c))
		printf("%f", c->flo);
	else{
		assert(atom(c));
		for(; c != nil; c = c->d)
			if(c->a == pname){
				printpname(c->d);
				return;
			}
		printf("%%ATOM%%");
	}
}

void
print(C *c)
{
	int fst;
	if(atom(c))
		printatom(c);
	else{
		printf("(");
		fst = 1;
		for(; c != nil; c = c->d){
			if(atom(c)){
				printf(" . ");
				printatom(c);
				break;
			}
			if(!fst)
				printf(" ");
			print(c->a);
			fst = 0;
		}
		printf(")");
	}
}

void
dbgprint(C *c)
{
	for(; c != nil; c = c->d)
		printf("L %p %p\n", c->a, c->d);
}

/*
 * input
 */

int nextc;

static int
chsp(void)
{
	int c;
	if(nextc){
		c = nextc;
		nextc = 0;
		return c;
	}
	c = getc(stdin);
	if(isspace(c))
		c = ' ';
	return c;
}

static int
ch(void)
{
	int c;
	while(c = chsp(), c == ' ');
	return c;
}

C*
readnum(void)
{
	int c;
	int type;
	fixnum oct;
	fixnum dec;
	flonum flo, fract, div;
	int sign;
	int nchar;

	sign = 1;
	type = 0;	/* octal */
	oct = 0;
	dec = 0;
	flo = 0.0;
	fract = 0.0;
	div = 10.0;
	nchar = 0;

	nextc = chsp();
	if(nextc == '-' || nextc == '+'){
		sign = nextc == '-' ? -1 : 1;
		nextc = 0;
		nchar++;
	}

	while(c = chsp(), strchr(" ()", c) == nil && c != EOF){
		if(c >= '0' && c <= '9'){
			if(type == 0){
				oct = oct*8 + c-'0';
				dec = dec*10 + c-'0';
				flo = flo*10.0 + c-'0';
			}else{
				type = 2;	/* float */
				fract += (c-'0')/div;
				div *= 10.0;
			}
		}else if(c == '.' && type == 0){
			type = 1;	/* decimal */
		}else
			err("error: number format");
		nchar++;
	}
	nextc = c;
	if(nchar == 1)
		return digits[dec];
	if(type == 0)
		return mkfix(sign*oct);
	if(type == 1)
		return mkfix(sign*dec);
	return mkflo(sign*(flo+fract));
}

C*
readatom(void)
{
	int c;
	char buf[128], *p;
	int spec, lc;

	p = buf;
	spec = 0;
	lc = 1;
	while(c = chsp(), c != EOF){
		if(!spec && strchr(" ()", c)){
			nextc = c;
			break;
		}
		if(c == '|'){
			lc = 0;
			spec = !spec;
			continue;
		}
		*p++ = c;
	}
	*p++ = '\0';
	if(lc)
		for(p = buf; *p; p++)
			*p = toupper(*p);
	if(strcmp(buf, "NIL") == 0)
		return nil;
	return intern(buf);
}

C *readsxp(void);

C*
readlist(void)
{
	int first;
	int c;
	C **p;

	first = 1;
	p = push(nil);
	while(c = ch(), c != ')'){
		/* TODO: only valid when next letter is space */
		if(c == '.'){
			if(first)
				err("error: unexpected '.'");
			*p = readsxp();
			if(c = ch(), c != ')')
				err("error: expected ')' (got %c)", c);
			break;
		}
		nextc = c;
		*p = cons(readsxp(), nil);
		p = &(*p)->d;
		first = 0;
	}
	return pop();
}

C*
readsxp(void)
{
	int c;
	c = ch();
	if(c == EOF)
		return noval;
	if(c == ')')
		err("error: unexpected ')'");
	if(c == '(')
		return readlist();
	nextc = c;
	if(strchr("0123456789+-", c))
		return readnum();
	return readatom();
}

/*
 * Eval Apply
 */

void
spread(C *l)
{
	nargs = 0;
	for(; l != nil; l = l->d){
		if(nargs >= 20)
			err("error: arg list too long");
		alist[nargs++] = l->a;
	}
}

C*
evcon(C *c, C *a)
{
	C *tt;
	int spdp;
	spdp = pdp;
	push(c);
	push(a);
	for(; c != nil; c = c->d){
		tt = eval(c->a->a, a);
		if(tt != nil){
			pdp = spdp;
			return eval(c->a->d->a, a);
		}
	}
	err("error: no cond clause");
	return nil;	/* make compiler happy */
}

C*
applysubr(C *subr, C *args)
{
	C *tt;
	int n;
	n = pushargs();
	spread(args);
	if(subr->af->n != nargs)
		err("error: arg count (expected %d, got %d)",
			subr->af->n, nargs);
	tt = subr->df->subr();
	popargs(n);
	return tt;
}

C*
eval(C *form, C *a)
{
	C *tt, *arg;
	int spdp;
	int n;

tail:
	if(form == nil)
		return nil;
	if(numberp(form))
		return form;
	if(atom(form)){
		if(tt = prop(form->d, apval), tt != nil)
			return tt->a;
		if(tt = sassoc(form, a), tt == noval)
			err("error: no value");
		return tt->d;
	}
	if(form->a == cond)
		return evcon(form->d, a);
	spdp = pdp;
	push(form);
	push(a);
	if(atom(form->a)){
		if(form->a == nil || numberp(form->a))
			err("error: no function");
		if(tt = prop(form->a->d, expr), tt != nil){
			arg = evlis(form->d, a);
			pdp = spdp;
			return apply(tt->a, arg, a);
		}
		if(tt = prop(form->a->d, fexpr), tt != nil){
			arg = cons(form->d, cons(a, nil));
			pdp = spdp;
			return apply(tt->a, arg, a);
		}
		if(tt = prop(form->a->d, subr), tt != nil){
			arg = evlis(form->d, a);
			pdp = spdp;
			return applysubr(tt->a, arg);
		}
		if(tt = prop(form->a->d, fsubr), tt != nil){
			pdp = spdp;
			n = pushargs();
			alist[0] = form->d;
			alist[1] = a;
			nargs = 2;
			tt = tt->af->subr();
			popargs(n);
			return tt;
		}
		if(tt = sassoc(form->a, a), tt == noval)
			err("error: no function");
		form = cons(tt->d, form->d);
		pdp = spdp;
		goto tail;
	}
	arg = evlis(form->d, a);
	pdp = spdp;
	return apply(form->a, arg, a);
}

C*
evalquote(C *fn, C *args)
{
	if(args != nil && (atom(args) || numberp(args)))
		err("error: no list");
	if(fn != nil && atom(fn) && !numberp(fn) &&
	   (prop(fn->d, fexpr) != nil ||
	    prop(fn->d, fsubr) != nil))
		return eval(cons(fn, args), nil);
	else
		return apply(fn, args, nil);
}

C*
evlis(C *m, C *a)
{
	C **p;
	int spdp;

	p = push(nil);
	spdp = pdp;
	push(m);
	push(a);
	for(; m != nil; m = m->d){
		*p = cons(eval(m->a, a), nil);
		p = &(*p)->d;
	}
	pdp = spdp;
	return pop();
}

C*
apply(C *fn, C *args, C *a)
{
	C *tt;
	int spdp;

	if(atom(fn)){
		if(fn == nil || numberp(fn))
			err("error: no function");
		if(tt = prop(fn->d, expr), tt != nil)
			return apply(tt->a, args, a);
		if(tt = prop(fn->d, subr), tt != nil)
			return applysubr(tt->a, args);
		if(tt = sassoc(fn, a), tt == noval)
			err("error: no function");
		return apply(tt->d, args, a);
	}
	spdp = pdp;
	push(fn);
	push(args);
	push(a);
	if(fn->a == label){
		tt = cons(fn->d->a, fn->d->d->a);
		a = cons(tt, a);
		pdp = spdp;
		return apply(fn->d->d->a, args, a);
	}
	if(fn->a == funarg){
		pdp = spdp;
		return apply(fn->d->a, args, fn->d->d->a);
	}
	if(fn->a == lambda){
		args = pair(fn->d->a, args);
		pdp = spdp;
		return eval(fn->d->d->a, nconc(args, a));
	}
	fn = eval(fn, a);
	pdp = spdp;
	return apply(fn, args, a);
}


/*
 * top level
 */

void
init(void)
{
	int i;

	gc();

	/* init oblist so we can use intern */
	pname = cons(Atom, nil);
	pname->d = cons(pname, makepname("PNAME"));
	oblist = cons(pname, nil);

	/* Now enable GC */
	gcen = 1;

	t = intern("T");
	apval = intern("APVAL");
	subr = intern("SUBR");
	fsubr = intern("FSUBR");
	expr = intern("EXPR");
	fexpr = intern("FEXPR");
	quote = intern("QUOTE");
	label = intern("LABEL");
	funarg = intern("FUNARG");
	lambda = intern("LAMBDA");
	cond = intern("COND");
	set = intern("SET");
	setq = intern("SETQ");
	go = intern("GO");
	retrn = intern("RETURN");

	for(i = 0; i < 10; i++){
		digits[i] = mkfix(i);
		oblist = cons(digits[i], oblist);
	}

	initsubr();

	star = intern("*");
}

void
eval_repl(void)
{
	C *e;

	defprop(star, star, apval);
	for(;;){
		print(eval(star, nil));
		printf("\n");
		e = readsxp();
		if(e == noval)
			return;
		e = eval(e, nil);
		if(e == noval)
			defprop(star, star, apval);
		else
			defprop(star, e, apval);
	}
}

void
evalquote_repl(void)
{
	C *e, *fn, *args;
	int spdp;

	defprop(star, star, apval);
	for(;;){
		print(eval(star, nil));
		printf("\n");
		fn = readsxp();
		if(e == noval)
			return;
		spdp = pdp;
		push(fn);
		args = readsxp();
		if(args == noval)
			return;
		pdp = spdp;
		e = evalquote(fn, args);
		if(e == noval)
			defprop(star, star, apval);
		else
			defprop(star, e, apval);
	}
}

void
eval_file(void)
{
	C *e;
	for(;;){
		e = readsxp();
		if(e == noval)
			return;
		eval(e, nil);
	}
}

void
load(char *filename)
{
	FILE *oldin, *f;
	f = fopen(filename, "r");
	if(f == nil)
		return;
	oldin = stdin;
	stdin = f;
	if(setjmp(tljmp))
		exit(1);
	eval_file();
	stdin = oldin;
	fclose(f);
}

int
main(int argc, char *argv[])
{
#ifdef LISP32
	/* only works on 32 bits */
	assert(sizeof(void*) == 4);
#else
	/* only works on 64 bits */
	assert(sizeof(void*) == 8);
#endif

	int evq = 0;
	if(argc > 1)
		if(strcmp(argv[1], "-q") == 0)
			evq++;

	init();

	load("lib.l");

//	print(oblist);
//	printf("\n");

	if(setjmp(tljmp))
		printf("→\n");
	pdp = 0;
	memset(&temlis, 0, sizeof(temlis));

	if(evq)
		evalquote_repl();
	else
		eval_repl();

	return 0;
}
