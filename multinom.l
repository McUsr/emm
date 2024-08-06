
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
#include <ctype.h>
#include "multinom.h"
#define SPACE ' '

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


int yywrap( void )
{
    return 1;
}

// trims the command line for extranous whitespace
// and validates that it isn't too long.

#define MIN(x,y) \
    (x) < (y) ? x : y ;

int print_cmdln( int argc, char *argv[], int treshold, int *consumed)
{
    int numbytes=treshold;

    ++argv;
    for (int i = argc ; i>1;i--) {
         int len = strlen(*argv) ;
        char *buf = calloc(len +1,1 );
        if (buf == NULL) {
            fprintf(stderr,"print_cmdln: Error: malloc() Out Of Memory. Exiting\n");
            return 1;
        }
        char *strbuf = buf;

        if (i<argc) {
            *(strbuf++) = SPACE ; 
        }

        while (**argv) {
            if (isspace(**argv)) {
                *(strbuf++) = SPACE ;
                (*argv)++;
                while (isspace(**argv)) {
                    (*argv)++;
                }
            } else {
                *strbuf = **argv ;
                strbuf++;
                (*argv)++;
            }
        }

        int buflen = strlen(buf);

        if (buf[buflen-1] == SPACE ) {
            buf[buflen-1] = '\0' ;
            buflen--;
        }            
        *consumed += buflen ; // newline not part of the argstrlength capacity equation.
        treshold -= (buflen + (i>2) ? 0 : 1  );
        //  we add the newline after the last argument (u/inz_)
        if (treshold > 0 ) {
            printf("%s",buf);
            free(buf);    
        } else {
            fprintf(stderr, "print_cmdln: Error: argument string  too long, maximum is %d bytes\n",numbytes);
            free(buf);    
            return 1;
        }

        argv++;
    }
    return 0;
}

