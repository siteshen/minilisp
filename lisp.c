#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>
#include <string.h>

#include "lisp.h"

char *sub_str(char *str, int beg, int end) {
  char *new_str;
  if (beg >= end) return NULL;
  new_str = malloc(end - beg + 1);
  bzero(new_str, end - beg + 1);
  strncpy(new_str, str + beg, end - beg);
  return new_str;
}

/* empty_*: create empty struct */
Atom *empty_atom() { Atom *atom = malloc(sizeof(Atom)); bzero(atom, sizeof(Atom)); return atom; }
Cons *empty_cons() { Cons *cons = malloc(sizeof(Cons)); bzero(cons, sizeof(Cons)); return cons; }
Sexp *empty_sexp() { Sexp *sexp = malloc(sizeof(Sexp)); bzero(sexp, sizeof(Sexp)); return sexp; }


/* {{{ make atom/cons/sexp */
/* make an atom */
Sexp *make_atom(char *str) {
  Atom *atom = empty_atom();
  atom->name = copy_str(str);
  return make_sexp(ATOM, atom);
}

/* make a cons */
Sexp *make_cons(Sexp *car, Sexp *cdr) {
  Cons *cons = empty_cons();
  cons->car = copy_sexp(car);
  cons->cdr = copy_sexp(cdr);
  return make_sexp(CONS, cons);
}

/* make a sexp */
Sexp *make_sexp(Type type, void *sexp) {
  Sexp *new_sexp = empty_sexp();
  new_sexp->type = type;
  new_sexp->sexp = sexp;
  return new_sexp;
}
/* }}} */


/* {{{ copy  str/atom/cons/sexp */
/* return a copy of str */
char *copy_str(char *str) {
  char *new_str = malloc(strlen(str) + 1);
  strcpy(new_str, str);
  return new_str;
}

/* return a copy of atom */
Atom *copy_atom(Atom *atom) {
  Atom *new_atom = empty_atom();
  if (atom) memcpy(new_atom, atom, sizeof(Atom));
  return new_atom;
}

/* return a copy of cons */
Cons *copy_cons(Cons *cons) {
  Cons *new_cons = empty_cons();
  if (cons) memcpy(new_cons, cons, sizeof(Cons));
  return new_cons;
}

/* return a copy of sexp */
Sexp *copy_sexp(Sexp *sexp) {
  Sexp *new_sexp = empty_sexp();
  if (sexp) memcpy(new_sexp, sexp, sizeof(Sexp));
  return new_sexp;
}
/* }}} */


/* {{{ string  str/atom/cons/sexp */
/* to string: atom, cons, sexp */
char *atom_to_string(Atom *atom) {
  if (!atom) return "nil";
  return atom->name;
}

char *cons_to_string(Cons *cons, int racket) {
  char *car_str;
  char *cdr_str;
  int str_len;
  char *new_str;

  if (!cons) return "nil";
  car_str = sexp_to_string(cons->car);
  if (!cons->cdr) return car_str;
  if (cons->cdr->type == ATOM) {
    cdr_str = atom_to_string((Atom *)(cons->cdr->sexp));
  } else {
    cdr_str = cons_to_string((Cons *)(cons->cdr->sexp), 0);
  }
  str_len = strlen(car_str) + strlen(cdr_str) + 6;
  new_str = malloc(str_len);

  bzero(new_str, str_len);
  if (racket) strcat(new_str, "(");
  strcat(new_str, car_str);
  if (strcmp(cdr_str, "nil")) {
    if (cons->cdr->type == ATOM) {
      strcat(new_str, " . ");
    } else if (cons->cdr->type == CONS) {
      strcat(new_str, " ");
    }
    strcat(new_str, cdr_str);
  }
  if (racket) strcat(new_str, ")");
  return new_str;
}

char *sexp_to_string(Sexp *sexp) {
  if (!sexp) return "nil";
  switch (sexp->type) {
  case ATOM: return atom_to_string((Atom *)(sexp->sexp));
  case CONS: return cons_to_string((Cons *)(sexp->sexp), 1);
  default  : return "nil";                      /* gcc warning */
  }
}
/* }}} */


