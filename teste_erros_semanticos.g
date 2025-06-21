// Arquivo: teste_erros_semanticos.g
// Objetivo: Testar a detecção de erros pelo analisador semântico.
// O código é sintaticamente válido, mas viola as regras da linguagem.

// Função para auxiliar nos testes
int soma(int x, int y) {
    retorne x + y;
}

// ERRO SEMÂNTICO: Função declarada com um tipo de retorno, mas retorna outro
/*
int func_retorno_errado() {
    retorne 'x'; // Deve retornar INT
}
*/

programa {
    int i;
    int j;
    car c;

    // ERRO SEMÂNTICO: Uso de variável não declarada
    //k = 10; 

    // ERRO SEMÂNTICO: Redeclaração de variável no mesmo escopo
    //int i; 

    // ERRO SEMÂNTICO: Incompatibilidade de tipos na atribuição
    //i = 'a'; // Atribuindo CAR a um INT
    //c = 120; // Atribuindo INT a um CAR

    // ERRO SEMÂNTICO: Incompatibilidade de tipos em expressões
    //j = i + c; // Operação aritmética entre INT e CAR
    
    // --- Erros de Função ---

    // ERRO SEMÂNTICO: Chamada de função não declarada
    //funcao_inexistente();

    // ERRO SEMÂNTICO: Número incorreto de argumentos na chamada da função
    //j = soma(5); // Faltou o segundo argumento

    // ERRO SEMÂNTICO: Tipos incorretos dos argumentos na chamada da função
    //j = soma(10, 'b'); // O segundo argumento deveria ser INT, não CAR

    // ERRO SEMÂNTICO: Tipo de retorno da função incompatível com a atribuição
    //c = soma(1, 2); // 'soma' retorna INT, que não pode ser atribuído a 'c' (CAR)

    novalinha;
}