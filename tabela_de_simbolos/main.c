#include <stdio.h>
#include "tabela_de_simbolos.h"

int main() {
    PilhaDeTabelas pilha;
    inicializarPilha(&pilha);

    empilhar(&pilha);
    inserirVariavelNaPilha(&pilha, "x", 1, 0);
    inserirFuncaoNaPilha(&pilha, "func", 2, 1);
    inserirParametroNaPilha(&pilha, "param1", 1, 0, buscarNaPilha(&pilha, "func"));

    empilhar(&pilha);
    inserirVariavelNaPilha(&pilha, "y", 2, 1);
    inserirVariavelNaPilha(&pilha, "x", 2, 2);

    printf("\n====================================================\n");
    imprimirPilha(&pilha);

    Simbolo* encontrado = buscarNaPilha(&pilha, "x");
    if (encontrado) {
        printf("\nValor de x no escopo atual: %d\n", encontrado->posicao);
    } else {
        printf("x não encontrado.\n");
    }
    printf("====================================================\n");

    desempilhar(&pilha);

    printf("\n====================================================\n");
    imprimirPilha(&pilha);
    encontrado = buscarNaPilha(&pilha, "x");
    if (encontrado) {
        printf("\nValor de x no escopo atual: %d\n", encontrado->posicao);
    } else {
        printf("x não encontrado.\n");
    }
    printf("====================================================\n");

    return 0;
}
