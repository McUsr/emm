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

#include <stdio.h>
#include "multinom.h" 

/* we get the product of coeffecients which we multiply with the multnomial 
 * coeffecient.
 */
static int calc_cur_factor_coeff( int nr_vars,int *terms_table, int *coefftbl )
{
    /* We get the multnomial coeffecient from last column */
    int factor_coeff = terms_table[nr_vars] ; //
    /* compounds the product of it, and individual coeffe
     * per variable, raised to the correct power for thiss term. */
    for(int i=0;i<nr_vars;i++) {
        if (terms_table[i] > 0) {
            if ( coefftbl[i] != 0 && coefftbl[i] != 1 ) {
                factor_coeff *= power(coefftbl[i],terms_table[i] ) ;
            }
        }
    }
    return factor_coeff ;
}

/* Prints variables raised to a power, like we expect:
 * example:
 *      xy^2z^3
 */
static void print_raised_vars( int nr_vars, int *terms_table,
        char *vartable)
{
    for(int i=0;i<nr_vars;i++) {
        if (terms_table[i] > 0) {
            // we will include this in the factor
            if ( terms_table[i] == 1 ) {
                printf("%c",vartable[i]);
            } else if (terms_table[i] > 1) {
                printf("%c^%d",vartable[i],terms_table[i]);
            }
        }
    }
}

/* we adjust any signs of coeffecients when we have a 
 * '-' operator in front of it.
 */
void adjust_coeffs(int nr_vars,int *coefftbl, char *optbl) 
{
    for(int i=1;i<nr_vars;i++) {
        if (optbl[i-1] == '-'  ) {
            coefftbl[i] *= -1 ;
        } 
    }
}

/* Every row in the terms table becomes one factor in the expanded
 * multnomial. */
void expand_expr(int terms_rows, int nr_vars, int *terms_table,
        char *vartable, int *coefftbl)
{

    for(int i=0;i<terms_rows;i++) {

       int factor_coeff = calc_cur_factor_coeff(nr_vars,
               (terms_table+(i *(nr_vars+1))),coefftbl);

       if (i == 0) {
            printf("%d",factor_coeff);
       } else if ( factor_coeff < 0 ) {
           printf(" - ") ;
           factor_coeff *= -1 ;
           printf("%d",factor_coeff);
        } else { 
           printf(" + %d",factor_coeff);
        }
       print_raised_vars( nr_vars, (terms_table+(i *(nr_vars+1))),vartable);
    }
    printf("\n");
}

