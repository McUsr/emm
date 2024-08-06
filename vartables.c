
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
#include "multinom.h"
/**
 * Transfers data from nodeTable into more suitable tables related to the termstable
 * so that we can expand the terms into something meaningful.
 * returns: the exponent.
 */
int make_vartables( int nritems, nodeType ** nodeTable, int nrvars, int nrops, char **vars, int **coeffs, char **ops )
{
    int vars_c = 0,
        op_c = 0,
        exponent = 0;
    ;
    *vars = malloc( nrvars );
    if ( *vars == NULL ) {
        fprintf( stderr, "vars: Out of memory, exiting\n" );
        exit( EXIT_FAILURE );
    }
    *coeffs = malloc( nrvars * sizeof( int ) );
    if ( *coeffs == NULL ) {
        fprintf( stderr, "coeffs: Out of memory, exiting\n" );
        exit( EXIT_FAILURE );
    }
    *ops = malloc( nrops );

    if ( *ops == NULL ) {
        fprintf( stderr, "ops: Out of memory, exiting\n" );
        exit( EXIT_FAILURE );
    }

    for ( int i = 0; i < nritems; i++ ) {
        switch ( nodeTable[i]->type ) {
        case typeFact:
            ( *vars )[vars_c] = ( nodeTable[i] )->factor.var;
            ( *coeffs )[vars_c++] = ( nodeTable[i] )->factor.coeff;
            break;
        case typeOpr:
            ( *ops )[op_c++] = ( nodeTable[i] )->opr.oper;
            break;
        case typePwr:
            exponent = ( nodeTable[i] )->pwr.pwer;
            break;
        default:
            fprintf( stderr, "Can't happen in mk_vartables, bad tag enum\n" );
            exit( EXIT_FAILURE );
        }
    }
    return exponent;
}

void free_vartables( char **vars, int **coeffs, char **ops )
{
    free( *vars );
    free( *coeffs );
    free( *ops );
}
