# example Make command line, when standing in the folder with the makefile:
# AFTER you have installed flex: "make -B BUILD=release", if you skip
# the BUILD parameter, then the debug version will be built.

# I have installed old-flex : flex version 2.5.4
# And I have used gcc version 12.2  on X86-64.

OBJS = multinom.o permtable.o mk_struct.o syntax_err.o finitestate.o\
			 vartables.o expand_expr.o arguments.o

LDFLAGS = -L/usr/local/lib/so64
# where the flex library resides.

LDLIBS = -lfl
# The flex library.

ifeq ($(origin BUILD),undefined)
	# https://stackoverflow.com/questions/38801796/how-to-conditionally-set-makefile-variable-to-something-if-it-is-empty
	# how to make Git-tag in there.
	BUILD := debug
endif

CVERSION := -std=c99
cflags.common := -Wall -Wextra -fPIC
cflags.debug := -g3 -O0  -DTEST
# cflags.debug := -g3 -O0  -static-libasan -DTEST
cflags.sanitize := -g3 -O0 -fsanitize=address,undefined 
cflags.release := -O2 -D_FORTIFY_SOURCE=3 -fstack-protector-strong

CFLAGS  := $(cflags.$(BUILD)) $(cflags.common)

CPPFLAGS := -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=500

COMPILE.c = $(CC) $(CVERSION) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c

LINK := $(CC) $(CVERSION) $(CFLAGS) $(CPPFLAGS) 

LEX = lex


.PHONY: all 

all: multinom tags

multinom: $(OBJS)


tags:
	ls *.c  | sed  '/\.[0-9]\./ d' >c-files
	ctags -L c-files

.PRECIOUS: multinom.c

# vim: foldlevel=99
