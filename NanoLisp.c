//
//  NanoLisp.c
//  nanoLisp
//
//  Created by Jean-Pierre De Soza on 15/07/2021.
//
#include <stdlib.h>
#include <ctype.h>
#include <stdlib.h>
#include "NanoLisp.h"

// Setup of environment
void setup(void) {
    PtObList current;
    
    // Auto-Evaluated Atoms T and NIL : val == self
    current = new_atom(NULL, "()"); NIL = current->info; VALUE_OF(NIL) = NIL;
    OBLIST = current;
    current = new_atom(current, "T"); T= current->info; VALUE_OF(T) = T;
    current = new_atom(current, "QUOTE"); QUOTE = current->info;
    current = new_atom(current, "CAR");
    current = new_atom(current, "CDR");
    current = new_atom(current, "CONS");
    current = new_atom(current, "LAMBDA"); LAMBDA = current->info;
    current = new_atom(current, "ATOM");
    current = new_atom(current, "READ");
    current = new_atom(current, "PRINT");
    current = new_atom(current, "COND");
    current = new_atom(current, "TRACE");
    current = new_atom(current, "UNTRACE");
    current = new_atom(current, "SETQ");
    current = new_atom(current, "LOAD");
    current = new_atom(current, "OBLIST");
    current = new_atom(current, "QUIT");
    current = new_atom(current, ""); MUTE = current->info;
    S_CONSOLE = new_atom(NULL, "CONSOLE")->info;
}

// Interfaceable functions based on Sexp (some could be macros)

// THE "READ" FUNCTION
Sexp f_load(Sexp filename) {
    String file_name;
    Sexp s1, s2;
    
    file_name = NAME_OF(filename);
    if (TRACE) {
        printf("\n\tLoading~~>%s", file_name);
    }
    if (ATOMP(filename) && !NULLP(filename)) {
        if (strcmp(file_name, CONSOLE) == 0) {
            SOURCE = stdin;
        } else {
            SOURCE = fopen(file_name, "r");
            if (!SOURCE) {
                perror("fopen");
                return error(strerror(errno), filename);
            }
        }
        printf("\n%s",PROMPT1);
        do {
            s1 = f_read(SOURCE);
            printf("\n%s",PROMPT2);
            s2 = f_eval(s1);
            f_print(s2);
            printf("\n%s", PROMPT1);
        } while (!(feof(SOURCE)
                || ERROR
                || DONE));
        fclose(SOURCE);
        return T;
    } else {
        return error("LOAD", filename);
    }
}

// THE EVAL FUNCTION

Sexp f_eval(Sexp e) {
    Sexp s;
    String nameOfS;
    DBG_TRACE(printf("\nEval: "));
    DBG_TRACE(f_print(e));
    if (ERROR)
        return e;
    else if (ATOMP(e)) {
        if (NULLP(VALUE_OF(e)))
            return NIL;
        else
            return VALUE_OF(e);
    } else {
        s = f_car(e);
        nameOfS = NAME_OF(s);
        if (ATOMP(s)) {
            if (NUMBERP(s) || VARIABLEP(s)) {
                return f_cons(s, f_eval_list(f_cdr(e)));
            } else
                if (strcmp(nameOfS, "QUOTE") == 0) { return f_car(f_cdr(e)); } else
                if (strcmp(nameOfS, "COND") == 0) { return f_cond(f_cdr(e)); } else
                if (strcmp(nameOfS, "TRACE") == 0) { TRACE = true; return MUTE; } else
                if (strcmp(nameOfS, "UNTRACE") == 0) { TRACE = false; return MUTE; } else
                if (strcmp(nameOfS, "QUIT") == 0) { DONE = true; return MUTE; } else
                if (strcmp(nameOfS, "OBLIST") == 0) { f_oblist(); return MUTE; } else
                if (strcmp(nameOfS, "SETQ") == 0) { return f_setq(f_cdr(e)); } else
                if (strcmp(nameOfS, "DE") == 0) { return f_de(f_cdr(e)); } else
                if (strcmp(nameOfS, "READ") == 0) { return f_read(SOURCE); } else
                // Apply first item (as a function) to rest of items (as arguments)
                return f_apply(s, f_eval_list(f_cdr(e)));
        }
        return f_apply(s, f_eval_list(f_cdr(e)));
    }
}

// THE <APPLY A FUNCTION> FUNCTION

