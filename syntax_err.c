#include <stdio.h>
#include "multinom.h"
/*
 * Uses the global variables `consumed_text` and  `ignored_spaces`
 * for pinpointing out exactly where the error is. This obviously doesn't
 * work to well with a proportional font!
 */
void syntax_err(const char * const details) 
{
    fflush(stdout);
    fprintf(stderr,"\n%*s^\n",(consumed_text+ignored_spaces)," ");
    if (details == NULL ) {
        fprintf(stderr,"%*sSyntax error.\n",(consumed_text+ignored_spaces)," ");
    } else {
        fprintf(stderr,"%*sSyntax error: %s\n",
                (consumed_text+ignored_spaces)," ",details);
    }
}

void syntax_err2(const char * const details1,const char * const details2) 
{
    fflush(stdout);
    fprintf(stderr,"\n%*s^\n",(consumed_text+ignored_spaces)," ");
    fprintf(stderr,"%*sSyntax error: %s: %s\n", (consumed_text+ignored_spaces)," ",
            details1,details2);
}
