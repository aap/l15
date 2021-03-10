#include "lisp.h"
#include <limits.h>
#include <float.h>
#include <math.h>

int
floeq(flonum x, flonum y)
{
	return fabs(x-y) < 0.000003;
}

int
equal(C *a, C *b)
{
	if(atom(a) != atom(b))
		return 0;
	if(atom(a)){
		if(fixnump(a))
			return fixnump(b) &&
				a->fix == b->fix;
		if(flonump(a))
			return flonump(b) &&
				floeq(a->flo, b->flo);
		return a == b;
	}
	return equal(a->a, b->a)
		&& equal(a->d, b->d);
}

/* this is a bit ugly... */
int
getnumcase(void)
{
	int type;
	type = 0;
	if(fixnump(alist[0]))
		;
	else if(flonump(alist[0]))
		type |= 1;
	else
		type |= ~0;
	if(fixnump(alist[1]))
		;
	else if(flonump(alist[1]))
		type |= 2;
	else
		type |= ~0;
	return type;
}

/* syntax */

C *quote_fsubr(void){
	if(alist[0] == nil)
		err("error: arg count");
	return alist[0]->a;
}
C *function_fsubr(void){
	if(alist[0] == nil)
		err("error: arg count");
	return cons(funarg, cons(alist[0]->a, cons(alist[1], nil)));
}

/* elementary functions */

C *car(C *pair){
	if(pair == nil)
		return nil;
	if(numberp(pair))
		err("error: not a pair");
	return pair->a;
}
C *cdr(C *pair){
	if(pair == nil)
		return nil;
	if(numberp(pair))
		err("error: not a pair");
	return pair->d;
}

C *car_subr(void){ return car(alist[0]); }
C *cdr_subr(void){ return cdr(alist[0]); }
C *caar_subr(void){ return car(car(alist[0])); }
C *cadr_subr(void){ return car(cdr(alist[0])); }
C *cdar_subr(void){ return cdr(car(alist[0])); }
C *cddr_subr(void){ return cdr(cdr(alist[0])); }
C *caaar_subr(void){ return car(car(car(alist[0]))); }
C *caadr_subr(void){ return car(car(cdr(alist[0]))); }
C *cadar_subr(void){ return car(cdr(car(alist[0]))); }
C *caddr_subr(void){ return car(cdr(cdr(alist[0]))); }
C *cdaar_subr(void){ return cdr(car(car(alist[0]))); }
C *cdadr_subr(void){ return cdr(car(cdr(alist[0]))); }
C *cddar_subr(void){ return cdr(cdr(car(alist[0]))); }
C *cdddr_subr(void){ return cdr(cdr(cdr(alist[0]))); }
C *caaaar_subr(void){ return car(car(car(car(alist[0])))); }
C *caaadr_subr(void){ return car(car(car(cdr(alist[0])))); }
C *caadar_subr(void){ return car(car(cdr(car(alist[0])))); }
C *caaddr_subr(void){ return car(car(cdr(cdr(alist[0])))); }
C *cadaar_subr(void){ return car(cdr(car(car(alist[0])))); }
C *cadadr_subr(void){ return car(cdr(car(cdr(alist[0])))); }
C *caddar_subr(void){ return car(cdr(cdr(car(alist[0])))); }
C *cadddr_subr(void){ return car(cdr(cdr(cdr(alist[0])))); }
C *cdaaar_subr(void){ return cdr(car(car(car(alist[0])))); }
C *cdaadr_subr(void){ return cdr(car(car(cdr(alist[0])))); }
C *cdadar_subr(void){ return cdr(car(cdr(car(alist[0])))); }
C *cdaddr_subr(void){ return cdr(car(cdr(cdr(alist[0])))); }
C *cddaar_subr(void){ return cdr(cdr(car(car(alist[0])))); }
C *cddadr_subr(void){ return cdr(cdr(car(cdr(alist[0])))); }
C *cdddar_subr(void){ return cdr(cdr(cdr(car(alist[0])))); }
C *cddddr_subr(void){ return cdr(cdr(cdr(cdr(alist[0])))); }
C *rplaca_subr(void){
	if(atom(alist[0]))
		err("error: atom");
	alist[0]->a = alist[1];
	return alist[0];
}
C *rplacd_subr(void){
	if(atom(alist[0]))		/* this could work on a symbolic atom */
		err("error: atom");
	alist[0]->d = alist[1];
	return alist[0];
}
C *cons_subr(void){
	return cons(alist[0], alist[1]);
}
C *atom_subr(void){
	return atom(alist[0]) ? t : nil;
}
C *eq_subr(void){
	return alist[0] == alist[1] ? t : nil;
}
C *equal_subr(void){
	return equal(alist[0], alist[1]) ? t : nil;
}
C *null_subr(void){
	return alist[0] == nil ? t : nil;
}
C *list_fsubr(void){
	return evlis(alist[0], alist[1]) ;
}

