CC = gcc
LEX = flex
YACC = bison
CFLAGS = -g -Wall -std=c99 -D_POSIX_C_SOURCE=200809L
EXEC = goianinha

OBJS = main.o lex.yy.o goianinha.tab.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

main.o: main.c goianinha.tab.h
	$(CC) $(CFLAGS) -c main.c -o main.o

lex.yy.o: lex.yy.c goianinha.tab.h
	$(CC) $(CFLAGS) -c lex.yy.c -o lex.yy.o

goianinha.tab.o: goianinha.tab.c
	$(CC) $(CFLAGS) -c goianinha.tab.c -o goianinha.tab.o

lex.yy.c: goianinha.l
	$(LEX) $<

goianinha.tab.c goianinha.tab.h: goianinha.y
	$(YACC) -d $<

clean:
	rm -f $(OBJS) lex.yy.c goianinha.tab.* $(EXEC)

cleanWin:
	del /Q /F *.o lex.yy.c goianinha.tab.* $(EXEC).exe