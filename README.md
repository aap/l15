# LISP 1.5

This is an implementation of LISP 1.5 as described by the manual.
I have not slavishly implemented everything and some things
may be incomplete. Please open issues if things are missing.

## Build

Just type `make`.

## How to use

By default you're talking to an `eval` REPL. e.g.

```
% ./lisp
*
(car (quote (a b)))
A
```

By passing the `-q` option you get an `evalquote` REPL as decribed
in the LISP manual. e.g.

```
% ./lisp -q
*
car ((a b))
A
```

## To-Do

* Implement more useful things
* Arrays
* Assembler (and compiler?)
* some code examples
