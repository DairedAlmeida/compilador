CC = gcc
LEX = flex
YACC = bison
CFLAGS = -g -Wall -std=c99 -D_POSIX_C_SOURCE=200809L
EXEC = goianinha

# 1. CORREÇÃO: Adicionado 'analise_semantica.o' à lista de objetos.
OBJS = main.o lex.yy.o goianinha.tab.o arvore.o tabela_simbolos.o analise_semantica.o

all: $(EXEC)

# O linker agora receberá analise_semantica.o e encontrará a função 'analisar'.
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

# Adicionada a dependência 'analise_semantica.h' para main.o, por boas práticas.
main.o: main.c goianinha.tab.h arvore.h tabela_simbolos.h analise_semantica.h
	$(CC) $(CFLAGS) -c main.c -o main.o

# 2. CORREÇÃO: Adicionada a regra para compilar analise_semantica.c
analise_semantica.o: analise_semantica.c analise_semantica.h arvore.h tabela_simbolos.h
	$(CC) $(CFLAGS) -c analise_semantica.c -o analise_semantica.o

arvore.o: arvore.c arvore.h
	$(CC) $(CFLAGS) -c arvore.c -o arvore.o

tabela_simbolos.o: tabela_simbolos.c tabela_simbolos.h arvore.h
	$(CC) $(CFLAGS) -c tabela_simbolos.c -o tabela_simbolos.o

lex.yy.o: lex.yy.c goianinha.tab.h
	$(CC) $(CFLAGS) -c lex.yy.c -o lex.yy.o

goianinha.tab.o: goianinha.tab.c arvore.h tabela_simbolos.h
	$(CC) $(CFLAGS) -c goianinha.tab.c -o goianinha.tab.o

lex.yy.c: goianinha.l
	$(LEX) $<

goianinha.tab.c goianinha.tab.h: goianinha.y
	$(YACC) -d $<

clean:
	rm -f $(OBJS) lex.yy.c goianinha.tab.* $(EXEC)

cleanWin:
	del /Q /F *.o lex.yy.c goianinha.tab.* $(EXEC).exe