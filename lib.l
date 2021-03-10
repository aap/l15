;;; putprop puts one properly on a p-list. taken from MACLISP
(attrib (quote putprop)
  (quote (expr (lambda (atm val ind)
                ((lambda (p)
                   (cond ((null p) (attrib atm (list ind val)))
                         (t (rplaca p val))))
                 (prop atm ind (quote (lambda () nil))))))))

;;; defprop is like putprop but doesn't eval its args. taken from MACLISP
(putprop (quote defprop)
  (quote (lambda (args env)
           (putprop (car args) (cadr args) (caddr args))))
  (quote fexpr))

(defprop deflist
  (lambda (x ind)
    (map x (function (lambda (pair)
             (putprop (caar pair) (cadar pair) ind)))))
  expr)

(defprop define
  (lambda (x) (deflist x (quote expr)))
  expr)



;;;
;;; examples
;;;


;;; compute greatest common divisor
(define (quote (
(gcd (lambda (a b)
  (cond ((lessp a b) (gcd b a))
        ((eq b 0) a)
        (t (gcd b (difference a b))))))
)))


;;; differentiate expression exp w.r.t. x
(defprop diff
  (lambda (exp x)
     (cond ((eq exp x) 1)
           ((atom exp) 0)
           ((eq (car exp) (quote plus))
            (cons (quote plus)
                  (maplist (cdr exp)
                    (function (lambda (J)
                      (diff (car J) x))))))
           ((eq (car exp) (quote times))
            (cons (quote plus)
                  (maplist (cdr exp)
                    (function (lambda (J)
                      (cons (quote times)
                            (maplist (cdr exp)
                              (function (lambda (K)
                                (cond ((equal J K) (diff (car K) x))
                                      (t (car K))))))))))))
           (t (quote invalid))))
   expr)



;;; simplify mathematical expression
(defprop simplify
  (lambda (exp)
    (cond ((atom exp) exp)
           ((eq (car exp) (quote plus)) (simpsum (simplis (cdr exp))))
           ((eq (car exp) (quote times)) (simpprod (simplis (cdr exp))))
           (t exp)))
  expr)

;;; simplify a list of expressions
(defprop simplis
  (lambda (lst) (maplist lst (quote (lambda (l) (simplify (car l))))))
  expr)

;;; simplify the terms of a sum
(defprop simpsum
  (lambda (terms)
    (prog (sep const var)
      (setq sep (separate terms nil nil))
      (setq const (car sep))
      (setq var (cadr sep))
      (setq const (eval (cons (quote plus) const) nil))
      (return (cond ((null var) const)
                    ((eq const 0)
                     (cond ((null (cdr var)) (car var))
                           (t (cons (quote plus) var))))
                    (t (cons (quote plus) (cons const var)))))))
  expr)

;;; simplify the terms of a product
(defprop simpprod
  (lambda (terms)
    (prog (sep const var)
      (setq sep (separate terms nil nil))
      (setq const (car sep))
      (setq var (cadr sep))
      (setq const (eval (cons (quote times) const) nil))
      (return (cond ((null var) const)
                    ((eq const 0) 0)
                    ((eq const 1)
                     (cond ((null (cdr var)) (car var))
                           (t (cons (quote times) var))))
                    (t (cons (quote times) (cons const var)))))))
  expr)

;;; separate constants from variables in a list
(defprop separate
  (lambda (lst const var)
    (cond ((null lst)
           (list const var))
          ((numberp (car lst))
           (separate (cdr lst) (cons (car lst) const) var))
          (t (separate (cdr lst) const (cons (car lst) var)))))
  expr)
