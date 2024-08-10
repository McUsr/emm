
/**
 * Copyright (c) 2024 Tommy Bollman <tommy.bollman@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * GNU LPGL 3.0
 */
#ifndef MULTINOM_H
#define MULTINOM_H
#include <stdbool.h>
/* We aren't using yacc so we need to define our own values  for returned datatypes. */

/* For the record: we change the decimal separator with a sed script, since it is a no can
 * do from within lex. */

#define MINUS '-'
#define PLUS '+'
#define PLEFT '('
#define PRIGHT ')'
// TODO: lex: power

/* the convention is that we pass yylval for OPERAND, OPERATOR, and POWER */
#define OPERAND 260
#define OPERATOR 261
#define LEFT_P 262
#define RIGHT_P 263
#define PWR_V 264           /* power value */


/* Can I make the full %union here, or can I declare it in the lex file? */

typedef enum { F_FULL, F_NO_COEFF, F_JUSTVAR, F_JUST_COEFF } content_type; 

/* We pass a record, that is  later to be translated into an array of records? */
typedef enum { typeFact, typeOpr, typePwr } nodeEnum;
/* constants */
typedef struct {
    int coeff;      /* value of constant */
    char var ;
} factNodeType;

/* operators */
typedef struct {
    char oper;       /* operator */
} oprNodeType;


/* power */
typedef struct {
    int pwer;       /* operator */
} pwrNodeType;

typedef struct nodeTypeTag itemData ;

struct nodeTypeTag {
    nodeEnum type;          /* type of node */
    union {
        factNodeType factor; /* A factor contains a coeffecient, maybe with a sign, and a variable name. */
        oprNodeType opr;    /* operators */
        pwrNodeType pwr;
    };
    itemData *next;
} ;

typedef enum { FAIL=0,OK,ACCEPT} validity ;

extern itemData *yylval ;

/* Variables for pointing to syntax errors in the expressions.
 * TODO: tabs are so far surmised to be 8 spaces, this must change to a
 * setting!
 */
#define TABSPACING 8
/* 
 * GLOBAL variables for parsing
 *  We add to ignored_spaces within yylex(), and add yyleng to consumed_text
 *  after the expression has been  validitated after lexing in main().
 */
extern int consumed_text;
extern int ignored_spaces;

/*
 * GLOBAL variables during parsing
 * Freeing memory, if exiting due to syntax errors in yylex();
 */
extern bool PARSING_STAGE; 
extern int nritems;
extern itemData **itemTable;
extern itemData itemsHead;
extern bool NO_PREPROC;
extern char *argstr; /* freed by an atexit routine */
/* MODULE permute.o */
int power(int base, int exp);
int mk_permtable(int nr_vars, int exponent, int **terms_table );
void print_term_tbl(int nr_vars,int exponent, int *terms_table,  int nr_rows );

/* MODULE mk_struct.o */
itemData *newOperator( char *str );
itemData *newPower( char *str );
itemData *newVariable( char *str, int len, content_type what);
void lexer_exit(void);

/* MODULE syntax_err.o */
void syntax_err(const char * const details) ;
void syntax_err2(const char * const details1,const char * const details2);

/* MODULE finitestate.o */
validity validator( int item_type, int *nrvars, int *nrops, int *nritems);

/* MODULE vartables.o */
int make_vartables(int nritems,itemData **itemTable, int nrvars, int nrops,
        char **vars, int **coeffs, char **ops);
void free_vartables(char **vars, int **coeffs, char **ops);

/* MODULE expand_expr.o */
void expand_expr(int terms_rows, int nr_vars, int *terms_table,
        char *vartable, int *coefftbl);
void adjust_coeffs(int nr_vars,int *coefftbl, char *optbl);

/* MODULE arguments.o */

typedef enum { OPT_BAD= -1,OPT_NONE=0,OPT_HELP, OPT_PREPROCESS} opt_tp;

void show_usage( char *prog_name);
void show_help(void );
int print_cmdln_child( int argc, char *argv[], int treshold, int *consumed, int arg_start );
int print_cmdln_parent( int argc, char *argv[], int arg_start);
int save_cmdln_parent( int argc, char *argv[], int treshold, int arg_start);
void argstr_free(void);
int options( int argc, char *argv[] );

#endif

