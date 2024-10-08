
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
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <limits.h>
/**
 * @file permtable.c  (based on baelditer.c)
 */

static void log_msg( const char *format, ... );

#define LOG(FMT, ...) \
  log_msg(__FILE__":%d %s "FMT, __LINE__,__func__ __VA_OPT__(,)__VA_ARGS__)

static bool doprint = false;
static void log_msg( const char *format, ... )
{
    va_list args;
    va_start( args, format );
    if ( doprint )
        vfprintf( stderr, format, args );
    va_end( args );
}

static unsigned fact_too_big_param = 0;

/* 
 * Truly fixed detecting overflows and underflows  thanks to u/inz_ 
 * This can be done better I'm sure though, by bit counting.
 */
int power( int base, int exp )
{
    assert( exp >= 0 );

    long result = 1;

    if ( exp == 0 )
        return ( int ) result;

    if ( base == 0 )
        return ( int ) 0;

    if ( !( base < 0 && exp % 2 ) ) {
        int adj_base = ( base > 0 ) ? base : ( base * -1 );
        for ( int i = 1; i <= exp; i++ ) {
            if ( result > INT_MAX / adj_base ) {
                fprintf( stderr, "power:" " integer overflow when raising" " base %d to exponent %d\n", base, exp );
                exit( EXIT_FAILURE );
                break;
            } else {
                result *= adj_base;
            }
        }
    } else {
        long pos_base = -1 * base;
        for ( int i = 1; i <= exp; i++ ) {

            if ( result < INT_MIN / pos_base ) {
                fprintf( stderr, "power:" " integer underflow when raising" " base %d to exponent %d\n", base, exp );
                exit( EXIT_FAILURE );
                break;
            } else {
                result *= base;
                if ( result > 0 ) {
                    result *= -1;
                }

            }
        }
    }
    return ( int ) result;
}

/*
 * We return with -1 when there is overflow.
 * Could count bits instead of the invariant with division.
 * Not sure if there are much to saved, and it isn't so precarious
 * here anyway. 
 */
int factorial( unsigned n )
{
    unsigned tmp = n;
    if ( n <= 1 ) {
        return 1;
    } else {
        unsigned next = n - 1;
        fact_too_big_param = n;
        while ( n > 1 ) {
            if ( tmp > UINT_MAX / next ) {
                return -1;
            } else {
                tmp *= ( n - 1 );
                --n;
                next = ( n - 1 );
            }
        }
    }
   /* msb of tmp can't be set if we get here. implies that the UINT can be
      casted to INT! */
    return ( int ) tmp;
}

long l_factorial( unsigned n )
{
    unsigned long tmp = n;
    if ( n <= 1 ) {
        return 1L;
    } else {
        unsigned long next = n - 1;
        fact_too_big_param = n;
        while ( n > 1 ) {
            if ( tmp > ULONG_MAX / next ) {
                return -1L;
            } else {
                tmp *= ( n - 1 );
                --n;
                next = ( n - 1 );
            }
        }
    }
   /* msb of tmp can't be set if we get here. implies that the ULONG can be
      casted to LONG! */
    return ( long ) tmp;
}

/**
 * @brief Calculates an r-combination. 
 * @detail
 *
 * c(n,r) == n!/(r!*(n-r)!)
 * if n>=r && fact(n) > -1  we're okay!
 *
 * There is more that doesn't make sense here, if r > n. f.ex.
 * It should be taken care of before trying to get factorials.
 * A too large parameter for factorial being another.
 * Return value in this case should be -1 too.
 *
 */
static int c( int n, int r )
{
    assert( r <= n );
    if ( n == 0 )
        return 1;
    if ( r == 0 )
        return 1; /* there is just one combination of zero elements. */


    long ntmp = l_factorial( n );
    if ( ntmp == -1 ) {
        fprintf( stderr, "c(): integer overflow; value of n too big: %d exponent == %d\n", n, fact_too_big_param );
        return -1;
    }
    long rtmp = l_factorial( r ),
        nminusrtmp = l_factorial( n - r );

    ntmp /= rtmp;
    ntmp /= nminusrtmp;
    if ( ntmp < INT_MAX ) {
        return ntmp;
    } else {
        return -1;
    }
}

