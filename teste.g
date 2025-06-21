/*
 * ARQUIVO DE TESTE SEMANTICAMENTE CORRETO
 * ---------------------------------------
 * Este código segue todas as regras semânticas da Goianinha
 * e deve ser compilado sem erros.
 */

// Declaração de variáveis globais
int g_contador;
car g_letra_inicial;


// Declaração de funções antes do bloco 'programa'
int fatorial(int n) {
    se (n == 0) entao {
        retorne 1; // Retorno do tipo 'int', compatível com a função
    }

    retorne n * fatorial(n - 1); // Operadores e operandos do tipo 'int'
}

car proxima_letra(car c, int offset) {
    // Bloco aninhado com shadowing de parâmetro (permitido)
    {
        int c; // 'c' aqui é uma nova variável local, sobrepõe o parâmetro
        c = 100;
        escreva "Valor do 'c' local: ";
        escreva c;
        novalinha;
    }

    // A expressão de retorno deve ser do tipo 'car'
    //retorne c; // Permitido, pois 'car' é tratado como inteiro na aritmética
}


// Bloco principal do programa
programa {
    
        // Declarações de variáveis locais
        int numero;
        int resultado_fatorial;
        car minha_letra;

        // Atribuições com tipos compatíveis e uso de globais
        g_contador = 5;
        g_letra_inicial = 'b';
        numero = g_contador;
        minha_letra = g_letra_inicial;
        
        // Chamada de função com número e tipos de argumentos corretos
        resultado_fatorial = fatorial(numero);

        escreva "Fatorial de ";
        escreva numero;
        escreva " eh: ";
        escreva resultado_fatorial;
        novalinha;

        // A condição do 'enquanto' é do tipo 'int' (resultado de uma comparação)
        enquanto (g_contador > 0) execute {
            // Shadowing de variável global
            int g_contador;
            g_contador = -1; // Esta é uma variável local ao laço

            escreva "Contador local do laco: ";
            escreva g_contador; // Deve imprimir -1
            novalinha;
            
            // A variável global não é afetada aqui
            numero = numero - 1; // Para evitar loop infinito no teste
        }
        
        // Chamada de outra função
        minha_letra = proxima_letra(minha_letra, 1);
        escreva "A proxima letra eh: ";
        escreva minha_letra;
        novalinha;
}