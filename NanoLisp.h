//
//  NanoLisp.h
//  nanoLisp
//
//  Created by Jean-Pierre De Soza on 15/07/2021.
//

#ifndef NanoLisp_h
#define NanoLisp_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <math.h>

#define BANNER "NanoLisp-V0.0.1 - by jpds - 2021\n"

#define MAX_STRING_LENGTH 255
#define LF      0x10
#define SPC     0x20
#define QUOTE_S 0x27
#define SMC     ';'
#define PAREN_L '('
#define PAREN_R ')'
#define TILDE   '~'

// Macros
#define DBG_TRACE(x) if (TRACE) (x)

#define NULLP(S) (((S) == NULL) || ((S) == NIL))
#define QUOTEP(S) ((S) == QUOTE)
#define ATOMP(S) ((NULLP(S) || (S)->kind == ATOM))
#define LISTP(S) ((!NULLP(S)) && ((S)->kind == LIST))
#define NUMBERP(S) (isNumeric(NAME_OF((S))))
#define AUTOEVAL(S) ((!NULLP((S))) && ((S)==VALUE_OF((S))))
#define VARIABLEP(S) ((!NULLP(S)) && (!(NUMBERP((S)))) && ATOMP((S)) && (AUTOEVAL(VALUE_OF((S)))))

#define NAME_OF(s)  ((s)->unode.atom->name)
#define VALUE_OF(s) ((s)->unode.atom->val)

#define TRACE GLOBAL_ENV._trace
#define ERROR GLOBAL_ENV._error
#define DONE GLOBAL_ENV._done
#define OBLIST  GLOBAL_ENV._oblist
#define T       GLOBAL_ENV._true
#define NIL     GLOBAL_ENV._nil
#define QUOTE   GLOBAL_ENV._quote
#define LAMBDA  GLOBAL_ENV._lambda
#define MUTE    GLOBAL_ENV._mute

#define BUFFER  GLOBAL_ENV._buffer
#define INDEX   GLOBAL_ENV._index
#define S_CONSOLE GLOBAL_ENV._console
#define SOURCE  GLOBAL_ENV._source
#define EPSILON GLOBAL_ENV._epsilon

#define CONSOLE NAME_OF(S_CONSOLE)

#define PROMPT1 "\t" 
#define PROMPT2 "" 
#define PROMPT3 "" 
#define FMT_NO_SPC "%s"
#define FMT_SPC "%s "

typedef unsigned char KindOfNode;
typedef struct struct_object* PtObList;
typedef struct struct_node* Sexp;
typedef struct struct_atom* Atom;
typedef struct struct_list* List;
typedef struct struct_env* Env;
typedef char* String;

enum KindOfNode {ATOM, LIST};
enum KindOfToken {L_PAREN, R_PAREN, S_QUOTE, SYMBOL, COMMENT};
enum ArithmOp { PLUS, MINUS, MULT, DIV };
enum CompOp { EQU, NEQU, GT, LT, GE, LE };


typedef struct struct_atom {
    String name;
    Sexp val;
} atom_t;

typedef struct struct_list {
    Sexp car;
    Sexp cdr;
} list_t;

typedef struct struct_node {
    enum KindOfNode kind;
    union {
        Atom atom;
        List list;
    } unode;
    // PtObList plist;
} node_t;

typedef struct struct_object {
    Sexp info;
    PtObList next;
} object_t ;

// Globales
typedef struct struct_env {
    bool _error;
    bool _done;
    bool _trace;
    PtObList _oblist;
    unsigned int _index;
    char _buffer[MAX_STRING_LENGTH];
    float _epsilon;
    FILE *_source;
    Sexp _nil;
    Sexp _true;
    Sexp _quote;
    Sexp _lambda;
    Sexp _console;
    Sexp _mute;
    Sexp _zero;
    Sexp _one;
} env_t;

extern env_t GLOBAL_ENV;
extern Sexp ZERO;
extern Sexp ONE;

// Function prototypes

// Utility functions
int isNumeric (const char *s);
bool is_long_integer(double d);
void setup(void);
void s_swap(Sexp *s1, Sexp *s2);
void s_pair_list(Sexp *names, Sexp *values);
PtObList new_atom(PtObList position, String name);
Sexp find_sexp2(const PtObList position, String name);
Sexp find_sexp(String name);

// Interfaceable functions : Sexp only, denoted by f_ names
Sexp f_car(Sexp s) ;
Sexp f_cdr(Sexp s) ;
Sexp f_setq(Sexp s);
Sexp f_cons(Sexp s1, Sexp s2);
Sexp f_de(Sexp s);
Sexp f_eq(Sexp s1, Sexp s2);
void f_print(Sexp s);
void f_oblist(void);
Sexp f_read(FILE * source);
Sexp f_load(Sexp filename);
Sexp f_eval(Sexp s);
Sexp f_apply(Sexp fn, Sexp args);
Sexp f_atom(Sexp s);
Sexp f_cond(Sexp s);
Sexp f_eval_list(Sexp s);

// Arithmetics and Mathematics
Sexp f_opari(enum ArithmOp op, const Sexp s);
Sexp f_add(const Sexp s);
Sexp f_sub(const Sexp s);
Sexp f_mult(const Sexp s);
Sexp f_div(const Sexp s);

// Could be but is not exposed
Sexp f_eval_list_return_last(Sexp s);

// Error routine
Sexp error(String message, Sexp origin);

// Display function
void print1(Sexp s, String format);
void print_atom(Sexp s, String format);
void obprint(const PtObList start);

// Input function
char read_one_char(FILE *source);
Sexp read_atom(char buffer[], enum KindOfToken *token, FILE *source);
Sexp read1(char buffer[], bool list_in_progress, FILE *source);
void ignore_end_of_line(FILE *source);
#endif /* NanoLisp_h */
