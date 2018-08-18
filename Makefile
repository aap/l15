CFLAGS=-g -Wall -Wextra
LDFLAGS=-lm
lisp: lisp.o subr.o mem.o
lisp.o: lisp.h
subr.o: lisp.h
mem.o: lisp.h
