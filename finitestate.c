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

typedef enum {S_START, S_OPERAND, S_OP_OR_RP, S_PWR } fsm_state ;

validity validator( int item_type, int *nrvars, int *nrops, int *nritems)
{
    // update after?
    static fsm_state next_state=S_START;
    validity ret=FAIL;
    switch (next_state) {
        case S_START: {
            if (item_type == LEFT_P ) {
                next_state = S_OPERAND ;
                ret= OK ;
            } else {
                ret= FAIL ;
            }
            break;
        }
        case S_OPERAND: {
            if (item_type == OPERAND ) {
                (*nritems)++;
                (*nrvars)++;
                next_state = S_OP_OR_RP;
                ret= OK ;
            } else {
                ret= FAIL ;
            }
            break;
        }
        case S_OP_OR_RP: {
            if (item_type == RIGHT_P ) {
                    next_state =  S_PWR ;
                    ret= OK ;
            } else if (item_type == OPERATOR ) { /* some work with my type hierarchy needed! */
                    (*nritems)++;
                    (*nrops)++;
                    next_state =  S_OPERAND ;
                    ret= OK ;
            } else {
                ret= FAIL ;
            }
            break;      
        }
        case S_PWR: {
            if (item_type == PWR_V ) {
                (*nritems)++;
                ret= ACCEPT;
            }
            break;
        }
        default:
                ret= FAIL ;
    }
    return ret;
}
/* 
 * This statemmachine to be included in the lex file.
 * purpose to be sure of the definitions  and stuff.
 */

/* Altercations:
 * We need to keep the operators. to get things right, especially the negative ones.
 *
 * If it isn't the first operand, we'll need to check the operator, after we have resolved
 * the sign of the factor.
 * This meanns we will need a union, much like the one that yacc  would have been used for
 * yyval.
 */

/* Later, we generate a sign  */


