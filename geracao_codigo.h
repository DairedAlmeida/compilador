// geracao_codigo.h

#ifndef GERACAO_CODIGO_H
#define GERACAO_CODIGO_H

#include "arvore.h"

/**
 * @brief Função principal para iniciar a geração de código.
 *
 * Percorre a Árvore Sintática Abstrata (ASA) e gera o código assembly MIPS
 * em um arquivo de saída.
 *
 * @param raiz_arvore Ponteiro para o nó raiz da ASA.
 * @param nome_arquivo_saida O nome do arquivo .asm a ser criado.
 */
void gerar_codigo(No* raiz_arvore, const char* nome_arquivo_saida);
TipoDado string_para_tipo(char* str);

#endif // GERACAO_CODIGO_H