C *and_fsubr(void){
	C *l;
	int ret;
	ret = 1;
	for(l = alist[0]; l != nil; l = l->d)
		if(eval(l->a, alist[1]) == nil){
			ret = 0;
			break;
		}
	return ret ? t : nil;
}
C *or_fsubr(void){
	C *l;
	int ret;
	ret = 0;
	for(l = alist[0]; l != nil; l = l->d)
		if(eval(l->a, alist[1]) != nil){
			ret = 1;
			break;
		}
	return ret ? t : nil;
}

C *read_subr(void){
	return readsxp();
}
C *prin1_subr(void){
	if(!atom(alist[0]))
		err("error: need atom");
	printatom(alist[0]);
	return noval;
}
C *print_subr(void){
	print(alist[0]);
	printf("\n");
	return alist[0];
}
C *terpri_subr(void){
	printf("\n");
	return noval;
}


/*
 * Definitions and p-lists
 */

C *attrib_subr(void){
	C *l;
	for(l = alist[0]; l != nil; l = l->d){
//		if(atom(l))	// have to allow this for p-lists
		if(numberp(l))
			err("error: no list");
		if(l->d == nil){
			l->d = alist[1];
			break;
		}
	}
	return alist[1];
}
C *prop_subr(void){
	C *l;
	for(l = alist[0]; l != nil; l = l->d)
		if(l->a == alist[1])
			return l->d;
	return apply(alist[2], nil, nil);
}
C *get_subr(void){
	C *l;
	for(l = alist[0]; l != nil; l = l->d)
		if(l->a == alist[1])
			return l->d->a;
	return nil;
}
/* slightly advanced cset functions */
C *cset_subr(void){
	return defprop(alist[0], alist[1], apval);
}
C *csetq_fsubr(void){
	C *l;
	nargs = 0;
	for(l = alist[0]; l != nil; l = l->d->d){
		if(!atom(l->a))
			err("error: need atom");
		if(l->d == nil){
			defprop(l->a, nil, apval);
			break;
		}
		defprop(l->a, eval(l->d->a, alist[1]), apval);
	}
	return noval;
}
C *remprop_subr(void){
	C *l, **p;
	p = &alist[0]->d;
	for(l = *p; l != nil; l = l->d){
		if(l->a == alist[1]){
			*p = l->d->d;
			break;
		}
		p = &(*p)->d;
	}
	return nil;
}

C *pair_subr(void){
	return pair(alist[0], alist[1]);
}
C *sassoc_subr(void){
	C *l;
	for(l = alist[1]; l != nil; l = l->d)
		if(l->a->a == alist[0])
			return l->a;
	return apply(alist[2], nil, nil);
}

/*
 * Lists
 */

