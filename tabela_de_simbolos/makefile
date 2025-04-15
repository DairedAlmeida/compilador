CC = gcc

CFLAGS = -g -Wall
LFLAGS = -lm

EXEC = main

OBJ = tabela_de_simbolos.o main.o

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXEC) $(LFLAGS)

main.o: main.c tabela_de_simbolos.h
	$(CC) $(CFLAGS) -c main.c -o main.o

tabela_de_simbolos.o: tabela_de_simbolos.c tabela_de_simbolos.h
	$(CC) $(CFLAGS) -c tabela_de_simbolos.c -o tabela_de_simbolos.o

clean:
	rm -f *.o $(EXEC)