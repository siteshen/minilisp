typedef enum   Type { ATOM, CONS } Type;
typedef struct Atom { char *name; } Atom;
typedef struct Cons { struct Data *car; struct Cons *cdr; } Cons;
typedef struct Data { Type type; void *data; } Data;

Atom *empty_atom();
Cons *empty_cons();
Data *empty_data();

Atom *make_atom(char *str);
Cons *make_cons(Data *car, Cons *cdr);
Data *make_data(Type type, void *data);

char *copy_str(char *str);
Atom *copy_atom(Atom *atom);
Cons *copy_cons(Cons *cons);
Data *copy_data(Data *data);

char *atom_to_string(Atom *atom);
char *cons_to_string(Cons *cons);
char *data_to_string(Data *data);
