CC=gcc
DEBUG=-g -O0
CFLAGS=-I. $(DEBUG) -Wall -Werror -pedantic -ansi
DEPS:= $(wildcard *.h)
CFILES = mmn14lab.c front.c errors.c datastruct.c middle.c back.c preproc.c debug.c

mmn14lab: $(CFILES) $(DEPS)
	$(CC) -o $@ $(CFILES) $(CFLAGS)

all: mmn14lab

clean:
	-rm mmn14lab
