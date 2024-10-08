
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
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include "multinom.h"

/* nothing safe about this function, trusts the regexp fully for now.
 *
 * This function is to be called from within yylex to make a new structure
 * and make yylval point to it.
 * */

static int sym[53]; /* 1 more for the case we have a coeffecient without a
                       variable. */
// check this out, I have it defined somewhere else!

static itemData *mkVarNode( int coeff, char var )
{
    itemData *retval = malloc( sizeof( itemData ) );
    if ( retval != NULL ) {
        retval->type = typeFact;
        retval->factor.coeff = coeff;
        retval->factor.var = var;
        retval->next = NULL;
    } else {
        fprintf( stderr, "mkVarNode: Out of memory, exiting\n" );
        exit( EXIT_FAILURE );
    }
    return retval;
}

static itemData *mkOperNode( char oper )
{
    itemData *retval = malloc( sizeof( itemData ) );
    if ( retval != NULL ) {
        retval->type = typeOpr;
        retval->opr.oper = oper;
        retval->next = NULL;
    } else {
        fprintf( stderr, "mkOperNode: Out of memory, exiting\n" );
        exit( EXIT_FAILURE );
    }
    return retval;
}

static itemData *mkPowerNode( int pwer )
{
    itemData *retval = malloc( sizeof( itemData ) );
    if ( retval != NULL ) {
        retval->type = typePwr;
        retval->pwr.pwer = pwer;
        retval->next = NULL;
    } else {
        fprintf( stderr, "mkPowerNode: Out of memory, exiting\n" );
        exit( EXIT_FAILURE );
    }
    return retval;
}

/* This only checks if it has been used, the converted char value is not the value we store
 * in the variable array Here we store the original char value, or '0' for no variable
 * name (coeffecient).
 */

static bool accepted_var( char var )
{
    if ( isupper( ( unsigned char ) var ) ) {
        var = var - 'A' + 27;
    } else if ( islower( ( unsigned char ) var ) ) {
        var = var - 'a' + 1;
    } else if ( var == 0 ) {
        ;
    } else {
        syntax_err( NULL );
        fprintf( stderr, "accepted_var: Can't happen varable name outside legal range: %c\n", var );
        exit( EXIT_FAILURE );
    }
    if ( sym[( int ) var] > 0 ) {
        return false;
    } else {
        sym[( int ) var] += 1;
        return true;
    }
}

itemData *newOperator( char *str )
{
    return mkOperNode( *str );
}

itemData *newPower( char *str )
{
    char *endptr;
    str++;
    long val = strtol( str, &endptr, 10 );

    if ( errno != 0 ) {
        syntax_err2( "newPower:strtol", strerror( errno ) );
        exit( EXIT_FAILURE );
    }

    if ( endptr == str ) {
        syntax_err2( "newPower:", "No digits were found" );
        fprintf( stderr, "No digits were found\n" );
        exit( EXIT_FAILURE );
    }

    if ( val >= INT_MAX ) {
        syntax_err2( "newPower", "Value greater than INT_MAX!" );
        exit( EXIT_FAILURE );
    }
    int pwer = ( int ) val;
    return mkPowerNode( pwer );
}

itemData *newVariable( char *str, int len, content_type what )
{
    itemData *retval = NULL;
    int coeff = 0;
    char var = 0;
    switch ( what ) {
    case F_FULL:{
          /* also covers no sign */
            char *endptr;
            long val = strtol( str, &endptr, 10 );

            if ( errno != 0 ) {
                syntax_err2( "newVariable:strtol", strerror( errno ) );
                exit( EXIT_FAILURE );
            }

            if ( endptr == str ) {
                syntax_err2( "newVariable:", "No digits were found" );
                exit( EXIT_FAILURE );
            }

            if ( *endptr == '\0' ) { /* Not necessarily an error... */
                syntax_err2( "newVariable", "Missing variable!" );
                exit( EXIT_FAILURE );
            }
            if ( val >= INT_MAX ) {
                syntax_err2( "newVariable", "Value greater than INT_MAX!" );
                exit( EXIT_FAILURE );
            } else if ( val <= INT_MIN ) {
                syntax_err2( "newVariable", "Value less than INT_MIN!" );
                exit( EXIT_FAILURE );
            }


            coeff = ( int ) val;

            var = *endptr;
            if ( *( endptr + 1 ) != '\0' ) {
                syntax_err2( "newVariable", "Variable can only be one character!" );
                exit( EXIT_FAILURE );

            }
            if ( !accepted_var( var ) ) {
                syntax_err2( "newVariable", "A variable can only be used once in an expression!" );
                exit( EXIT_FAILURE );
            } else {
                retval = mkVarNode( coeff, var );
            }
            break;
        }
    case F_NO_COEFF:{
          /* but with a sign */
            if ( len > 2 ) {
                syntax_err2( "newVariable", "Variable can only be one character!" );
                exit( EXIT_FAILURE );
            }

            if ( *str == '-' ) {
                coeff = -1;
                str++;
            } else if ( *str == '+' ){
                coeff = 1;
                str++;
            } else {
                coeff = 1;
            }

            var = *str;
            if ( !accepted_var( var ) ) {
                syntax_err2( "newVariable", "A variable can only be used once in an expression!" );
                exit( EXIT_FAILURE );
            } else {
                retval = mkVarNode( coeff, var );
            }
            break;
        }
    case F_JUSTVAR:{
            if ( len > 1 ) {
                syntax_err2( "newVariable", "Variable can only be one character!" );
                exit( EXIT_FAILURE );
            }
            coeff = 1;
            var = *str;
            if ( !accepted_var( var ) ) {
                syntax_err2( "newVariable", "A variable can only be used once in an expression!" );
                exit( EXIT_FAILURE );
            } else {
                retval = mkVarNode( coeff, var );
            }
            break;
        }
    case F_JUST_COEFF:{
            char *endptr;
            long val = strtol( str, &endptr, 10 );

            if ( errno != 0 ) {
                syntax_err2( "newVariable:strtol", strerror( errno ) );
                exit( EXIT_FAILURE );
            }

            if ( endptr == str ) {
                syntax_err2( "newVariable:", "No digits were found" );
                exit( EXIT_FAILURE );
            }

            if ( *endptr == '\0' ) { /* Not necessarily an error... */
                syntax_err2( "newVariable", "Missing variable!" );
                exit( EXIT_FAILURE );
            }
            if ( val >= INT_MAX ) {
                syntax_err2( "newVariable", "Value greater than INT_MAX!" );
                exit( EXIT_FAILURE );
            } else if ( val <= INT_MIN ) {
                syntax_err2( "newVariable", "Value less than INT_MIN!" );
                exit( EXIT_FAILURE );
            }

            coeff = ( int ) val;
            var = 0;
            if ( !accepted_var( var ) ) {
                syntax_err2( "newVariable", "A variable can only be used once in an expression!" );
                exit( EXIT_FAILURE );
            } else {
                retval = mkVarNode( coeff, var );
            }
            break;
        }
    default:{
            fprintf( stderr, "Can't happen, BAD SWITCH CASE in newVariable.\n" );
            exit( EXIT_FAILURE );
        }
    }
    return retval;
}

void lexer_exit( void )
{
   /* uses global variables */
    if ( PARSING_STAGE == true ) {
        itemData  *p = itemsHead.next, *q=NULL ;

        for ( q=p->next; q != NULL; q=p->next ) {
            free( p );
            p = q ;
        }
    }

}
