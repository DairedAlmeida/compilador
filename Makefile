# -----------------------------------------------------------------------------
# Makefile para o Compilador da Linguagem Goianinha
# -----------------------------------------------------------------------------

# Compilador a ser usado
CC = gcc

# Flags de compilação: -g para informações de debug, -Wall para todos os warnings
CFLAGS = -g -Wall

# Ferramentas de geração
LEX = flex
BISON = bison

# Flags de linkagem: -lfl é necessária para a biblioteca do Flex
LDFLAGS = -lfl

# Nome do executável final
EXEC = goianinha

# Arquivos de código-fonte (.c) do projeto.
# Os arquivos gerados (goianinha.tab.c, lex.yy.c) são adicionados automaticamente.
SRCS = main.c \
       arvore.c \
       tabela_simbolos.c \
       analise_semantica.c \
       geracao_codigo.c

# Converte a lista de fontes (.c) para uma lista de objetos (.o)
OBJS = $(SRCS:.c=.o)

# Arquivos gerados pelo Flex e Bison
GENERATED_SRCS = goianinha.tab.c lex.yy.c
GENERATED_OBJS = $(GENERATED_SRCS:.c=.o)
GENERATED_HDRS = goianinha.tab.h

# --- Regras do Makefile ---

# A regra padrão, chamada quando se executa 'make' sem argumentos.
# O alvo é o executável final.
all: $(EXEC)

# Regra de linkagem: cria o executável final a partir de todos os arquivos objeto.
$(EXEC): $(OBJS) $(GENERATED_OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS) $(GENERATED_OBJS) $(LDFLAGS)
	@echo "Compilador '$(EXEC)' criado com sucesso!"

# Regra para gerar o parser do Bison e o cabeçalho correspondente.
# O '-d' cria o arquivo de cabeçalho goianinha.tab.h.
goianinha.tab.c goianinha.tab.h: goianinha.y
	$(BISON) -d goianinha.y

# Regra para gerar o scanner do Flex.
# Depende do cabeçalho gerado pelo Bison para conhecer os tokens.
lex.yy.c: goianinha.l goianinha.tab.h
	$(LEX) goianinha.l

# Regra de compilação genérica: transforma qualquer arquivo .c em .o.
# O alvo (.o) depende do seu respectivo .c e dos cabeçalhos gerados.
%.o: %.c $(GENERATED_HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

# Regra "clean": remove todos os arquivos gerados pela compilação.
# Útil para forçar uma reconstrução completa do projeto.
clean:
	rm -f $(EXEC) $(OBJS) $(GENERATED_OBJS) $(GENERATED_SRCS) $(GENERATED_HDRS)
	rm -f *.asm
	@echo "Limpeza concluída."

# Declara os alvos que não são arquivos reais.
.PHONY: all clean