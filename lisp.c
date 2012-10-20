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
Atom *make_atom(char *str) {
  Atom *atom = empty_atom();
  atom->name = copy_str(str);
  return atom;
}

/* make a cons */
Cons *make_cons(Data *car, Data *cdr) {
  Cons *cons = empty_cons();
  cons->car = copy_data(car);
  cons->cdr = copy_data(cdr);
  return cons;
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
  if (!atom) return "";
  return atom->name;
}

char *cons_to_string(Cons *cons) {
  char *car_str;
  char *cdr_str;
  int str_len;
  char *new_str;

  if (!cons) return "";
  if (!cons->car && !cons->cdr) return "()";
  car_str = data_to_string(cons->car);
  cdr_str = data_to_string(cons->cdr);
  str_len = strlen(car_str) + strlen(cdr_str) + 4;
  new_str = malloc(str_len);

  bzero(new_str, str_len);
  strcat(new_str, "(");
  strcat(new_str, car_str);
  strcat(new_str, " ");
  strcat(new_str, cdr_str);
  strcat(new_str, ")");
  return new_str;
}

char *data_to_string(Data *data) {
  if (!data) return "";
  switch (data->type) {
  case ATOM: return atom_to_string((Atom *)(data->data));
  case CONS: return cons_to_string((Cons *)(data->data));
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
  return make_data(ATOM, make_atom(name));
}

Data *read_cons(char *str, int beg) {
  Data *car;
  Data *cdr;

  while (isspace(str[beg])) beg++;
  car = read_from(str, beg + 1);
  cdr = read_from(str, read_from_string_index);
  return make_data(CONS, make_cons(car, cdr));
}

Data *read_from(char *str, int beg) {
  while (isspace(str[beg])) beg++;
  switch (str[beg]) {
  case '(': return read_cons(str, beg);
  case ')': return NULL;
  default : return read_atom(str, beg);
  }
}

Data *read_data(char *str) {
  return read_from(str, 0);
}
/* }}} */

Data *_read_(char *str) {
  return read_data(str);
}

/* TODO: implement it */
Data *_eval_(Data *data) {
  return data;
}

Data *_cons_(Data *car, Data *cdr) {
  Cons *cons = make_cons(_eval_(car), _eval_(cdr));
  return make_data(CONS, cons);
}

Data *_car_(Data *data) {
  data = _eval_(data);
  if (!data) return NULL;
  switch (data->type) {
  case ATOM: return NULL;
  case CONS: return ((Cons *)(data->data))->car;
  default  : return NULL;                       /* gcc warning */
  }
}

Data *_cdr_(Data *data) {
  data = _eval_(data);
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

Data *_if_(Data *if_, Data *then_, Data *else_) {
  return _eval_(if_) ? _eval_(then_) : _eval_(else_);
}

int _atom_(Data *data) {
  data = _eval_(data);
  if (!data) return 1;
  switch (data->type) {
  case ATOM: return !strcmp(data_to_string(data), "NIL");
  case CONS: return !strcmp(data_to_string(data), "( )");
  default  : return 0;                          /* gcc warning */
  }
}

int _eq_(Data *d1, Data *d2) {
  return _eval_(d1) == _eval_(d2);
}

int main(int argc, char *argv[]) {
  Atom *atom1 = make_atom("hello-world");
  Atom *atom2 = make_atom("hello-emacs");
  Atom *atom3 = make_atom("hello-linux");
  Data *data1 = make_data(ATOM, atom1);
  Data *data2 = make_data(ATOM, atom2);
  Data *data3 = make_data(ATOM, atom3);
  Cons *cons1 = make_cons(data1, NULL);
  Cons *cons2 = make_cons(data1, data2);
  Cons *cons3 = make_cons(data2, data3);

  printf("%s\n", atom_to_string(atom1));
  printf("%s\n", atom_to_string(atom2));
  printf("%s\n", atom_to_string(atom3));
  printf("%s\n", data_to_string(data1));
  printf("%s\n", cons_to_string(cons1));
  printf("%s\n", cons_to_string(cons2));

  printf("\n==================== expr beg ====================\n");
  printf("cons: %s\n", data_to_string(_cons_(data2, data3)));
  printf("car: %s\n", data_to_string(_car_(make_data(CONS, cons3))));
  printf("cdr: %s\n", data_to_string(_cdr_(make_data(CONS, cons3))));
  printf("quote: %s\n", data_to_string(_quote_(make_data(CONS, cons3))));
  printf("eq: %d\n", (_eq_(make_data(ATOM, make_atom("hello")),
                           make_data(ATOM, make_atom("hello")))));
  /* printf("(read 'hello'): %s\n", data_to_string(read_data("hello"))); */
  /* printf("index: %d\n", read_from_string_index); */
  printf("read('hello'): %s\n", data_to_string(_read_("hello")));
  printf("read('(hello)'): %s\n", data_to_string(_read_("(hello)")));
  printf("read('(hello world)'): %s\n", data_to_string(_read_("(hello world)")));
  printf("\n==================== expr end ====================\n");
  return 0;
}