int main( int argc, char *argv[] )
{
    int wc_pid;
    int wc_pfd[2];
    int chldstatus;
    FILE *in;

    if ( argc < 2 ) {
        fprintf( stderr, "%s: Usage: \"%s '(a -2b +c)^4'\"\n", *argv, *argv );
        fprintf( stderr, "You need to supply a bi/multinomial to expand. Exiting\n");
        exit( EXIT_FAILURE );
    }

    if ( pipe( wc_pfd ) < 0 ) {
        fprintf( stderr, "Can't set up the pipe: %s\n", strerror( errno ) );
        fprintf( stderr, "Exiting!\n" );
        exit( EXIT_FAILURE );
    }

    wc_pid = fork(  );
    if ( wc_pid < 0 ) {
        fprintf( stderr, "Can't fork child process: %s\n", strerror( errno ) );
        fprintf( stderr, "Exiting!\n" );
        exit( EXIT_FAILURE );

    } else if ( wc_pid == 0 ) { /* child */
        close( wc_pfd[0] );
        if ( dup2( wc_pfd[1], 1 ) < 0 ) {
            fprintf( stderr, "child couldn't duplicate write end of pipe: %s\n", strerror( errno ) );
            return 2;
        }

        long numbytes = fpathconf(wc_pfd[1],_PC_PIPE_BUF) ;
        if ( numbytes < 0 ) {
            fprintf(stderr,"fpathconf didn't return anything sensible: %s\n",strerror(errno));
            return 2;
        } else if (YY_BUF_SIZE > numbytes) {
            fprintf(stderr,"YY_BUF_SIZE > _PC_PIPE_BUF: you need to reconfigure YY_BUF_SIZE > %ld\n",numbytes);
            return 2;
        }
        int ret_code, dummy=0;
        ret_code= print_cmdln( argc, argv, YY_BUF_SIZE, &dummy) ;
        if (!ret_code) {
            printf("\n");
        }
        return ret_code;
    }
    waitpid( wc_pid, &chldstatus, 0 );
    if ( close( wc_pfd[1] ) < 0 ) {
        fprintf( stderr, "parent couldn't close write end of pipe: %s\n", strerror( errno ) );
        return 1;
    }
    if ( WIFEXITED( chldstatus ) ) {
        int ret_code = WEXITSTATUS( chldstatus );
        if ( ret_code ) {
            switch ( ret_code ) {
            case 1:
                fprintf( stderr, "There were problems with parsing the command line. Exiting!\n" );
                break;
            case 2:
                fprintf( stderr, "There were problems with system calls during parsing the command line. Exiting!\n" );
                break;
            default:
                ;
            }
            return 1;
        }
    }
    in = fdopen( wc_pfd[0], "r" );
    if ( dup2( fileno( in ), 0 ) == -1 ) {
        perror( strerror( errno ) );
        exit( EXIT_FAILURE );
    }

    int arrlen = 0;
    if (print_cmdln( argc, argv, YY_BUF_SIZE, &arrlen)) {
        exit(EXIT_FAILURE) ;
    }
    arrlen -= 3 ; /* subtracting for "(",")" and "^". */
    arrlen = ((arrlen+1)/4)+1;
    /* because there are one more than the operators, and there are two spaces
       for each operator. so, this will be a pretty generous estimate. */

    LOG( "number of elements we can store in the array is %d\n", arrlen );
    nodeTable = malloc( arrlen * sizeof( nodeType * ) );

    if ( nodeTable == NULL ) {
        fprintf( stderr, "nodeTable: Out of memory, exiting\n" );
        exit( EXIT_FAILURE );
    }

   /* 
      Arrays we'll transfer data to, that will work in parallel with the
      permutations table. */
    char *vars = NULL,
        *ops = NULL;
    int *coeffs = NULL,
        exponent = 0;

#if 1 == 1
    int item_type,
     nrvars = 0,
        nrops = 0;
    validity end_cond;
   /* We parse the input through lex, validate the items in the validator and
      only then do we insert the items which are dynamically allocated tagged
      structs into the nodeTable. */
    PARSING_STAGE = true;
    if ( atexit( lexer_exit ) != 0 ) {
        fprintf( stderr, "Something awfully wrong, couldn't install exit handler for lexer!\n" );
        exit( EXIT_FAILURE );
    }
    while ( ( item_type = yylex(  ) ) != 0 ) {
        if ( ( end_cond = validator( item_type, &nrvars, &nrops, &nritems ) ) == OK ) {
            consumed_text += yyleng;
            LOG( "STATUS == OK " );
            switch ( item_type ) {
            case OPERAND:
                LOG( " OPERAND \n" );
                break;
            case OPERATOR:
                LOG( " OPERATOR \n" );
                break;
            case PWR_V:
                LOG( " POWER \n" );
                break;
            default:
                LOG( "\n" );
            }
            switch ( item_type ) {
            case OPERAND:
            case OPERATOR:
            case PWR_V:
                nodeTable[( nritems - 1 )] = yylval;
                break;
            default:
                ;
            }
           /* into struct with yylval? */
            continue;
        } else if ( end_cond == FAIL ) {
            syntax_err( NULL );
           /* Could have saved the last state, and figured what was amiss. */
            LOG( "STATUS == FAIL\n" );
            break;
        } else if ( end_cond == ACCEPT ) {
            PARSING_STAGE = false;
            printf( " =\n" );
            nodeTable[( nritems - 1 )] = yylval;
            LOG( "STATUS == ACCEPT POWER:  we got %d items in the table:\n", nritems );
            LOG( "And we got %d varss  and %d operrators in the table:\n", nrvars, nrops );

            exponent = make_vartables( nritems, nodeTable, nrvars, nrops, &vars, &coeffs, &ops );

            LOG( "Factor data: \n" );
            for ( int i = 0; i < nrvars; i++ ) {
                LOG( "%d%c\n", coeffs[i], vars[i] );
            }
            yylval = NULL;
            LOG( "exponent == %d\n", exponent );
           /* Time to free the nodeTable */
            for ( int i = 0; i < nritems; i++ ) {
                free( nodeTable[i] );
            }
            free( nodeTable );
           /* this is where we call make_permtable() It is a great idea to
              return the number of rows. */
            int *terms_table;

            int terms_rows = mk_permtable( nrvars, exponent, &terms_table );
            if ( terms_rows == -1 ) {
                free_vartables( &vars, &coeffs, &ops );
                fclose( in );
                close( wc_pfd[0] );
                exit( EXIT_FAILURE );
            }

            adjust_coeffs( nrvars, coeffs, ops );
           // print_term_tbl(nrvars, exponent, terms_table, terms_rows);
            expand_expr( terms_rows, nrvars, terms_table, vars, coeffs );
           /* free_vartables(&vars, &coeffs,&ops); */
            free( terms_table );
            free_vartables( &vars, &coeffs, &ops );

        }
    }
#else
    yylex(  );
/* } */
#endif
    fclose( in );
    close( wc_pfd[0] );
    return 0;
}
/*
 vim: nospell
*/
