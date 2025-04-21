/* Teste completo para o compilador Goianinha
   - Declarações de variáveis e funções
   - Estruturas de controle
   - Operações aritméticas e lógicas
   - Entrada/saída */

int global;  /* Variável global */

/* Função simples */
int soma(int a, int b) {
    retorne a + b;
}

/* Programa principal */
programa {
    int x, y, resultado;
    car caractere;
    
    /* Atribuições e operações */
    x = 10;
    y = 5 * 2 + 3;
    global = x - y;
    caractere = "A";
    
    /* Chamada de função */
    resultado = soma(x, y);
    
    /* Estruturas condicionais */
    se (resultado > 10) entao {
        escreva "Resultado maior que 10";
    } senao {
        escreva "Resultado menor ou igual a 10";
    }
    
    /* Loop enquanto */
    enquanto (x > 0) execute {
        escreva x;
        x = x - 1;
    }
    
    /* Entrada/saída */
    leia y;
    escreva "O valor de y é: ", y;
    
    /* Operações lógicas */
    se (x == 0 && y != 0) entao {
        escreva "x é zero e y é não-zero";
    }
    
    novalinha;
}