Sexp f_apply(Sexp fn, Sexp args) {
    DBG_TRACE(printf("\n Apply: "));
    DBG_TRACE(f_print(fn));
    DBG_TRACE(printf("~~>"));
    DBG_TRACE(f_print(args)); ;
    DBG_TRACE(printf("\n"));
    if (NULLP(fn)) {
        return NIL;
    } else if (ATOMP(fn)) {
        if (strcmp(NAME_OF(fn), "CAR") == 0) { return f_car(f_car(args)); } else
        if (strcmp(NAME_OF(fn), "CDR") == 0) { return f_cdr(f_car(args)); } else
        if (strcmp(NAME_OF(fn), "CONS") == 0) { return f_cons(f_car(args),f_car(f_cdr(args))); } else
        if (strcmp(NAME_OF(fn), "EQ") == 0) { return f_eq(f_car(args), f_car(f_cdr(args))); } else
        if (strcmp(NAME_OF(fn), "ATOM") == 0) { return f_atom(f_car(args)); } else
        if (strcmp(NAME_OF(fn), "LIST") == 0) { return f_eval_list(args); } else
        if (strcmp(NAME_OF(fn), "PRINT") == 0) { f_print(f_car(args)); return MUTE; } else
        if (strcmp(NAME_OF(fn), "EVAL") == 0) {
            if (QUOTEP(f_car(f_car(args))))
                return f_eval(f_car(f_cdr(f_car(args))));
            else
                return f_eval(f_car(args));
        } else
        if (strcmp(NAME_OF(fn), "LOAD") == 0) {
            if (QUOTEP(f_car(f_car(args))))
                return f_load(f_car(f_cdr(f_car(args))));
            else
                return f_load(f_car(args));
        } else
            return f_apply(f_eval(fn), args);
    } else if (LISTP(fn)) {
        if (f_car(fn) == LAMBDA) {
            Sexp last;
            s_pair_list(&(fn->unode.list->cdr->unode.list->car),&args);
            last = f_eval_list_return_last(f_cdr(f_cdr((fn))));
            s_pair_list(&(fn->unode.list->cdr->unode.list->car),&args);
            return last;
        }
        if (f_car(fn) == QUOTE) {
            return f_cons(fn, args);
        }
    }
    // Error path
    return error("APPLY", fn);
}

// PRINT FUNCTION
void f_print(const Sexp s) {
    if (LISTP(s) && !QUOTEP(f_car(s))) {
        printf("%c", PAREN_L);
    }
    // Print inside of parens
    print1(s,FMT_SPC);
}

// PRIMITIVES

Sexp f_car(Sexp s) {
    if (NULLP(s))
        return NIL;
    else
        if (LISTP(s))
            return s->unode.list->car;
        else
            if (ATOMP(s) || NUMBERP(s))
                return s;
            else
                return error("CAR", s);
}
Sexp f_cdr(Sexp s) {
    if (NULLP(s))
        return NIL;
    else
        if (LISTP(s))
            return s->unode.list->cdr;
        else
            if (NUMBERP(s))
                return NIL;
            else
                return error("CDR", s);
}
// Input from Console or File
Sexp f_read(FILE * source) {
    char buffer[1];
    buffer[0] = SPC;
    return read1(buffer, false, source);
}

Sexp f_atom(Sexp s) {
    return (ATOMP(s)) ? T : NIL;
}

Sexp f_eq(Sexp s1, Sexp s2) {
    return (s1 == s2) ? T : NIL;
}

Sexp f_neq(Sexp s1, Sexp s2) {
    return (s1 != s2) ? T : NIL;
}

Sexp f_setq(Sexp s) {
    Sexp variable, values, temp_values;
    variable = f_car(s);
    values = f_cdr(s);
    temp_values = f_eval_list(values);
    if (ATOMP(variable) && !NULLP(variable)) {
        VALUE_OF(variable) = f_car(temp_values);
        DBG_TRACE(printf("\nSETQ %s <- ",NAME_OF(variable)));
        DBG_TRACE(f_print(temp_values));
        DBG_TRACE(printf("\n"));
        return VALUE_OF(variable);
    } else {
        return error("SETQ", s);
    }
}

// Name and Function binding
Sexp f_de(Sexp s) {
    // car(s) has already a name, parsed by EVAL
    // Its value will be the rest of the definition, with LAMBDA replacing the name.
    if (LISTP(s)) {
        Sexp func_name, definition;
        func_name = f_car(s);
        definition = f_cdr(s);
        VALUE_OF(func_name) = f_cons(LAMBDA, definition);
        return f_car(s);
    } else {
        return error("DE",s);
    }
}

