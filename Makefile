PWD := ~/C/ccsocket/
LN := ~/bin/
CC := clang
CFLAG := -g -Wall -o
SOUT := servercc
COUT := clientcc

.PHONY: install clean all

all: $(SOUT).out $(COUT).out

$(SOUT).out: $(SOUT).c
	$(CC) $(CFLAG) $@ $<

$(COUT).out: $(COUT).c
	$(CC) $(CFLAG) $@ $<

install:
	ln -s $(PWD)$(SOUT).out $(LN)$(SOUT)
	ln -s $(PWD)$(COUT).out $(LN)$(COUT)

clean:
	rm $(PWD)$(SOUT).out
	rm $(LN)$(SOUT)
	rm $(PWD)$(COUT).out
	rm $(LN)$(COUT)
	

	
	