C *append_subr(void){
	C *l, **p;
	assert(temlis.a == nil);
	p = (C**)&temlis.a;
	for(l = alist[0]; l != nil; l = l->d){
		if(atom(l))
			err("error: no list");
		*p = cons(l->a, nil);
		p = &(*p)->d;
	}
	*p = alist[1];
	l = temlis.a;
	temlis.a = nil;
	return l;
}
C *nconc_subr(void){
	C *l;
	for(l = alist[0]; l != nil; l = l->d){
		if(atom(l))
			err("error: no list");
		if(l->d == nil){
			l->d = alist[1];
			break;
		}
	}
	return alist[0];
}
C *copy_subr(void){
	C *l, **p;
	assert(temlis.a == nil);
	p = (C**)&temlis.a;
	for(l = alist[0]; l != nil; l = l->d){
		if(atom(l))
			err("error: no list");
		*p = cons(l->a, nil);
		p = &(*p)->d;
	}
	l = temlis.a;
	temlis.a = nil;
	return l;
}
/*
C *nreverse_subr(void){

	C *l, *n, *last;
	last = nil;
	for(l = alist[0]; l != nil; l = n){
		if(atom(l))
			err("error: no list");
		n = l->d;
		l->d = last;
		last = l;
	}
	return last;
}
*/
C *reverse_subr(void){
	C *l;
	assert(temlis.a == nil);
	for(l = alist[0]; l != nil; l = l->d){
		if(atom(l))
			err("error: no list");
		temlis.a = cons(l->a, temlis.a);
	}
	l = temlis.a;
	temlis.a = nil;
	return l;
}
C *member_subr(void){
	C *l;
	for(l = alist[1]; l != nil; l = l->d){
		if(atom(l))
			err("error: no list");
		if(equal(l->a, alist[0]))
			return t;
	}
	return nil;
}
C *length_subr(void){
	C *l;
	fixnum n;
	n = 0;
	for(l = alist[0]; l != nil; l = l->d)
		n++;
	return mkfix(n);
}
C *efface_subr(void){
	C *l, **p;
	p = &alist[1];
	for(l = alist[1]; l != nil; l = l->d){
		if(atom(l))
			err("error: no list");
		if(equal(l->a, alist[0])){
			*p = l->d;
			break;
		}
		p = &(*p)->d;
	}
	return alist[1];
}

C *maplist_subr(void){
	C *l, *c, **p;
	p = push(nil);
	for(l = alist[0]; l != nil; l = l->d){
		push(c = cons(l, nil));
		c->a = apply(alist[1], c, nil);
		c->d = nil;
		*p = pop();
		p = &(*p)->d;
	}
	return pop();
}
C *mapcon_subr(void){
	C *l, *a, **p;
	p = push(nil);
	push(a = cons(nil, nil));
	for(l = alist[0]; l != nil; l = l->d){
		a->a = l;
		a->d = nil;
		*p = apply(alist[1], a, nil);
		if(*p == nil)
			err("error: nil in mapcon");
		for(; *p != nil; p = &(*p)->d)
			if(atom(*p))
				err("error: no list");
	}
	pop();
	return pop();
}
C *map_subr(void){
	C *l, *a;
	push(a = cons(nil, nil));
	for(l = alist[0]; l != nil; l = l->d){
		a->a = l;
		a->d = nil;
		apply(alist[1], a, nil);
	}
	pop();
	return nil;
}

/*
 * Arithmetic
 */

