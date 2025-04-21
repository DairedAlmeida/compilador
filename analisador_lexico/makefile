#windows e linux
CC = gcc
CFLAGS = -g -Wall -ansi
LFLAGS = -lm

goianinha: main.o lex.yy.o
	$(CC) $(CFLAGS) $(LFLAGS) lex.yy.o main.o -o goianinha

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

lex.yy.c: goianinha.l
	flex goianinha.l

lex.yy.o: lex.yy.c
	$(CC) $(CFLAGS) -c lex.yy.c -o lex.yy.o

clean:
	rm -f lex.yy.c *.o goianinha

cleanObj:
	rm -f *.o