/* {{{ read atom/cons/sexp */
static int read_from_string_index = 0;
Sexp *read_atom(char *str, int beg) {
  int atom_len = 0;
  char *name = NULL;

  while (isspace(str[beg])) beg++;
  while (str[beg] && !isspace(str[beg]) && str[beg] != '(' && str[beg] != ')') {
    beg++;
    atom_len++;
  }
  name = sub_str(str, beg - atom_len, beg);
  read_from_string_index = beg;
  return make_atom(name);
}

Sexp *read_cons(char *str, int beg) {
  Sexp *car;
  Sexp *cdr;

  while (isspace(str[beg])) beg++;
  if (str[beg] == ')') {
    beg++;
    read_from_string_index = beg;
    return empty_sexp();
  }
  if (str[beg] == '.') return read_atom(str, beg + 1);
  car = read_from(str, beg);
  cdr = read_cons(str, read_from_string_index);
  return make_cons(car, cdr);
}

Sexp *read_from(char *str, int beg) {
  while (isspace(str[beg])) beg++;
  switch (str[beg]) {
  case '(':
    read_from_string_index++;
    return read_cons(str, beg + 1);
  case '\'':
    read_from_string_index++;
    return make_cons(Qquote, make_cons(read_from(str, beg + 1), empty_sexp()));
  case ')':
    read_from_string_index++;
    return Qnil;
  default : return read_atom(str, beg);
  }
}

Sexp *read_sexp(char *str) {
  return read_from(str, 0);
}

static Sexp *env = NULL;
Sexp *eval(Sexp *sexp, Cons *env) {
  return NULL;
}
/* }}} */

Sexp *_cons_(Sexp *car, Sexp *cdr) {
  return make_cons(car, cdr);
}

Sexp *_car_(Sexp *sexp) {
  if (!sexp) return Qnil;
  switch (sexp->type) {
  case ATOM: return Qnil;
  case CONS: return ((Cons *)(sexp->sexp))->car;
  default  : return Qnil;                       /* gcc warning */
  }
}

Sexp *_cdr_(Sexp *sexp) {
  if (!sexp) return Qnil;
  switch (sexp->type) {
  case ATOM: return Qnil;
  case CONS: return ((Cons *)(sexp->sexp))->cdr;
  default  : return Qnil;                       /* gcc warning */
  }
}

Sexp *_quote_(Sexp *sexp) {
  return sexp;
}

Sexp *_if_(Sexp *if_, Sexp *then_, Sexp *else_) {
  return !NILP(if_) ? (then_) : (else_);
}

Sexp *_atom_(Sexp *sexp) {
  if (!sexp) return Qt;
  switch (sexp->type) {
  case ATOM: return Qt;
  case CONS: return _eq_(sexp, Qt);
  default  : return Qnil;                       /* gcc warning */
  }
}

Sexp *_eq_(Sexp *d1, Sexp *d2) {
  return !strcmp(sexp_to_string(d1), sexp_to_string(d2)) ? Qt : Qnil;
}

Sexp *_read_(char *str) {
  return read_sexp(str);
}

int NILP(Sexp *sexp) {
  return (sexp && sexp->type == ATOM && !strcmp(sexp_to_string(sexp), "nil"));
}

int LAMBDAP(Sexp *sexp) {
  char *car_str;

  if (sexp && sexp->type == CONS) {
    car_str = sexp_to_string(_car_(sexp));
    return (!strcmp(car_str, "lambda"));
  }
  return 0;
}

Sexp * _nth_(int n, Sexp *sexp) {
  while (n > 0 && !NILP(sexp)) {
    sexp = _cdr_(sexp);
    n--;
  }
  return _car_(sexp);
}

