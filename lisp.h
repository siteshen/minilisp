#ifndef _LISP_H_
#define _LISP_H_


typedef enum   { ATOM, CONS, } Type;
typedef struct { char *name; } Atom;
typedef struct { Type type; void *sexp; } Sexp;
typedef struct { Sexp *car; Sexp *cdr;  } Cons;

Atom *empty_atom();
Cons *empty_cons();
Sexp *empty_sexp();

Sexp *make_atom(char *str);
Sexp *make_cons(Sexp *car, Sexp *cdr);
Sexp *make_sexp(Type type, void *sexp);

char *copy_str(char *str);
Atom *copy_atom(Atom *atom);
Cons *copy_cons(Cons *cons);
Sexp *copy_sexp(Sexp *sexp);

char *atom_to_string(Atom *atom);
char *cons_to_string(Cons *cons, int racket);
char *sexp_to_string(Sexp *sexp);

Sexp *read_atom(char *str, int beg);
Sexp *read_cons(char *str, int beg);
Sexp *read_from(char *str, int beg);
Sexp *read_sexp(char *str);

Sexp *_eval_(Sexp *sexp);
Sexp *_cons_(Sexp *car, Sexp *cdr);
Sexp *_car_(Sexp *sexp);
Sexp *_cdr_(Sexp *sexp);
Sexp *_quote_(Sexp *sexp);
Sexp *_if_(Sexp *_if, Sexp *_then, Sexp *_else);
Sexp *_atom_(Sexp *sexp);
Sexp *_eq_(Sexp *d1, Sexp *d2);

Sexp *fn_call(Sexp *fn, Sexp *args);
Sexp *eval(Sexp *sexp, Sexp *env);
Sexp *_assoc_(Sexp *key, Sexp *pair);


static Sexp *Qnil, *Qt, *Qquote, *Qunbound;

int NILP(Sexp *sexp);
int LAMBDAP(Sexp *sexp);


#endif /* _LISP_H_ */
