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
Data *empty_data() { Data *data = malloc(sizeof(Data)); bzero(data, sizeof(Data)); return data; }


/* {{{ make atom/cons/data */
/* make an atom */
Data *make_atom(char *str) {
  Atom *atom = empty_atom();
  atom->name = copy_str(str);
  return make_data(ATOM, atom);
}

/* make a cons */
Data *make_cons(Data *car, Data *cdr) {
  Cons *cons = empty_cons();
  cons->car = copy_data(car);
  cons->cdr = copy_data(cdr);
  return make_data(CONS, cons);
}

/* make a data */
Data *make_data(Type type, void *data) {
  Data *new_data = empty_data();
  new_data->type = type;
  new_data->data = data;
  return new_data;
}
/* }}} */


/* {{{ copy  str/atom/cons/data */
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

/* return a copy of data */
Data *copy_data(Data *data) {
  Data *new_data = empty_data();
  if (data) memcpy(new_data, data, sizeof(Data));
  return new_data;
}
/* }}} */


/* {{{ string  str/atom/cons/data */
/* to string: atom, cons, data */
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
  car_str = data_to_string(cons->car);
  if (!cons->cdr) return car_str;
  if (cons->cdr->type == ATOM) {
    cdr_str = atom_to_string((Atom *)(cons->cdr->data));
  } else {
    cdr_str = cons_to_string((Cons *)(cons->cdr->data), 0);
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

char *data_to_string(Data *data) {
  if (!data) return "";
  switch (data->type) {
  case ATOM: return atom_to_string((Atom *)(data->data));
  case CONS: return cons_to_string((Cons *)(data->data), 1);
  default  : return "";                         /* gcc warning */
  }
}
/* }}} */


/* {{{ read atom/cons/data */
static int read_from_string_index = 0;
Data *read_atom(char *str, int beg) {
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

Data *read_cons(char *str, int beg) {
  Data *car;
  Data *cdr;

  while (isspace(str[beg])) beg++;
  if (str[beg] == ')') {
    read_from_string_index++;
    return empty_data();
  }
  if (str[beg] == '.') return read_atom(str, beg + 1);
  car = read_from(str, beg);
  cdr = read_cons(str, read_from_string_index);
  return make_cons(car, cdr);
}

Data *read_from(char *str, int beg) {
  Data *quote = make_atom("quote");

  while (isspace(str[beg])) beg++;
  switch (str[beg]) {
  case '(':
    read_from_string_index++;
    return read_cons(str, beg + 1);
  case '\'':
    read_from_string_index++;
    return make_cons(quote, make_cons(read_from(str, beg + 1), empty_data()));
  case ')':
    read_from_string_index++;
    return NULL;
  default : return read_atom(str, beg);
  }
}

Data *read_data(char *str) {
  return read_from(str, 0);
}

static Data *env = NULL;
Data *eval(Data *data, Cons *env) {
  return NULL;
}
/* }}} */

Data *_cons_(Data *car, Data *cdr) {
  return make_cons(car, cdr);
}

Data *_car_(Data *data) {
  if (!data) return NULL;
  switch (data->type) {
  case ATOM: return NULL;
  case CONS: return ((Cons *)(data->data))->car;
  default  : return NULL;                       /* gcc warning */
  }
}

Data *_cdr_(Data *data) {
  if (!data) return NULL;
  switch (data->type) {
  case ATOM: return NULL;
  case CONS: return ((Cons *)(data->data))->cdr;
  default  : return NULL;                       /* gcc warning */
  }
}

Data *_quote_(Data *data) {
  return data;
}

int _nil_(Data *data) {
  return !(data && data->type == ATOM && !strcmp(data_to_string(data), "nil"));
}

Data *_if_(Data *if_, Data *then_, Data *else_) {
  return _nil_(if_) ? (then_) : (else_);
}

Data *_atom_(Data *data) {
  if (!data) return Qt;
  switch (data->type) {
  case ATOM: return Qt;
  case CONS: return _eq_(data, Qt);
  default  : return Qnil;                       /* gcc warning */
  }
}

Data *_eq_(Data *d1, Data *d2) {
  return !strcmp(data_to_string(d1), data_to_string(d2)) ? Qt : Qnil;
}

Data *_read_(char *str) {
  return read_data(str);
}

/* TODO: implement it */
Data *_eval_(Data *data) {
  Data *car;
  Data *cdr;
  Data *key, *val;
  char *car_str;

  if (!data) return NULL;

  switch (data->type) {
  case ATOM: return _assoc_(data, env);
  case CONS:
    car = _car_(data);
    cdr = _cdr_(data);

    if (_atom_(car)) {
      car_str = data_to_string(car);
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
      }
    }
    return data;
  default  : return data;
  }
}

Data *_assoc_(Data *key, Data *pair) {
  Data *car;
  Data *cdr;
  if (!pair) return NULL;
  car = _car_(pair);
  cdr = _cdr_(pair);
  if (_eq_(_car_(car), key)) {
    return _cdr_(car);
  } else {
    return _assoc_(key, _cdr_(pair));
  }
}

/* init env with some testing data ... */
void init_env() {
  char *keys [] = { "os",  "editor", "who"   };
  char *vals [] = { "mac", "emacs",  "xshen" };
  Data *key;
  Data *val;
  Data *pair;
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
  Data *data;
  Data *value;
  init_env();

  while (1) {
    printf("env  : %s\n", data_to_string(env));
    printf("lisp > ");
    gets(str);
    /* printf("=== I: %s\n", str); */
    data = _read_(str);
    /* printf("=== O: %s\n", data_to_string(data)); */
    value = _eval_(data);
    printf("=== V: %s\n", data_to_string(value));
  }
}

void read_print(char *str) {
  Data *data = _read_(str);
  printf("read: \"%s\" => %s\n", str, data_to_string(data));
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
    /* printf("read(\"%s\") => %s\n", str[i], data_to_string(_read_(str[i]))); */
  }

  /* read_print("who"); */
  /* read_print("'who"); */
  /* read_print("'(who)"); */
  repl();
  return 0;
}