C *plus_fsubr(void){
	C *l;
	C *tt;
	fixnum fix;
	flonum flo;
	int type;

	fix = 0;
	flo = 0.0;
	type = 0;	// fix;
	for(l = alist[0]; l != nil; l = l->d){
		tt = eval(l->a, alist[1]);
		if(fixnump(tt))
			fix += tt->fix;
		else if(flonump(tt)){
			flo += tt->flo;
			type = 1;
		}else
			err("error: not a number");
	}
	return type == 0 ? mkfix(fix) : mkflo(fix+flo);
}
C *difference_subr(void){
	fixnum fix;
	flonum flo;
	int type;

	fix = 0;
	flo = 0.0;
	type = 0;	// fix;
	if(fixnump(alist[0]))
		fix = alist[0]->fix;
	else if(flonump(alist[0])){
		flo = alist[0]->flo;
		type = 1;
	}else
		err("error: not a number");
	if(fixnump(alist[1]))
		fix -= alist[1]->fix;
	else if(flonump(alist[1])){
		flo -= alist[1]->flo;
		type = 1;
	}else
		err("error: not a number");
	return type == 0 ? mkfix(fix+flo) : mkflo(fix+flo);
}
C *times_fsubr(void){
	C *l;
	C *tt;
	fixnum fix;
	flonum flo;
	int type;

	fix = 1;
	flo = 1.0;
	type = 0;	// fix;
	for(l = alist[0]; l != nil; l = l->d){
		tt = eval(l->a, alist[1]);
		if(fixnump(tt))
			fix *= tt->fix;
		else if(flonump(tt)){
			flo *= tt->flo;
			type = 1;
		}else
			err("error: not a number");
	}
	return type == 0 ? mkfix(fix) : mkflo(fix*flo);
}
C *minus_subr(void){
	if(fixnump(alist[0]))
		return mkfix(-alist[0]->fix);
	if(flonump(alist[0]))
		return mkflo(-alist[0]->flo);
	err("error: not a number");
	return nil;
}
C *add1_subr(void){
	if(fixnump(alist[0]))
		return mkfix(alist[0]->fix+1);
	if(flonump(alist[0]))
		return mkflo(alist[0]->flo+1.0);
	err("error: not a number");
	return nil;
}
C *sub1_subr(void){
	if(fixnump(alist[0]))
		return mkfix(alist[0]->fix-1);
	if(flonump(alist[0]))
		return mkflo(alist[0]->flo-1.0);
	err("error: not a number");
	return nil;
}
C *max_fsubr(void){
	C *l;
	C *tt;
	fixnum fix;
	flonum flo;
	int type;

	fix = LONG_MIN;
	flo = DBL_MIN;
	type = 0;	// fix;
	for(l = alist[0]; l != nil; l = l->d){
		tt = eval(l->a, alist[1]);
		if(fixnump(tt))
			fix = tt->fix > fix ? tt->fix : fix;
		else if(flonump(tt)){
			flo = tt->flo > flo ? tt->flo : flo;
			type = 1;
		}else
			err("error: not a number");
	}
	return type == 0 ? mkfix(fix) : mkflo(fix > flo ? fix : flo);
}
C *min_fsubr(void){
	C *l;
	C *tt;
	fixnum fix;
	flonum flo;
	int type;

	fix = LONG_MAX;
	flo = DBL_MAX;
	type = 0;	// fix;
	for(l = alist[0]; l != nil; l = l->d){
		tt = eval(l->a, alist[1]);
		if(fixnump(tt))
			fix = tt->fix < fix ? tt->fix : fix;
		else if(flonump(tt)){
			flo = tt->flo < flo ? tt->flo : flo;
			type = 1;
		}else
			err("error: not a number");
	}
	return type == 0 ? mkfix(fix) : mkflo(fix < flo ? fix : flo);
}
C *recip_subr(void){
	if(fixnump(alist[0]))
		return mkfix(1/alist[0]->fix);
	else if(flonump(alist[0])){
		return mkflo(1/alist[0]->flo);
	}else
		err("error: not a number");
	return nil;
}
C *quotient_subr(void){
	switch(getnumcase()){
	case 0:
		if(alist[1]->fix == 0)
			err("error: division by zero");
		return mkfix(alist[0]->fix / alist[1]->fix);
		break;
	case 1:
		return mkflo(alist[0]->flo / alist[1]->fix);
		break;
	case 2:
		return mkflo(alist[0]->fix / alist[1]->flo);
		break;
	case 3:
		return mkflo(alist[0]->flo / alist[1]->flo);
		break;
	default:
		err("error: not a number");
		return nil;
	}
}
C *remainder_subr(void){
	switch(getnumcase()){
	case 0:
		if(alist[1]->fix == 0)
			err("error: division by zero");
		return mkfix(alist[0]->fix % alist[1]->fix);
		break;
	case 1:
		return mkflo(fmod(alist[0]->flo, alist[1]->fix));
		break;
	case 2:
		return mkflo(fmod(alist[0]->fix, alist[1]->flo));
		break;
	case 3:
		return mkflo(fmod(alist[0]->flo, alist[1]->flo));
		break;
	default:
		err("error: not a number");
		return nil;
	}
}
C *divide_subr(void){
	C *q, *r;
	switch(getnumcase()){
	case 0:
		if(alist[1]->fix == 0)
			err("error: division by zero");
		q = mkfix(alist[0]->fix / alist[1]->fix);
		temlis.a = q;
		r = mkfix(alist[0]->fix % alist[1]->fix);
		break;
	case 1:
		q = mkflo(alist[0]->flo / alist[1]->fix);
		temlis.a = q;
		r = mkflo(fmod(alist[0]->flo, alist[1]->fix));
		break;
	case 2:
		q = mkflo(alist[0]->fix / alist[1]->flo);
		temlis.a = q;
		r = mkflo(fmod(alist[0]->fix, alist[1]->flo));
		break;
	case 3:
		q = mkflo(alist[0]->flo / alist[1]->flo);
		temlis.a = q;
		r = mkflo(fmod(alist[0]->flo, alist[1]->flo));
		break;
	default:
		err("error: not a number");
		return nil;
	}
	r = cons(q, cons(r, nil));
	temlis.a = nil;
	return r;
}
C *expt_subr(void){
	switch(getnumcase()){
	case 0:
		if(alist[1]->fix == 0)
			err("error: division by zero");
		return mkfix(pow(alist[0]->fix, alist[1]->fix));
		break;
	case 1:
		return mkflo(exp(log(alist[0]->flo) * alist[1]->fix));
		break;
	case 2:
		return mkflo(exp(log(alist[0]->fix) * alist[1]->flo));
		break;
	case 3:
		return mkflo(exp(log(alist[0]->flo) * alist[1]->flo));
		break;
	default:
		err("error: not a number");
		return nil;
	}
}
C *lessp_subr(void){
	int res;
	switch(getnumcase()){
	case 0:
		res = alist[0]->fix < alist[1]->fix;
		break;
	case 1:
		res = alist[0]->flo < alist[1]->fix;
		break;
	case 2:
		res = alist[0]->fix < alist[1]->flo;
		break;
	case 3:
		res = alist[0]->flo < alist[1]->flo;
		break;
	default:
		err("error: not a number");
		return nil;
	}
	return res ? t : nil;
}
C *greaterp_subr(void){
	int res;
	switch(getnumcase()){
	case 0:
		res = alist[0]->fix > alist[1]->fix;
		break;
	case 1:
		res = alist[0]->flo > alist[1]->fix;
		break;
	case 2:
		res = alist[0]->fix > alist[1]->flo;
		break;
	case 3:
		res = alist[0]->flo > alist[1]->flo;
		break;
	default:
		err("error: not a number");
		return nil;
	}
	return res ? t : nil;
}
C *zerop_subr(void){
	int res;
	res = 0;
	if(fixnump(alist[0]))
		res = alist[0]->fix == 0;
	else if(flonump(alist[0]))
		res = floeq(alist[0]->flo, 0.0);
	else
		err("error: not a number");
	return res ? t : nil;
}
C *onep_subr(void){
	int res;
	res = 0;
	if(fixnump(alist[0]))
		res = alist[0]->fix == 1;
	else if(flonump(alist[0]))
		res = floeq(alist[0]->flo, 1.0);
	else
		err("error: not a number");
	return res ? t : nil;
}
C *minusp_subr(void){
	int res;
	res = 0;
	if(fixnump(alist[0]))
		res = alist[0]->fix < 0;
	else if(flonump(alist[0]))
		res = alist[0]->flo < 0.0;
	else
		err("error: not a number");
	return res ? t : nil;
}
C *numberp_subr(void){
	return numberp(alist[0]) ? t : nil;
}
C *fixp_subr(void){
	return fixnump(alist[0]) ? t : nil;
}
C *floatp_subr(void){
	return flonump(alist[0]) ? t : nil;
}
C *logor_fsubr(void){
	C *l;
	C *tt;
	fixnum fix;

	fix = 0;
	for(l = alist[0]; l != nil; l = l->d){
		tt = eval(l->a, alist[1]);
		if(fixnump(tt))
			fix |= tt->fix;
		else
			err("error: not a fixnum");
	}
	return mkfix(fix);
}
C *logand_fsubr(void){
	C *l;
	C *tt;
	fixnum fix;

	fix = ~0;
	for(l = alist[0]; l != nil; l = l->d){
		tt = eval(l->a, alist[1]);
		if(fixnump(tt))
			fix &= tt->fix;
		else
			err("error: not a fixnum");
	}
	return mkfix(fix);
}
C *logxor_fsubr(void){
	C *l;
	C *tt;
	fixnum fix;

	fix = 0;
	for(l = alist[0]; l != nil; l = l->d){
		tt = eval(l->a, alist[1]);
		if(fixnump(tt))
			fix ^= tt->fix;
		else
			err("error: not a fixnum");
	}
	return mkfix(fix);
}
C *leftshift_subr(void){
	if(!fixnump(alist[0]) || !fixnump(alist[1]))
		err("error: not a fixnum");
	if(alist[1]->fix < 0)
		return mkfix(alist[0]->fix >> -alist[1]->fix);
	else
		return mkfix(alist[0]->fix << alist[1]->fix);
}