// List constructor
Sexp f_cons(Sexp s1, Sexp s2) {
    Sexp new_list;
    if (!(NULLP(s2) || NUMBERP(s2) || LISTP(s2))) {
        return error("cons", s2);
    } else if (NUMBERP(s2)) {
        return f_cons(s1, f_cons(s2, NIL));
    } else {
        new_list = malloc(sizeof(node_t));
        new_list->kind = LIST;
        new_list->unode.list = malloc(sizeof(list_t));
        new_list->unode.list->car = s1;
        new_list->unode.list->cdr = s2;
        return new_list;
    }
}

// Conditional evaluation
Sexp f_cond(Sexp s) {
    if (!NULLP(f_eval(f_car(f_car(s))))) {
        return f_eval_list_return_last(f_cdr(f_car(s)));
    } else {
        if (!NULLP(f_cdr(s))) {
            return f_cond(f_cdr(s));
        }
    }
    return NIL;
}
// Dump of global Sexp list
void f_oblist(void) {
    obprint(OBLIST);
    printf("\n");
}

// Utility functions
// Input
Sexp read1(char buffer[], bool list_in_progress, FILE *source) {
    Sexp new_symbol, r1, r2;
    enum KindOfToken token;
    new_symbol = read_atom(buffer, &token, source);
    switch (token) {
        case L_PAREN:
            if (list_in_progress) {
                r1 = read1(buffer, true, source);
                r2 = read1(buffer, true, source);
                return f_cons(r1, r2);
            } else {
                return read1(buffer, true, source);
            }
            break;
        case R_PAREN:
            return NIL;
            break;
        case S_QUOTE:
            if (list_in_progress) {
                r1=read1(buffer, false, source);
                r2=read1(buffer, true, source);
                return f_cons(f_cons(QUOTE,f_cons(r1, NIL)),r2);
            } else {
                r1=read1(buffer, false, source);
                return f_cons(QUOTE, f_cons(r1, NIL));
            }
            break;
        case SYMBOL:
            if (list_in_progress) {
                r1 = read1(buffer, true, source);
                return f_cons(new_symbol, r1);
            } else
                return new_symbol;
            break;
        case COMMENT:
            ignore_end_of_line(source);
            return MUTE;
            break;
    }
}
//
Sexp read_atom(char buffer[], enum KindOfToken *token, FILE *source) {
    String new_name;
    Sexp new_symbol;
    char pad;
    char *current_char;
    bool sep_found;
    
    new_name = malloc(MAX_STRING_LENGTH);
    current_char = new_name;
    *current_char ='\0';
    pad = SPC;
    sep_found = false;
    
    if (buffer[0] == SPC) {
        // Trim off spaces, line feeds, tabs
        while (
        (!feof(source))
        && ((buffer[0] <= SPC)
        || (buffer[0] > TILDE)) // UTF-8 scories
        ) {
            buffer[0] = read_one_char(source);
        };
        while (!(feof(source)
        || (buffer[0] == PAREN_L)
        || (buffer[0] == PAREN_R)
        || (buffer[0] == QUOTE_S)
        || (buffer[0] == SMC)
        || (buffer[0] <= SPC)
        || ((current_char - new_name) > MAX_STRING_LENGTH))) {
            pad = buffer[0];
            *current_char++ = pad;
            buffer[0] = read_one_char(source);
        };
    };
   
    if (strlen(new_name) == 0) {
        pad = buffer[0];
        *current_char++ = pad;
        buffer[0] = SPC;
    }
    // Terminate string;
    *current_char ='\0';
    new_symbol = NIL;
    switch(*new_name) {
        case PAREN_L:
            *token = L_PAREN;
            sep_found = true;
            break;
        case PAREN_R:
            *token = R_PAREN;
            sep_found = true;
            break;
        case QUOTE_S:
            *token = S_QUOTE;
            sep_found = true;
            break;
        case SMC:
            *token = COMMENT;
            sep_found = true;
            break;
    }
    if (!sep_found) {
        *token = SYMBOL;
        new_symbol = find_sexp(new_name);
        if (NULLP(new_symbol))
            new_symbol = new_atom(OBLIST, new_name)->info;
    }
    return new_symbol;
}
// Input routines
char read_one_char(FILE *source) {
    char ignored_cr;
    int i;
    INDEX++;
    if (TRACE) {
        printf("\n«%s»\n", BUFFER);
        for (i = 0; i < INDEX; i++)
            printf("%c",SPC);
        printf(" ^\n");
        for (i = 0; i < INDEX; i++)
            printf("%c",SPC);
        printf(" |\n");
    }
    if (INDEX == strlen(BUFFER)) {
        return SPC;
    } else if (INDEX > strlen(BUFFER)) {
        if (feof(source)) {
            fclose(source);
            source = stdin;
            INDEX = MAX_STRING_LENGTH;
            return SPC;
        }
        // Read a new buffer
        fscanf(source, "%s", BUFFER);
        // Read and ignore the carriage return
        INDEX = 0;
        ignored_cr = fgetc(source);
    }
    return BUFFER[INDEX];
}

