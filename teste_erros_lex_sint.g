// Arquivo: teste_erros_sintaticos.g
// Objetivo: Testar a detecção de erros pelo analisador sintático.
// Cada trecho de código abaixo viola uma regra da gramática.

// ERRO SINTÁTICO: Ponto e vírgula faltando após a declaração de variável
/*
programa {
    int a
    a = 10;
}
*/
// ERRO SINTÁTICO: Palavra-chave 'programa' no lugar errado (dentro de um bloco)
/*
programa {
    int a;
    programa {} 
}
*/

// ERRO SINTÁTICO: Faltando a palavra-chave 'entao' no comando 'se'
/*
programa {
    se (1 > 0) {
        escreva("oi");
    }
}
*/
// ERRO SINTÁTICO: Parênteses desbalanceados em uma expressão
/*
programa {
    int x = (5 + 3 * (2 );
}
*/

// ERRO SINTÁTICO: Uso incorreto do 'senao' sem um 'se' correspondente
/*
programa {
    int y = 10;
    senao {
        y = 20;
    }
}
*/

// ERRO SINTÁTICO: Atribuição do lado errado de uma expressão
/*
programa {
    int z;
    5 = z;
}
*/

// ERRO SINTÁTICO: Argumentos de função separados incorretamente (ex: ponto e vírgula em vez de vírgula)
/*
int minha_func(int a; int b) {
    retorne a+b;
}
programa {
    minha_func(1; 2);
}
*/

// ERRO SINTÁTICO: Declaração de função dentro do bloco principal (se a gramática não permitir)
// Baseado na gramática corrigida, isso seria um erro.

/*
programa {
    {
        int f() { retorne 1; }
    }
}
*/

// ERRO SINTÁTICO: Comando 'retorne' fora de uma função
/*
programa {
    {
       retorne 10;
    }
}
*/

programa {
    novalinha;
}