/* https://www.baeldung.com/cs/permutations-with-repetition */
/**
 * @brief Generates the nextt permutation with repetition.
 * @detail Much like lexically ordered permutations, but 
 * here we change the order "odometer-wise".
 * There are 'k' positions in the buffer.
 */
static int *next( int *x, int n, int k )
{
   /* Find the suffix to change. */
    int j = k - 1; /* last position! */
    while ( j >= 0 && x[j] == n )
        j--;

    if ( j == -1 )
        return NULL; /* x was the last permutation */


   /* Change the suffix. */
    for ( int l = j; l < k; l++ ) {
        if ( x[l] == n )
            x[l] = 0;
        else
            x[l]++;
    }

    return x;

}

static bool processed( int *x, int n, int k, int *coefftbl, int sres_idx )
{
   /* 
    * The table is now stack, so we need to treat the area
    * as a stack.
    * maybe easy if everything inside a struct.
    *
    * if cross sum of x == k (power to whichh multinomial is raised.
    * assign row correctly in reverse list.
    * LATER:
    * Calculatate multinomial coeffecient.
    * place multinomial coeffecient correctly in reverse list.
   */

    bool passed = false;
    bool dbg_show = false;
    int cross_sum = 0;
    for ( int i = 0; i < k; i++ )
        if ( i == 0 )
            cross_sum = x[i];
        else
            cross_sum += x[i];

    if ( cross_sum == n ) {
        passed = true;
       /* calc of multinomial coeffecient happens later! */
        if ( dbg_show ) {
            for ( int i = 0; i < k; i++ )
                printf( "%2d ", x[i] );
            printf( "\n" );
        }
        int *tbl_ofs = coefftbl + ( sres_idx * ( k + 1 ) );
        for ( int i = 0; i < k; i++ )
            tbl_ofs[i] = x[i];
    }
    return passed;
}

static void perm_term_tbl( int k, int n, int *coefftbl, int sres_idx, int *p_buffer )
{

   /* INPUT
    * n = the set cardinality, the number of powers upto and including n.
    * n happens to be the exponent to which the multinomial is raised/expanded to.
    * k = the length of permutations with repetition (nr_vars).
    * OUTPUT
    * All the k-tuples of the set {1,2,...,n} are processed
    * and all the k-tuples with a cross-sum of n is inserted into the coefftbl by
    * processed().
    */

    doprint = false;
    while ( sres_idx >= 0 && p_buffer != NULL ) {
        LOG( "%2d %2d %2d\n", p_buffer[0], p_buffer[1], p_buffer[2] );
        if ( processed( p_buffer, n, k, coefftbl, sres_idx ) )
            sres_idx--;

        p_buffer = next( p_buffer, n, k ); 
        /* Next permutation to processe. */
    }
}


/**
 * The number of rows we need for the coeffecient table
 * is how many rows there are of permutations, with the cross sum
 * equalling the exponent. 
 *
 * The combinatoric formula:  (n+k-1) 
 *                            (  k  ) 
 *                            
 * Number of r-combinations of a set with n elements with repetition
 * allowed.  N is here the number of variables, k is the exponent to which
 * the multinomial (also a binomial or monomial) is raised.
 * (The power which the multinomial is expanded to.)
 *
 *     int coefftbl_rows( int exponent, int nr_vars )
 *     {
 *         return c( ( nr_vars + exponent - 1 ), exponent );
 *     } 
 */

