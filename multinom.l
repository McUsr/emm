/*
Second  lex program in "guide_Lex_yacc.pdf"
        using substitutions, maybe figuring out the right incantation to get the coeffecient.

First: Made to take input from the command line. (hopefully).
   tags: lex yacc argv arguments dup dup2
 */
%{

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <limits.h>
#include "multinom.h"

#define YY_BUF_SIZE 1024
/* There will still be a small amount unreachable memory! */
/* TODO: freeing of memory from within lex script dictates global variables
so that we can deallocate whatever memory has been allocated so far.
*/
/* u/daikatana */
static void log_msg(const char *format, ...) ;

#define LOG(FMT, ...) \
  log_msg(__FILE__":%d %s "FMT, __LINE__,__func__ __VA_OPT__(,)__VA_ARGS__)

static bool doprint=false;
static void log_msg(const char *format, ...) {
  va_list args;
  va_start(args, format);
  if (doprint)
      vfprintf(stderr, format, args);
  va_end(args);
}
int consumed_text=0, ignored_spaces=0;
int nritems=0;
bool PARSING_STAGE=false; 
nodeType **nodeTable;
nodeType *yylval ;
%}
sign    [-+]{1}
digits  [0-9]+
letter  [A-Za-z]
power "^"[0-9]+
leftp "("
rightp ")"

%%
    /* rules */
{sign}{digits}{letter} | 
{digits}{letter}        {
                            yylval = newVariable(yytext,yyleng,F_FULL);
                            return OPERAND ;
                        }
{sign}{letter}          {
                            yylval = newVariable(yytext,yyleng,F_NO_COEFF);
                            return OPERAND ;
                        }
{letter}                {
                            yylval = newVariable(yytext,yyleng,F_JUSTVAR);
                            return OPERAND ;
                        }
{sign}{digits}          |
{digits}                {
                            yylval = newVariable(yytext,yyleng,F_JUST_COEFF);
                            return OPERAND ;
                        }
{sign}                  {   
                            yylval = newOperator(yytext) ;
                            return OPERATOR ;
                        }
{leftp}                 { 
                            return LEFT_P;
                        }
{rightp}                {
                            return RIGHT_P ;
                        }
{power}                 {
                            yylval = newPower(yytext);
                            return PWR_V ;
                        } 
[ ]{1}                  { ignored_spaces++ ; }                     
[\t]{1}                 { ignored_spaces+= TABSPACING;}
[\n]+ ;

.                       { 
                            fflush(stdout);
                            fprintf(stderr,"\n%*s^\n",(consumed_text+ignored_spaces)," ");
                            fprintf(stderr,"%*sSyntax error: Illegal character.\n",(consumed_text+ignored_spaces)," ");
                            exit(EXIT_FAILURE);
                        }

%%

int yywrap(void) {
    return 1;
}

