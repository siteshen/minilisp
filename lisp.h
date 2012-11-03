typedef enum   Type { ATOM, CONS } Type;
typedef struct Atom { char *name; } Atom;
typedef struct Cons { struct Data *car; struct Data *cdr; } Cons;
typedef struct Data { Type type; void *data; } Data;

Atom *empty_atom();
Cons *empty_cons();
Data *empty_data();

Data *make_atom(char *str);
Data *make_cons(Data *car, Data *cdr);
Data *make_data(Type type, void *data);

char *copy_str(char *str);
Atom *copy_atom(Atom *atom);
Cons *copy_cons(Cons *cons);
Data *copy_data(Data *data);

char *atom_to_string(Atom *atom);
char *cons_to_string(Cons *cons, int racket);
char *data_to_string(Data *data);

Data *read_atom(char *str, int beg);
Data *read_cons(char *str, int beg);
Data *read_from(char *str, int beg);
Data *read_data(char *str);

Data *_eval_(Data *data);
Data *_cons_(Data *car, Data *cdr);
Data *_car_(Data *data);
Data *_cdr_(Data *data);
Data *_quote_(Data *data);
Data *_if_(Data *_if, Data *_then, Data *_else);
Data *_atom_(Data *data);
Data *_eq_(Data *d1, Data *d2);

Data *_assoc_(Data *key, Data *pair);

static Data *Qnil, *Qt, *Qquote;

int NILP(Data *data);
int LAMBDAP(Data *data);
