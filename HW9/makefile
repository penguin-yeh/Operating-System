SHELL = /bin/bash
CC = gcc
CFLAGS = -g -O3 -pthread
SRC = $(wildcard *.c)
EXE = $(patsubst %.c, %, $(SRC))

all: ${EXE}

%:	%.c
	${CC} ${CFLAGS} fair.c -o fair

clean:
	rm fair