static void calc_multinom_coeff( int nr_vars, int mnom_dividend, int *terms_table, int nr_rows )
{
   /* All values are supposed to be smalller or equal to exponent. so, no need
      to check return value of factorial for overflow.  */

    for ( int i = 0; i < nr_rows; i++ ) {
        int mnom_divisor = 1;

        for ( int j = 0; j < nr_vars; j++ ) {
            int tmp = factorial( *( terms_table + ( i * ( nr_vars + 1 ) + j ) ) );
            if ( tmp > INT_MAX / mnom_divisor ) {
                int cur_pwr = *( terms_table + ( i * ( nr_vars + 1 ) + j ) );
                fprintf( stderr,
                         "Error: Int overflow in calc_multinom_coeff."
                         " cur_ exponent == %d mnom_divisor == %d\n", cur_pwr, mnom_divisor );
                exit( EXIT_FAILURE );
            } else {
                mnom_divisor *= tmp;
            }
        }

        *( terms_table + ( i * ( nr_vars + 1 ) + nr_vars ) ) = mnom_dividend / mnom_divisor;
    }

}

#if 0 == 1
/* A debug routine */
void print_term_tbl( int nr_vars, int exponent, int *terms_table, int nr_rows )
{
    printf( "Multinomial table for exponent == %d, number of vars: %d\n", exponent, nr_vars );
    printf( "First columns, denotes the power of the variables\n" );
    printf( "Last column is the multinomial coeffecient.\n" );
    for ( int i = 0; i < nr_rows; i++ ) {
        for ( int j = 0; j <= nr_vars; j++ )
            printf( "| %3d ", *( terms_table + ( i * ( nr_vars + 1 ) + j ) ) );
        printf( "\n" );
    }

}
#endif 
void exponent_err( void )
{
    fflush( stdout );
    fprintf( stderr, "\nExponent too big to calculate the factorial of: was: %u, needs to exit.\n", fact_too_big_param );
}
/**
 * @brief Generates the table with multinomial coeffecients that satisfies
 * the condition that the cross sum of the row equals the power of the multinomial,
 * IN REVERSE ORDER, so the terms comes out correctly in the end.
 */
int mk_permtable( int nr_vars, int exponent, int **terms_table )
{
   /* Create the buffer we permute. */
    assert( nr_vars > 1 && exponent > 0 );
    int *perm_buffer = calloc( nr_vars, sizeof( int ) );
    if ( perm_buffer == NULL ) {
        fprintf( stderr, "perm_buffer: Out of memory, exiting\n" );
        exit( EXIT_FAILURE );
    }

    perm_buffer[nr_vars - 1] = exponent; // seed the buffer with start value

   /*
    * Calculate number of rows in the terms_table which contains the
    * power for the variables and the multinom coeff for the term.
    */
    int rows_termtbl;
    rows_termtbl = c( ( nr_vars + exponent - 1 ), exponent );
    if ( rows_termtbl == -1 ) {
        free( perm_buffer );
        fprintf( stderr, "mk_permtable: Integer overflow while computing number of factors.\n" );
        fprintf( stderr, "mk_permtable: The combination of the exponent (%d), and"
                 " the number of variables (%d) is too large.\n", exponent, nr_vars );
        return -1;
    }
   /* Allocate memory for the terms_table. */
    *terms_table = calloc( ( rows_termtbl * ( nr_vars + 1 ) ), sizeof( int ) );
    if ( terms_table == NULL ) {
        fprintf( stderr, "terms_table: Out of memory, exiting\n" );
        exit( EXIT_FAILURE );
    }

    int last_term_idx = ( rows_termtbl - 1 ); 
    /* we fill in the terms_table from bottom. so the permu
     * are ordered in reverse, which is correct in our case. 
     */

    perm_term_tbl( nr_vars, exponent, *terms_table, last_term_idx, perm_buffer );

    int mnom_dividend = factorial( exponent );
    if ( mnom_dividend == -1 ) {
        free( *terms_table );
        free( perm_buffer );
        exponent_err(  );
        return -1;
    }

    calc_multinom_coeff( nr_vars, mnom_dividend, *terms_table, rows_termtbl );

    free( perm_buffer );
    return rows_termtbl;

}
