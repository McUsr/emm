
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
#include "multinom.h"
/*
 * finitestate.c
 * ==============
 *
 * A finitestate machine to certify an expression is correct, it is supposed to be
 * reentrant, since we will need to call it per each token.
 * It should also keep track of the number of variables, and the number of operators.
 * When we have the correct number of variables, we can create the structure and 
 * we can also get out the power, which is the last element.
 *
 * New conventions:
 *
 * We use lex's return type as the value to probe in a state.
 * state should be named next_state.
 *
 */

typedef enum { S_START, S_OPERAND, S_OP_OR_RP, S_PWR } fsm_state;

validity validator( int item_type, int *nrvars, int *nrops, int *nritems )
{
   // update after?
    static fsm_state next_state = S_START;
    validity ret = FAIL;
    switch ( next_state ) {
    case S_START:{
            if ( item_type == LEFT_P ) {
                next_state = S_OPERAND;
                ret = OK;
            } else {
                ret = FAIL;
            }
            break;
        }
    case S_OPERAND:{
            if ( item_type == OPERAND ) {
                ( *nritems )++;
                ( *nrvars )++;
                next_state = S_OP_OR_RP;
                ret = OK;
            } else {
                ret = FAIL;
            }
            break;
        }
    case S_OP_OR_RP:{
            if ( item_type == RIGHT_P ) {
                next_state = S_PWR;
                ret = OK;
            } else if ( item_type == OPERATOR ) { /* some work with my type
                                                     hierarchy needed! */
                ( *nritems )++;
                ( *nrops )++;
                next_state = S_OPERAND;
                ret = OK;
            } else {
                ret = FAIL;
            }
            break;
        }
    case S_PWR:{
            if ( item_type == PWR_V ) {
                ( *nritems )++;
                ret = ACCEPT;
            }
            break;
        }
    default:
        ret = FAIL;
    }
    return ret;
}
