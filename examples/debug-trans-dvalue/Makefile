# Set DUKTAPE_SRC to 'src' dir of Duktape distributable.
# The default is for the dist environment.
DUKTAPE_SRC=../../src
DUKTAPE_OPTS=
DUKTAPE_OPTS+=-DDUK_OPT_ASSERTIONS
DUKTAPE_OPTS+=-DDUK_OPT_DEBUGGER_SUPPORT -DDUK_OPT_INTERRUPT_COUNTER
DUKTAPE_OPTS+=-DDUK_OPT_DEBUGGER_FWD_PRINTALERT
DUKTAPE_OPTS+=-DDUK_OPT_DEBUGGER_DUMPHEAP
#DUKTAPE_OPTS+=-DDUK_OPT_DEBUGGER_TRANSPORT_TORTURE
TRANS_OPTS=
#TRANS_OPTS+=-DDEBUG_PRINTS

test: test.c duk_trans_dvalue.c duk_trans_dvalue.h
	echo $(DUKTAPE_SRC)
	gcc -O0 -g -ggdb -Wall -Wextra -std=c99 -o test -I$(DUKTAPE_SRC) -I. \
		$(DUKTAPE_OPTS) $(TRANS_OPTS) \
		$(DUKTAPE_SRC)/duktape.c duk_trans_dvalue.c test.c -lm
