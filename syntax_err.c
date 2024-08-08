
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
/*
 * Uses the global variables `consumed_text` and  `ignored_spaces`
 * for pinpointing out exactly where the error is. This obviously doesn't
 * work to well with a proportional font!
 */
void syntax_err( const char *const details )
{
    fflush( stdout );
    if (NO_PREPROC) {
        fprintf( stderr, "\n%*s^\n", ( consumed_text + ignored_spaces ), " " );
        if ( details == NULL ) {
            fprintf( stderr, "%*sSyntax error.\n", ( consumed_text + ignored_spaces ), " " );
        } else {
            fprintf( stderr, "%*sSyntax error: %s\n", ( consumed_text + ignored_spaces ), " ", details );
        }
    } else {
        if ( details == NULL ) {
            fprintf( stderr, "\nSyntax error.\n");
        } else {
            fprintf( stderr, "\nSyntax error: %s\n", details );
        }
    }
}

void syntax_err2( const char *const details1, const char *const details2 )
{
    fflush( stdout );
    if (NO_PREPROC) {
        fprintf( stderr, "\n%*s^\n", ( consumed_text + ignored_spaces ), " " );
        fprintf( stderr, "%*sSyntax error: %s: %s\n", ( consumed_text + ignored_spaces ), " ", details1, details2 );
    } else {
        fprintf( stderr, "\nSyntax error: %s: %s\n", details1, details2 );
    }
}