/* TODO: implement it */
Sexp *_eval_(Sexp *sexp) {
  Sexp *car;
  Sexp *cdr;
  Sexp *key, *val;
  Sexp *lambda;
  char *car_str;

  if (!sexp) return Qnil;

  switch (sexp->type) {
  case ATOM: return _assoc_(sexp, env);
  case CONS:
    car = _car_(sexp);
    cdr = _cdr_(sexp);

    if (_atom_(car)) {
      car_str = sexp_to_string(car);
      if (!strcmp(car_str, "quote")) {
        return _quote_(_car_(cdr));
      } else if (!strcmp(car_str, "atom")) {
        return _atom_(_eval_(_car_(cdr)));
      } else if (!strcmp(car_str, "eq")) {
        return _eq_(_eval_(_car_(cdr)), _eval_(_car_(_cdr_(cdr))));
      } else if (!strcmp(car_str, "car")) {
        return _car_(_eval_(_car_(cdr)));
      } else if (!strcmp(car_str, "cdr")) {
        return _cdr_(_eval_(_car_(cdr)));
      } else if (!strcmp(car_str, "cons")) {
        return _cons_(_eval_(_car_(cdr)),
                      _eval_(_car_(_cdr_(cdr))));
      } else if (!strcmp(car_str, "if")) {
        return _if_(_car_(cdr),
                    _eval_(_car_(_cdr_(cdr))),
                    _eval_(_car_(_cdr_(_cdr_(cdr)))));
      } else if (!strcmp(car_str, "def")) {
        key = _car_(cdr);
        val = _eval_(_car_(_cdr_(cdr)));
        env = _cons_(_cons_(key, val), env);
        return _quote_(key);
      } else if (!strcmp(car_str, "lambda")) {
        return _quote_(sexp);
      } else if (LAMBDAP(_eval_(car))) {
        lambda = (_eval_(car));
        /* arg_list = (_nth_(1, lambda)); */
      } else if (!strcmp(car_str, "read")) {
        return _read_(sexp_to_string(_car_(cdr)));
      } else if (!strcmp(car_str, "eval")) {
        return _eval_((_car_(cdr)));
      }
    }
    return sexp;
  default  : return sexp;
  }
}

Sexp *_assoc_(Sexp *key, Sexp *pair) {
  Sexp *car;
  Sexp *cdr;
  if (!pair) return Qnil;
  car = _car_(pair);
  cdr = _cdr_(pair);
  if (_eq_(_car_(car), key)) {
    return _cdr_(car);
  } else {
    return _assoc_(key, _cdr_(pair));
  }
}

/* init env with some testing sexp ... */
void init_env() {
  char *keys [] = { "os",  "editor", "who"   };
  char *vals [] = { "mac", "emacs",  "xshen" };
  Sexp *key;
  Sexp *val;
  Sexp *pair;
  int i = 0;

  Qnil = make_atom("nil");
  Qt = make_atom("t");
  Qquote = make_atom("quote");

  for (i = 0; i < 3; ++i) {
    key = make_atom(keys[i]);
    val = make_atom(vals[i]);
    pair = _cons_(key, val);
    env = _cons_(pair, env);
  }
}

void repl() {
  char str[256];
  Sexp *sexp;
  Sexp *value;

  while (1) {
    printf("env  : %s\n", sexp_to_string(env));
    printf("lisp > ");
    gets(str);
    /* printf("=== I: %s\n", str); */
    sexp = _read_(str);
    /* printf("=== O: %s\n", sexp_to_string(sexp)); */
    value = _eval_(sexp);
    printf("=== V: %s\n", sexp_to_string(value));
  }
}

void read_print(char *str) {
  Sexp *sexp = _read_(str);
  printf("read: \"%s\" => %s\n", str, sexp_to_string(sexp));
}

int main(int argc, char *argv[]) {
  int i;
  char *str[] = {
    "hello", "(hello)", "((hello))",
    "(a b)", "(a (b))", "((a) b)", "((a) (b))",
    "(a b c)", "((a) b c)", "(a (b) c)", "((a) b c)", "((a) (b) (c))",
    "((a b) c)", "(a (b c))"
  };
  i = 5;
  for (i = 0; i < 14; i++) {
    /* printf("read(\"%s\") => %s\n", str[i], sexp_to_string(_read_(str[i]))); */
  }

  init_env();
  /* read_print("who"); */
  /* read_print("'who"); */
  /* read_print("'(who)"); */
  /* read_print("(car () 'a)"); */
  /* read_print("(car nil 'a)"); */
  /* read_print("(cons () 't)"); */
  /* read_print("(lambda () 'a)"); */
  repl();
  return 0;
}
