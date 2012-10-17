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
Cons *make_cons(Data *car, Cons *cdr) {
  Cons *cons = empty_cons();
  cons->car = copy_data(car);
  cons->cdr = copy_cons(cdr);
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
  cdr_str = cons_to_string(cons->cdr);
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
  }
}
/* }}} */


int main(int argc, char *argv[]) {
  Atom *atom1 = make_atom("hello-world");
  Atom *atom2 = make_atom("hello-emacs");
  Atom *atom3 = make_atom("hello-linux");
  Data *data1 = make_data(ATOM, atom1);
  Data *data2 = make_data(ATOM, atom2);
  Cons *cons1 = make_cons(data1, NULL);
  Cons *cons2 = make_cons(data2, cons1);

  printf("%s\n", atom_to_string(atom1));
  printf("%s\n", atom_to_string(atom2));
  printf("%s\n", atom_to_string(atom3));
  printf("%s\n", data_to_string(data1));
  printf("%s\n", cons_to_string(cons1));
  printf("%s\n", cons_to_string(cons2));
  return 0;
}
