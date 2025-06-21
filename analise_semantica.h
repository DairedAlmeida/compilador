#ifndef ANALISE_SEMANTICA_H
#define ANALISE_SEMANTICA_H

// Inclui a definição da estrutura da árvore, pois a função `analisar`
// recebe a raiz da Árvore Sintática Abstrata como seu principal parâmetro.
#include "arvore.h"

// --- Assinatura da Função Principal ---

// Esta é a função principal que inicia todo o processo de análise semântica.
// Ela recebe a árvore sintática completa, gerada pelo analisador sintático (parser).
// A função percorre a árvore, verifica as regras de tipo, escopo e uso de identificadores.
// No projeto original, ela retornaria o número de erros, mas como ele agora aborta no primeiro erro,
// o valor de retorno é menos crítico, mas a assinatura é mantida.
int analisar(No* raiz_arvore);

#endif // ANALISE_SEMANTICA_H