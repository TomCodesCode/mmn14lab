CC=gcc
DEBUG=-g -O0
CFLAGS=-I. $(DEBUG) -Wall -Werror -pedantic -ansi
DEPS:= $(wildcard *.h)
CFILES = front.c errors.c datastruct.c

mmn14lab: $(CFILES) $(DEPS)
	$(CC) -o $@ $(CFILES) $(CFLAGS)
	-cp $@ a.out

all: mmn14lab

clean:
	-rm mmn14lab *.o a.out
