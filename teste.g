/* 
 * Programa exemplo que demonstra todos os recursos da linguagem Goianinha
 * Inclui declarações de variáveis, funções, estruturas de controle, operações, etc.
 */

/* Declaração de variáveis globais */
int global1, global2;
car texto_global;

/* Função que calcula o fatorial de um número */
int fatorial(int n) {
    /* Declaração de variável local */
    int resultado;
    
    /* Estrutura condicional */
    se (n <= 1) entao
        retorne 1;
    senao
        resultado = n * fatorial(n - 1);
    
    retorne resultado;
}

/* Função que verifica se um número é primo */
int eh_primo(int num) {
    int i;
    
    se (num <= 1) entao
        retorne 0;
    
    /* Estrutura de repetição */
    enquanto (i * i <= num) execute {
        se (num e i == 0) entao
            retorne 0;
        i = i + 1;
    }
    
    retorne 1;
}

/* Função principal */
programa {
    /* Declaração de variáveis locais */
    int a, b, c, opcao;
    car caractere;
    
    /* Atribuições e operações aritméticas */
    a = 10;
    b = 20;
    c = a + b * 3 - 5 / 2;
    
    /* Chamada de função */
    c = fatorial(5);
    
    /* Entrada de dados */
    leia opcao;
    leia caractere;
    
    /* Saída de dados */
    escreva "O fatorial de 5 é: ";
    escreva c;
    novalinha;
    
    escreva "Digite um número para verificar se é primo: ";
    leia a;
    
    /* Estrutura condicional completa com operações lógicas */
    se (eh_primo(a) == 1) entao
        escreva "O número é primo";
    senao
        escreva "O número não é primo";
    novalinha;
    
    /* Operações de comparação */
    se (a > b e a < 100) entao {
        escreva "O número está entre ";
        escreva b;
        escreva " e 100";
        novalinha;
    }
    
    /* Operações lógicas complexas */
    se ((a == 0 ou b == 0) e (c < 0)) entao
        escreva "Condição complexa satisfeita";
    novalinha;
    
    /* Bloco de código aninhado */
    enquanto (opcao != 0) execute {
        escreva "Menu:";
        novalinha;
        escreva "1 - Calcular fatorial";
        novalinha;
        escreva "2 - Verificar primo";
        novalinha;
        escreva "0 - Sair";
        novalinha;
        escreva "Opção: ";
        
        leia opcao;
        
        se (opcao == 1) entao {
            escreva "Digite um número: ";
            leia a;
            escreva "Fatorial: ";
            escreva fatorial(a);
            novalinha;
        } senao se (opcao == 2) entao {
            escreva "Digite um número: ";
            leia a;
            se (eh_primo(a)) entao
                escreva "É primo";
            senao
                escreva "Não é primo";
            novalinha;
        }
    }
    
    /* Operações com strings (cadeias de caracteres) */
    escreva "Programa encerrado. Obrigado!";
    novalinha;
    
    /* Expressões complexas */
    c = (a + b) * (a - b) / 2 + (10 / 3);
    
    /* Operadores relacionais diversos */
    se (a >= b ou a <= c e b != a) entao
        escreva "Condição relacional satisfeita";
    novalinha;
    
    /* Uso de todos os operadores */
    a = a + 1;
    b = b - 1;
    c = c * 2;
    a = a / 2;
    
    /* Operadores unários */
    a = -a;
    
    retorne 0;
}