int main( int argc, char *argv[]  )
{
    int wc_pid;
    int wc_pfd[2];
    FILE *in;

    if (pipe( wc_pfd ) < 0 ) {
        fprintf(stderr, "Can't set up the pipe: %s\n",strerror(errno));
        fprintf(stderr,"Exiting!\n");
        exit(EXIT_FAILURE);
    }

    wc_pid = fork(  );
    if ( wc_pid == 0 ) {
        close( wc_pfd[0] );
        dup2( wc_pfd[1], 1 );

        argv++;
        for (int i = argc ; i>1;i--) {
            printf("%s ", *argv );
            argv++;
        }
        printf("\n") ;
        return 0;
    }
    waitpid( wc_pid, NULL, 0 );
    close( wc_pfd[1] );
    in = fdopen( wc_pfd[0], "r" );
    if (dup2(fileno(in),0) == -1) {
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
    int treshold = YY_BUF_SIZE ;
    char *argstr=calloc((YY_BUF_SIZE+1),1); ;
    if ( argstr == NULL ) {
        fprintf(stderr,"argstr: Out of memory, exiting\n");
        exit(EXIT_FAILURE);
    }
    argv++;
    for (int i = argc ; i>1;i--) {
        if (i<argc) {
            argstr=strcat(argstr," ");
        }
        argstr=strncat(argstr,*argv,treshold);
        treshold -= strlen(*argv);
        argv++;
    }
    int arrlen = strlen(argstr) -3 ; 
    /*  can trim whitespace  and remove 2 parens and a caret,
        and max(div 2 + 1, 1)!
    */
    printf("%s",argstr);
    free(argstr);
    LOG("number of elements we can store in the array is %d\n",arrlen);
    /* initial array size, - superluos, left/right parentheses and power symbol. */
    nodeTable = malloc(arrlen * sizeof(nodeType *));

    if ( nodeTable == NULL ) {
        fprintf(stderr,"nodeTable: Out of memory, exiting\n");
        exit(EXIT_FAILURE);
    }

    /* 
    Arrays we'll transfer data to, that will work in parallel with the permutations table.
    */
    char *vars=NULL, *ops=NULL;
    int *coeffs=NULL,exponent=0;

#if 1 == 1
    int item_type, nrvars=0,nrops=0;
    validity end_cond;
     /* We parse the input through lex, validate the items in the validator
     and only then do we insert the items which are dynamically allocated tagged structs
     into the nodeTable.*/
    PARSING_STAGE = true ;
    if (atexit(lexer_exit) != 0) {
        fprintf(stderr,
          "Something awfully wrong, couldn't install exit handler for lexer!\n");
        exit(EXIT_FAILURE);
    }
    while ((item_type = yylex()) != 0 ) {
        if ((end_cond = validator(item_type,&nrvars,&nrops,&nritems)) == OK) {
            consumed_text += yyleng ;
            LOG("STATUS == OK ");
            switch (item_type) {
                case OPERAND:
                    LOG(" OPERAND \n");
                    break;
                case OPERATOR: 
                    LOG(" OPERATOR \n");
                    break;
                case PWR_V:
                    LOG(" POWER \n");
                    break;
                default:
                    LOG("\n");
            }
            switch (item_type) {
                case OPERAND:
                case OPERATOR: 
                case PWR_V:
                    nodeTable[(nritems-1)] = yylval ;
                    break;
                default:
                    ;
            }
            /* into struct with yylval? */
            continue;
        } else if (end_cond == FAIL ) {
            syntax_err(NULL);
            /* Could have saved the last state, and figured what was amiss. */
            LOG("STATUS == FAIL\n");
            break;
        } else if (end_cond == ACCEPT )  {
            PARSING_STAGE = false ;
            printf(" =\n"); 
            nodeTable[(nritems-1)] = yylval ;
            LOG("STATUS == ACCEPT POWER:  we got %d items in the table:\n",nritems);
            LOG("And we got %d varss  and %d operrators in the table:\n",nrvars,nrops);

            exponent=make_vartables(nritems,nodeTable,
                 nrvars, nrops,  &vars, &coeffs, &ops);

            LOG("Factor data: \n");
            for (int i=0;i<nrvars;i++) {
                LOG("%d%c\n",coeffs[i],vars[i]);
            }
            yylval = NULL ;
            LOG("exponent == %d\n",exponent);
            /* Time to free the nodeTable */
            for (int i=0;i<nritems;i++) {
                free(nodeTable[i]) ;
            }
            free(nodeTable) ;
            /* this is where we call make_permtable()
                It is a great idea to return the number of rows.
            */
            int *terms_table;

            int terms_rows = mk_permtable(nrvars,exponent,&terms_table);
            if (terms_rows == -1 ) {
                 free_vartables(&vars, &coeffs,&ops);
                 fclose( in );
                 close(wc_pfd[0]);
                exit(EXIT_FAILURE);
            }

            adjust_coeffs(nrvars,coeffs, ops) ;
            // print_term_tbl(nrvars, exponent, terms_table,  terms_rows);
            expand_expr(terms_rows, nrvars, terms_table, vars, coeffs);
            /* free_vartables(&vars, &coeffs,&ops); */
            free(terms_table);
            free_vartables(&vars, &coeffs,&ops);
            
        }
    }
#else
    yylex();
/* } */
#endif
    fclose( in );
    close(wc_pfd[0]);
    return 0;
}
/*
 vim: nospell
*/
