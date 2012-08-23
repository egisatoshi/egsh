SHELL = /bin/sh

CC    = gcc -Wall -g
OBJS  = egsh.o builtin.o command.o pgrp.o signal.o

egsh: $(OBJS)
	$(CC) -o $@ $(OBJS)

.c.o:
	$(CC) -c $<

egsh.h: builtin.h command.h pgrp.h signal.h
egsh.o: egsh.h
builtin.o: builtin.h pgrp.h
command.o: command.h builtin.h
pgrp.o: pgrp.h
signal.o: signal.h pgrp.h
