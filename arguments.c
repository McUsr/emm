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
#include <libgen.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include "multinom.h"

#define SPACE ' '

void show_usage( char *prog_name)
{
  fprintf( stderr, "Usage: \"%s [-h|-p] (multinomial expression)^power.\"\n", basename(prog_name));
}
void show_help(void )
{
  fprintf(stderr, "\n -h  -- prints this help and exits.\n");
  fprintf(stderr, "\nA multinomial expression is on the form: \"(a -2b +c)^4\", parentheses mandatory.\n\n");
  fprintf(stderr, " Piping or redirecting to a file sends the result there without any explanation\n"
         " to be used as input for a calculator.\n");
  fprintf(stderr,"\"multinom '(-3x - 5y + 2z) ^4' > newout\" send the expression to newout.\n");
  fprintf(stderr,"\"multinom '(-3x - 5y + 2z) ^4' 2>/dev/null\" shows only the expression on screen..\n");
  fprintf(stderr,"\"multinom '(-3x - 5y + 2z) ^4' >& newout\" send everything to newout.\n");
  fprintf(stderr,"\"multinom '(-3x - 5y + 2z) ^4' 2>&1 >newout\" send everything to newout.\n");

  /* fprintf( stderr, "Usage: \"%s [-h|-p '(a -2b +c)^4'\"\n", basename(prog_name)); */
}

/*  
    trims the command line for extranous whitespace
    and assures that it isn't longer than the treshold, which in 
    this case is YY_BUF_SIZE.
*/
int print_cmdln_child( int argc, char *argv[], int treshold, int *consumed )
{
    int numbytes=treshold;

    ++argv;

    for (int i = argc ; i>1;i--) {
         int len = strlen(*argv) ;
        char *buf = calloc(len +1,1 );
        if (buf == NULL) {
            fprintf(stderr,"print_cmdln: Error: calloc() Out Of Memory. Exiting\n");
            return 1;
        }
        char *strbuf = buf;

        if (i<argc) {
            *(strbuf++) = SPACE ; 
        }

        while (**argv) {
            if (isspace((unsigned char)**argv)) {
                *(strbuf++) = SPACE ;
                (*argv)++;
                while (isspace((unsigned char)**argv)) {
                    (*argv)++;
                }
            } else {
                *strbuf = **argv ;
                strbuf++;
                (*argv)++;
            }
        }
        *strbuf = '\0' ;

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

int print_cmdln_parent( int argc, char *argv[],int *consumed)
{

    *consumed = 0;
    ++argv;

    for (int i = argc ; i>1;i--) {
        if (i<argc) {
            fprintf(stderr,"%c",SPACE); ; 
            (*consumed)++;
        }

        while (**argv) {
            if (isspace((unsigned char)**argv)) {
                fprintf(stderr,"%c",SPACE); ; 
                (*argv)++;
                (*consumed)++;
                while (isspace((unsigned char)**argv)) {
                    (*argv)++;
                }
            } else {
                fprintf(stderr,"%c",**argv); ; 
                (*consumed)++;
                (*argv)++;
            }
        }
         argv++;
    }
    return 0;
}
/* parses any command line options. */
int options( int argc, char *argv[] )
{
    int opt=0;
    opt_tp ret_val= OPT_NONE ;

    while ( ( opt = getopt( argc, argv, ":hp" ) ) != -1 ) {
        switch ( opt ) {
        case 'h':
            ret_val = OPT_HELP;
            break;
        default: /* '?' */
            ret_val = OPT_BAD;
        }
    }
    return ret_val;
}
#if 0 == 1
int main(int argc, char *argv[])
{
    
    show_cmdln(argv[0]);
    show_help();
    return 0;
}
#endif