C *apply_subr(void){
	nargs = 0;
	return apply(alist[0], alist[1], alist[2]);
}
C *eval_subr(void){
	nargs = 0;
	return eval(alist[0], alist[1]);
}
C *evlis_subr(void){
	nargs = 0;
	return evlis(alist[0], alist[1]);
}


typedef struct Prog Prog;
struct Prog
{
	C *a;
	C *go;
	C *pc;
	C *ret;
};

void
setq_prog(Prog *prog, C *form)
{
	C *tt;
	if(form == nil)
		err("error: arg count");
	if(!atom(form->a))
		err("error: no atom");
	tt = sassoc(form->a, prog->a);
	if(tt == noval)
		err("error: undefined");
	tt->d = eval(form->d->a, prog->a);
}

void
set_prog(Prog *prog, C *form)
{
	C *tt;
	if(form == nil)
		err("error: arg count");
	tt = eval(form->a, prog->a);
	if(!atom(tt))
		err("error: no atom");
	tt = sassoc(tt, prog->a);
	if(tt == noval)
		err("error: undefined");
	tt->d = eval(form->d->a, prog->a);
}

void
progstmt(Prog *prog, C *form)
{
	C *tt;
	C *pc;

	if(atom(form))
		;
	else if(form->a == setq)
		setq_prog(prog, form->d);
	else if(form->a == set)
		set_prog(prog, form->d);
	else if(form->a == cond){
		for(form = form->d; form != nil; form = form->d)
			if(eval(form->a->a, prog->a) != nil){
				for(pc = form->a->d; pc != nil; pc = pc->d)
					progstmt(prog, pc->a);
				return;
			}
	}else if(form->a == go){
		if(form->d == nil)
			err("error: arg count");
		if(tt = sassoc(form->d->a, prog->go), tt == noval)
			err("error: undefined label");
		prog->pc = tt->d;
	}else if(form->a == retrn){
		if(form->d == nil)
			prog->ret = nil;
		else
			prog->ret = eval(form->d->a, prog->a);
		prog->pc = nil;
	}else
		eval(form, prog->a);
}