// Read and discard input to the end of line, only used for comments.
void ignore_end_of_line(FILE *source) {
    while (fgetc(source) >= SPC);
}
// Print out an atom's name
void print_atom(Sexp s, String format) {
    printf(format, NAME_OF(s));
}
// Print a list, less the first left paren
void print1(Sexp s, String format) {
    if (NULLP(s))
        print_atom(NIL, format);
    else if (ATOMP(s)) {
        print_atom(s, format);
    } else {
        if (QUOTEP(f_car(s))) {
            printf("%c", QUOTE_S);
            // Print quoted symbol, could be atom or list
            f_print(f_car(f_cdr(s)));
        } else {
            if((!NULLP(f_car(s)))
               && LISTP(f_car(s))
               && (!QUOTEP(f_car(f_car(s)))))
                printf("%c",PAREN_L);
            print1(f_car(s), FMT_SPC);
            if (NULLP(f_cdr(s)))
                printf("%c",PAREN_R);
            else
                print1(f_cdr(s), FMT_SPC);
        }
    }
}


// This one: Borrowed from RosettaCode.org
int isNumeric (const char *s)
{
    if (s == NULL || *s == '\0' || isspace(*s))
      return 0;
    char *p;
    strtod (s, &p);
    return *p == '\0';
}

// Create new entry into given Sexp list
PtObList new_atom(const PtObList position, String name) {
    PtObList prev, new;
    prev = position;
    
     // Add new at bottom of the list
    while(prev != NULL && prev->next != NULL)
        prev = prev->next;
    
    new = malloc(sizeof(object_t));
    new->info = malloc(sizeof(node_t));
    // new->info->plist = NULL;
    new->info->kind = ATOM;
    new->info->unode.atom = malloc(sizeof(atom_t));
    NAME_OF(new->info) = name;
    // Make a number an auto-evaluated, value = self
    if (NUMBERP(new->info))
        VALUE_OF(new->info) = new->info;
     else
        VALUE_OF(new->info) = NULL;
    
    // Update old & new position (or not)
    if (prev != NULL)
        prev->next = new;
    if (new !=NULL)
        new->next = NULL;
    DBG_TRACE(printf("Atome créé: %s\n", NAME_OF(new->info)));
    return new;
}

// Dump of given Sexp list
void obprint(const PtObList position) {
    PtObList start;
    start = position;
    while (start != NULL) {
        f_print(start->info);
        start = start->next;
    }
}


// Search in given sexp list
Sexp find_sexp2(const PtObList position, String name) {
    PtObList current;
    current = position;
    while ((current != NULL) && (strcmp(name, NAME_OF(current->info)) != 0)) {
        current = current->next;
    }
    if (current == NULL)
        return NIL;
    else
        return current->info;
}
// Search in global sexp list
Sexp find_sexp(String name) {
    return find_sexp2(OBLIST, name);
}

// List of evals of each item in a list
Sexp f_eval_list(Sexp args) {
    if (NULLP(args))
        return NIL;
    else {
        return f_cons(f_eval(f_car(args)), f_eval_list(f_cdr(args)));
    }
}

// Last in evals of each item in a list
Sexp f_eval_list_return_last(Sexp s) {
    Sexp eval, sexp;
    if (NULLP(s))
        return NIL;
    else {
        eval = NIL;
        for (sexp = s;!(ERROR || (sexp == NIL)); sexp = f_cdr(sexp))
            eval = f_eval(f_car(sexp));
        return eval;
    }
}

// swap values of 2 Sexps
void s_swap(Sexp *s1, Sexp *s2) {
    Sexp temp;
    temp = *s1;
    *s1 = *s2;
    *s2 = temp;
}
// Used only for LAMBDA evaluation,
// to inject actual values into formal arguments
void s_pair_list(Sexp *names, Sexp *values) {
    if (ERROR)
        return;
    if (LISTP(*names)) {
        s_pair_list(&((*names)->unode.list->cdr), &((*values)->unode.list->cdr));
        s_swap(&((*names)->unode.list->car->unode.atom->val), &((*values)->unode.list->car));
    } else {
        if (!NULLP(*names)) {
            s_swap(&((*names)->unode.atom->val), values);
        }
    }
}
// Exception routine
Sexp error(String message, Sexp origin) {
    printf("****** %s ", message);
    f_print(origin);
    printf("******\n");
    ERROR = true;
    return NIL;
}
