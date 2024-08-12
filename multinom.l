
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
#include <getopt.h> /* optind */
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
    /* Global variables */
    int consumed_text=0;
    int ignored_spaces=0;
    int nritems=0;

    bool PARSING_STAGE=false; 
    bool NO_PREPROC=true;

    itemData **itemTable, *yylval,itemsHead;
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
                            if (NO_PREPROC == false ) {
                                fprintf(stderr,"%s\n",argstr);
                            }
                            fprintf(stderr,"\n%*s^\n",(consumed_text+ignored_spaces)," ");
                            fprintf(stderr,"%*sSyntax error: LEX: Illegal character.\n",(consumed_text+ignored_spaces)," ");
                            exit(EXIT_FAILURE);
                        }

%%

/* TODO: factor into a fitting module */
void list2table( itemData *listHead, itemData **itemTable )
{
    itemData *itemPtr = listHead ;
    itemPtr = itemPtr->next ;
    int k = 0 ;
    while (itemPtr != NULL ) {
        itemTable[k++] = itemPtr ;
        itemPtr = itemPtr->next ;
    }
}

int yywrap( void )
{
    return 1;
}

int main( int argc, char *argv[] )
{
    int wc_pid;
    int wc_pfd[2];
    int chldstatus;
    FILE *in;

    if ( argc < 2 ) {
        show_usage(argv[0]);
        fprintf( stderr, "Missing a multinomial  with a power to expand. Exiting.\n");
        exit( EXIT_FAILURE );
    }
    int ret_val = options(argc,argv); /* sets optind */

    switch (ret_val) {
    case OPT_NONE:
        break;
    case OPT_HELP:
        show_usage(argv[0]);
        show_help();
        exit(EXIT_SUCCESS);
        break;
    case OPT_PREPROCESS:
        NO_PREPROC=false;
        break;
    case OPT_BAD:
        show_usage(argv[0]);
        fprintf(stderr,"Non-existent option specified.\n");
        exit(EXIT_FAILURE);
    }

    if (optind == argc ) {
        show_usage(argv[0]);
        fprintf( stderr, "Missing a multinomial  with a power to expand. Exiting.\n");
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
            fprintf( stderr, "Child couldn't duplicate write end of pipe: %s\n", strerror( errno ) );
            return 2;
        }

        long numbytes = fpathconf(wc_pfd[1],_PC_PIPE_BUF) ;
        if ( numbytes < 0 ) {
            fprintf(stderr,"Fpathconf didn't return anything sensible: %s\n",strerror(errno));
            return 2;
        } else if (YY_BUF_SIZE > numbytes) {
            fprintf(stderr,"YY_BUF_SIZE > _PC_PIPE_BUF: you need to reconfigure YY_BUF_SIZE > %ld\n",numbytes);
            return 2;
        }
        int ret_code, dummy=0;
        ret_code= print_cmdln_child( argc, argv, YY_BUF_SIZE, &dummy, optind) ;
        if (!ret_code) {
            printf("\n"); /* if 0, then we conlude the input line to lex with a \n" */
        }
        return ret_code;
    }
    waitpid( wc_pid, &chldstatus, 0 );
    if ( close( wc_pfd[1] ) < 0 ) {
        fprintf( stderr, "Parent couldn't close write end of pipe: %s\n", strerror( errno ) );
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
    if (in == NULL) {
        fprintf( stderr, "Couldn't open an input stream from pipe[0]: %s. Exiting!\n", 
                strerror(errno ));
        exit( EXIT_FAILURE );
    }

    if ( dup2( fileno( in ), 0 ) == -1 ) {
        fprintf( stderr, "Couldn't duplicate FILE* in, to standard input: %s. Exiting!\n", 
                strerror(errno ));
        exit( EXIT_FAILURE );
    }

    if (NO_PREPROC ) {
        if (print_cmdln_parent( argc, argv, optind )) {
            exit(EXIT_FAILURE) ;
        }
    } else { 
        if (save_cmdln_parent( argc, argv,YY_BUF_SIZE,optind )) {
            exit(EXIT_FAILURE) ;
        }
    }

    itemData *itemPtr;
    itemsHead.next = NULL;

    int item_type,
     nrvars = 0,
        nrops = 0;
    validity end_cond;
   /* We parse the input through lex, validate the items in the validator and
      only then do we insert the items which are dynamically allocated tagged
      structs into the itemTable. */
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
                /* itemTable[( nritems - 1 )] = yylval; */
                if (itemsHead.next == NULL ) {
                    itemsHead.next = yylval;
                    itemPtr=itemsHead.next;
                    /* itemPtr=itemPtr->next ; */
                } else {
                    itemPtr->next =yylval;
                    itemPtr = itemPtr->next ;
                }
                break;
            default:
                ;
            }
            continue;
        } else if ( end_cond == FAIL ) {
            syntax_err( NULL );
            LOG( "STATUS == FAIL\n" );
            break;
        } else if ( end_cond == ACCEPT ) {
           /* 
              Arrays we'll transfer data to, that will work in parallel with the
              permutations table.
            */
            char *vars = NULL, *ops = NULL;
            int *coeffs = NULL, exponent = 0;

            PARSING_STAGE = false;

            if (NO_PREPROC ) {
                printf( " =\n" );
                fflush(stdout);
            }
            itemPtr->next=yylval ; /* final element */
            itemPtr = itemPtr->next ;

            itemTable = malloc( nritems * sizeof( itemData * ) );
            if ( itemTable == NULL ) {
                fprintf( stderr, "itemTable: Out of memory: %s, exiting\n",strerror(errno) );
                exit( EXIT_FAILURE );
            }

            list2table( &itemsHead, itemTable );

            LOG( "STATUS == ACCEPT POWER:  we got %d items in the table:\n", nritems );
            LOG( "And we got %d varss  and %d operrators in the table:\n", nrvars, nrops );

            exponent = make_vartables( nritems, itemTable, nrvars, nrops, &vars, &coeffs, &ops );

            LOG( "Factor data: \n" );
            for ( int i = 0; i < nrvars; i++ ) {
                LOG( "%d%c\n", coeffs[i], vars[i] );
            }
            /* doesn't point to freed memory. */
            yylval = NULL; 
            itemsHead.next = NULL ;
           /* Time to free the itemTable */
            for ( int i = 0; i < nritems; i++ ) {
                free( itemTable[i] );
            }
            free( itemTable );
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
            expand_expr( terms_rows, nrvars, terms_table, vars, coeffs );
            free( terms_table );
            free_vartables( &vars, &coeffs, &ops );

        }
    }
    fclose( in );
    close( wc_pfd[0] );
    return 0;
}
/*
 vim: nospell
*/