C *prog_fsubr(void){
	Prog prog;

	C *p;
	C **ap;

	prog.pc = alist[0]->d;

	/* build a-list */
	assert(temlis.a == nil);
	ap = (C**)&temlis.a;
	for(p = alist[0]->a; p != nil; p = p->d){
		*ap = cons(cons(p->a, nil), nil);
		ap = &(*ap)->d;
	}
	*ap = alist[1];
	alist[1] = temlis.a;
	prog.a = alist[1];
	temlis.a = nil;

	/* build go-list */
	for(p = prog.pc; p != nil; p = p->d)
		if(atom(p->a))
			temlis.a = cons(p, temlis.a);
	prog.go = temlis.a;
	temlis.a = nil;
	alist[nargs++] = prog.go;

	/* execute */
	prog.ret = nil;
	while(prog.pc != nil){
		p = prog.pc->a;
		prog.pc = prog.pc->d;
		progstmt(&prog, p);
	}

	return prog.ret;
}

void
initsubr(void)
{
	C *a;

	a = intern("QUOTE");
	defprop(a, (C*)consw((word)quote_fsubr), fsubr);
	a = intern("FUNCTION");
	defprop(a, (C*)consw((word)function_fsubr), fsubr);
	a = intern("PROG");
	defprop(a, (C*)consw((word)prog_fsubr), fsubr);


	defprop(intern("CAR"), mksubr(car_subr, 1), subr);
	defprop(intern("CDR"), mksubr(cdr_subr, 1), subr);
	defprop(intern("CAAR"), mksubr(caar_subr, 1), subr);
	defprop(intern("CADR"), mksubr(cadr_subr, 1), subr);
	defprop(intern("CDAR"), mksubr(cdar_subr, 1), subr);
	defprop(intern("CDDR"), mksubr(cddr_subr, 1), subr);
	defprop(intern("CAAAR"), mksubr(caaar_subr, 1), subr);
	defprop(intern("CAADR"), mksubr(caadr_subr, 1), subr);
	defprop(intern("CADAR"), mksubr(cadar_subr, 1), subr);
	defprop(intern("CADDR"), mksubr(caddr_subr, 1), subr);
	defprop(intern("CDAAR"), mksubr(cdaar_subr, 1), subr);
	defprop(intern("CDADR"), mksubr(cdadr_subr, 1), subr);
	defprop(intern("CDDAR"), mksubr(cddar_subr, 1), subr);
	defprop(intern("CDDDR"), mksubr(cdddr_subr, 1), subr);
	defprop(intern("CAAAAR"), mksubr(caaaar_subr, 1), subr);
	defprop(intern("CAAADR"), mksubr(caaadr_subr, 1), subr);
	defprop(intern("CAADAR"), mksubr(caadar_subr, 1), subr);
	defprop(intern("CAADDR"), mksubr(caaddr_subr, 1), subr);
	defprop(intern("CADAAR"), mksubr(cadaar_subr, 1), subr);
	defprop(intern("CADADR"), mksubr(cadadr_subr, 1), subr);
	defprop(intern("CADDAR"), mksubr(caddar_subr, 1), subr);
	defprop(intern("CADDDR"), mksubr(cadddr_subr, 1), subr);
	defprop(intern("CDAAAR"), mksubr(cdaaar_subr, 1), subr);
	defprop(intern("CDAADR"), mksubr(cdaadr_subr, 1), subr);
	defprop(intern("CDADAR"), mksubr(cdadar_subr, 1), subr);
	defprop(intern("CDADDR"), mksubr(cdaddr_subr, 1), subr);
	defprop(intern("CDDAAR"), mksubr(cddaar_subr, 1), subr);
	defprop(intern("CDDADR"), mksubr(cddadr_subr, 1), subr);
	defprop(intern("CDDDAR"), mksubr(cdddar_subr, 1), subr);
	defprop(intern("CDDDDR"), mksubr(cddddr_subr, 1), subr);

	a = intern("CONS");
	defprop(a, mksubr(cons_subr, 2), subr);
	a = intern("RPLACA");
	defprop(a, mksubr(rplaca_subr, 2), subr);
	a = intern("RPLACD");
	defprop(a, mksubr(rplacd_subr, 2), subr);
	a = intern("ATOM");
	defprop(a, mksubr(atom_subr, 1), subr);
	a = intern("EQ");
	defprop(a, mksubr(eq_subr, 2), subr);
	a = intern("EQUAL");
	defprop(a, mksubr(equal_subr, 2), subr);
	a = intern("NULL");
	defprop(a, mksubr(null_subr, 1), subr);
	a = intern("LIST");
	defprop(a, (C*)consw((word)list_fsubr), fsubr);

	a = intern("NOT");
	defprop(a, mksubr(null_subr, 1), subr);
	a = intern("AND");
	defprop(a, (C*)consw((word)and_fsubr), fsubr);
	a = intern("OR");
	defprop(a, (C*)consw((word)or_fsubr), fsubr);

	a = intern("APPLY");
	defprop(a, mksubr(apply_subr, 3), subr);
	a = intern("EVAL");
	defprop(a, mksubr(eval_subr, 2), subr);
	a = intern("EVLIS");
	defprop(a, mksubr(evlis_subr, 2), subr);

	a = intern("ATTRIB");
	defprop(a, mksubr(attrib_subr, 2), subr);
	a = intern("PROP");
	defprop(a, mksubr(prop_subr, 3), subr);
	a = intern("GET");
	defprop(a, mksubr(get_subr, 2), subr);
	a = intern("CSET");
	defprop(a, mksubr(cset_subr, 2), subr);
	a = intern("CSETQ");
	defprop(a, (C*)consw((word)csetq_fsubr), fsubr);
	a = intern("REMPROP");
	defprop(a, mksubr(remprop_subr, 2), subr);

	a = intern("PAIR");
	defprop(a, mksubr(pair_subr, 2), subr);
	a = intern("SASSOC");
	defprop(a, mksubr(sassoc_subr, 3), subr);

	a = intern("APPEND");
	defprop(a, mksubr(append_subr, 2), subr);
	a = intern("NCONC");
	defprop(a, mksubr(nconc_subr, 2), subr);
	a = intern("COPY");
	defprop(a, mksubr(copy_subr, 1), subr);
	a = intern("REVERSE");
	defprop(a, mksubr(reverse_subr, 1), subr);
	a = intern("MEMBER");
	defprop(a, mksubr(member_subr, 2), subr);
	a = intern("LENGTH");
	defprop(a, mksubr(length_subr, 1), subr);
	a = intern("EFFACE");
	defprop(a, mksubr(efface_subr, 2), subr);

	a = intern("MAPLIST");
	defprop(a, mksubr(maplist_subr, 2), subr);
	a = intern("MAPCON");
	defprop(a, mksubr(mapcon_subr, 2), subr);
	a = intern("MAP");
	defprop(a, mksubr(map_subr, 2), subr);

	a = intern("PLUS");
	defprop(a, (C*)consw((word)plus_fsubr), fsubr);
	a = intern("DIFFERENCE");
	defprop(a, mksubr(difference_subr, 2), subr);
	a = intern("MINUS");
	defprop(a, mksubr(minus_subr, 1), subr);
	a = intern("TIMES");
	defprop(a, (C*)consw((word)times_fsubr), fsubr);
	a = intern("ADD1");
	defprop(a, mksubr(add1_subr, 1), subr);
	a = intern("SUB1");
	defprop(a, mksubr(sub1_subr, 1), subr);
	a = intern("MAX");
	defprop(a, (C*)consw((word)max_fsubr), fsubr);
	a = intern("MIN");
	defprop(a, (C*)consw((word)min_fsubr), fsubr);
	a = intern("RECIP");
	defprop(a, mksubr(recip_subr, 1), subr);
	a = intern("QUOTIENT");
	defprop(a, mksubr(quotient_subr, 2), subr);
	a = intern("REMAINDER");
	defprop(a, mksubr(remainder_subr, 2), subr);
	a = intern("DIVIDE");
	defprop(a, mksubr(divide_subr, 2), subr);
	a = intern("EXPT");
	defprop(a, mksubr(expt_subr, 2), subr);
	a = intern("LESSP");
	defprop(a, mksubr(lessp_subr, 2), subr);
	a = intern("GREATERP");
	defprop(a, mksubr(greaterp_subr, 2), subr);
	a = intern("ZEROP");
	defprop(a, mksubr(zerop_subr, 1), subr);
	a = intern("ONEP");
	defprop(a, mksubr(onep_subr, 1), subr);
	a = intern("MINUSP");
	defprop(a, mksubr(minusp_subr, 1), subr);
	a = intern("NUMBERP");
	defprop(a, mksubr(numberp_subr, 1), subr);
	a = intern("FIXP");
	defprop(a, mksubr(fixp_subr, 1), subr);
	a = intern("FLOATP");
	defprop(a, mksubr(floatp_subr, 1), subr);
	a = intern("LOGOR");
	defprop(a, (C*)consw((word)logor_fsubr), fsubr);
	a = intern("LOGAND");
	defprop(a, (C*)consw((word)logand_fsubr), fsubr);
	a = intern("LOGXOR");
	defprop(a, (C*)consw((word)logxor_fsubr), fsubr);
	a = intern("LEFTSHIFT");
	defprop(a, mksubr(leftshift_subr, 2), subr);

	a = intern("READ");
	defprop(a, mksubr(read_subr, 0), subr);
	a = intern("PRIN1");
	defprop(a, mksubr(prin1_subr, 1), subr);
	a = intern("PRINT");
	defprop(a, mksubr(print_subr, 1), subr);
	a = intern("TERPRI");
	defprop(a, mksubr(terpri_subr, 1), subr);

	defprop(t, t, apval);
}
