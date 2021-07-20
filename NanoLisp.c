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
    current = new_atom(current, "READ");
    current = new_atom(current, "PRINT");
    current = new_atom(current, "COND");
    current = new_atom(current, "TRACE");
    current = new_atom(current, "UNTRACE");
    current = new_atom(current, "SETQ");
    current = new_atom(current, "LOAD");
    current = new_atom(current, "OBLIST");
    current = new_atom(current, "QUIT");
    S_CONSOLE = new_atom(NULL, "CONSOLE")->info;
}

// Interfaceable functions based on Sexp (some could be macros)

// THE "READ" FUNCTION
Sexp f_load(Sexp filename) {
    FILE *infile;
    String file_name;
    Sexp s1, s2;
    
    file_name = NAME_OF(filename);
    if (TRACE) {
        printf("\n\tLoading~~>%s\n", file_name);
    }
    if (ATOMP(filename) && !NULLP(filename)) {
        if (strcmp(file_name, CONSOLE) == 0) {
            infile = stdin;
        } else {
            infile = fopen(file_name, "r");
        }
        printf(PROMPT1);
        do {
            s1 = f_read(infile);
            printf(PROMPT2);
            s2 = f_eval(s1);
            printf(PROMPT3);
            f_print(s2);
            // DBG_TRACE(printf("%s",NAME_OF(s2)));
            printf("\n%s", PROMPT1);
        } while (!(feof(infile)
                || ERROR
                || DONE));
        fclose(infile);
        return T;
    } else {
        return f_error("f_load", filename);
    }
}

// THE EVAL FUNCTION

Sexp f_eval(Sexp e) {
    Sexp s;
    String nameOfS;
    s = f_car(e);
    nameOfS = NAME_OF(s);
    if (!NULLP(s)) {
        DBG_TRACE(f_print(s));
        if (NUMBERP(s)) { return s; } else
        if (strcmp(nameOfS, "TRACE") == 0) { TRACE = true; return NIL; } else
        if (strcmp(nameOfS, "UNTRACE") == 0) { TRACE = false; return NIL; } else
        if (strcmp(nameOfS, "QUIT") == 0) { DONE = true; return NIL; } else
        if (strcmp(nameOfS, "OBLIST") == 0) { f_oblist(); return NIL; } else
        if (strcmp(nameOfS, "QUOTE") == 0) { return e; } else
            return f_apply(s, f_eval(f_cdr(s)));
    }
    return NIL;
}

// THE APPLY FUNCTION
Sexp f_apply(Sexp fn, Sexp args) {
    DBG_TRACE(printf("Apply: "));
    DBG_TRACE(f_print(fn));
    DBG_TRACE(printf("~~>"));
    DBG_TRACE(f_print(args));
    DBG_TRACE(printf("\n"));
    if (strcmp(NAME_OF(f_car(fn)), "CAR") == 0) { return f_car(args); } else
    if (strcmp(NAME_OF(f_car(fn)), "CDR") == 0) { return f_cdr(args); } else
    return NIL;
}

// PRINT FUNCTION
void f_print(const Sexp s) {
    if (LISTP(s) && !QUOTEP(f_car(s))) {
        printf("%c", PAREN_L);
    }
    print1(s,"%s");
}

// PRIMITIVES

Sexp f_car(Sexp s) {
    return ((!NULLP(s)) ? (LISTP(s) ? (s->unode.list->car): ((NUMBERP(s))? s : NIL)): NIL);
}
Sexp f_cdr(Sexp s) {
    return ((!NULLP(s)) ? (LISTP(s) ? (s->unode.list->cdr): ((NUMBERP(s))? s : NIL)): NIL);
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

// Name and Function binding
Sexp f_de(Sexp s) {
    // car(s) has already a name, parsed by EVAL
    // Its value will be the rest of the definition, with LAMBDA replacing the name.
    VALUE_OF(s) = f_cons(LAMBDA, f_cdr(s));
    return s;
}

// List constructor
Sexp f_cons(Sexp s1, Sexp s2) {
    Sexp new_list;
    if (!(NULLP(s2) || NUMBERP(s2) || LISTP(s2))) {
        return f_error("cons", s2);
    } else if (NUMBERP(s2)) {
        return f_cons(s1, f_cons(s2, NIL));
    } else {
        new_list = malloc(sizeof(node_t));
        new_list->kind = LIST;
        new_list->unode.list = malloc(sizeof(node_t));
        new_list->unode.list->car = s1;
        new_list->unode.list->cdr = s2;
        return new_list;
    }
}

// Dump of global Sexp list
void f_oblist(void) {
    obprint(OBLIST);
    printf("\n");
}

///////////////////////////////////////////////////////////
// Utility function: Borrowed from RosettaCode.org
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
    new->info->plist = NULL;
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
// Print out an atom's name
void print_atom(Sexp s, String format) {
    printf(format, NAME_OF(s));
}

void print1(Sexp s, String format) {
    if (ATOMP(s)) {
        print_atom(s, "%s");
        format = "%s ";
    } else if (LISTP(s)) {
        if (QUOTEP(f_car(s))) {
            printf("%c", QUOTE_S);
            // Startover with the rest of the list
            f_print(f_car(f_cdr(s)));
        } else {
            print1(f_car(s), format);
            // Was that the last atom in the list ?
            if (NULLP(f_cdr(s))) {
                printf("%c", PAREN_R);
            } else {
                // Keep going
                print1(f_cdr(s), format);
            }
        }
    }
}

// Dump of given Sexp list
void obprint(const PtObList start) {
    if (start != NULL) {
        f_print(start->info);
        printf("%c", SPC);
        obprint(start->next);
    }
}

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
                return f_cons(QUOTE, f_cons(r1, r2));
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
        !feof(source)
        && ((buffer[0] <= SPC)
        || (buffer[0] > TILDE)) // UTF-8 scories
        ) {
            buffer[0] = read_one_char(source);
        };
        while (!(feof(source)
        || (buffer[0] == PAREN_L)
        || (buffer[0] == PAREN_R)
        || (buffer[0] == QUOTE_S)
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
    INDEX++;
    // DBG_TRACE(printf("%d/%ld\n", INDEX, strlen(BUFFER)));
    if (INDEX == strlen(BUFFER))
        return SPC;
    else if (INDEX > strlen(BUFFER)) {
        // Read a new buffer
        fscanf(source, "%s", BUFFER);
        //fgets(BUFFER, MAX_STRING_LENGTH, source);
        // Read and ignore the carriage return
        ignored_cr = fgetc(source);
        INDEX = 0;
        if (TRACE) {
            int i;
            printf("%ld ", strlen(BUFFER));
            for (i = 0; i <= strlen(BUFFER);i++) {
                printf("'%c'(%u) | ", BUFFER[i], BUFFER[i]);
            }
            printf("\n");
        }
    }
    return BUFFER[INDEX];
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
// Exception routine
Sexp f_error(String message, Sexp origin) {
    printf("****** %s ", message);
    f_print(origin);
    printf("******\n");
    ERROR = true;
    return NIL;
}

