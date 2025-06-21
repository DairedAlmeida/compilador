// Arquivo: teste_sucesso_completo.g
// Objetivo: Testar todas as funcionalidades corretas da linguagem de forma integrada.
// Deve compilar sem erros léxicos, sintáticos ou semânticos.

// --- Declarações Globais ---
int g_contador;
car g_letra_status;

// --- Declaração de Funções ---

// Função para cálculo fatorial (recursivo)
// Testa: recursão, parâmetros, retorno de valor, if-else, operadores.
int fatorial(int n) {
    int resultado_parcial; // Variável de escopo local
    
    se (n <= 1) entao {
        retorne 1;
    } senao {
        resultado_parcial = fatorial(n - 1);
        retorne n * resultado_parcial;
    }
}

// Função para processar e imprimir status
// Testa: múltiplos parâmetros, escopo, shadowing, comandos de E/S.
int processa_status(int g_contador, car novo_status) {
    int status_interno; // Esta variável "g_contador" é um parâmetro, sombreando a global.
    
    escreva "Processando status...";
    novalinha;
    escreva "Contador local (parametro): ";
    escreva g_contador;
    novalinha;

    se (novo_status == 'A' ou novo_status == 'I') entao {
        g_letra_status = novo_status; // Modifica a variável global
    } senao {
        g_letra_status = '?';
    }
    
    retorne 0; // Retorno de sucesso
}
    

programa {
    
    // --- Bloco Principal de Execução ---
    {
        int numero_leitura, resultado_fat;
        car char_lido;
        char_lido = 'A'; // Atribuição de char

        escreva "Digite um numero para calcular o fatorial: ";
        leia numero_leitura;

        // Testa chamada de função, atribuição e expressões complexas
        se (numero_leitura > 0 e !(numero_leitura > 10)) entao {
            resultado_fat = fatorial(numero_leitura);
            escreva "O fatorial eh: ";
            escreva resultado_fat;
            novalinha;
        } senao {
            escreva "Numero invalido (deve ser entre 1 e 10)";
            novalinha;
        }
        
        // Testa o comando 'enquanto' e operadores lógicos
        g_contador = 5;
        enquanto (g_contador > 0) execute {
            escreva "Contagem regressiva global: ";
            escreva g_contador;
            novalinha;
            g_contador = g_contador - 1;
        }

        // Testa chamada com múltiplos parâmetros
        processa_status(100, char_lido);

        escreva "Status final: ";
        escreva g_letra_status;
        novalinha;
        
        escreva "Fim do programa.";
        novalinha;
    }
}