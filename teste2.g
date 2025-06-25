int g_contador;

int somar_numeros(int x, int y) {
    int resultado;
    resultado = x + y;
    retorne resultado;
}

car trocar(car troca, int bo) {
    se (bo == 0) entao
        retorne troca;
    senao
        retorne 'a';
}

programa {
    car teste, trocar1, trocar2;
    int a, b, c;
    a = 1;
    b = 2;
    teste = 'b';
    g_contador = 5;
    
    c = somar_numeros(a,b);
    escreva "c: ";
    escreva c;
    novalinha;

    escreva "teste: ";
    escreva teste;
    novalinha;

    trocar1 = trocar(teste, 1);
    escreva "trocar1: ";
    escreva trocar1;
    novalinha;

    trocar2 = trocar(teste, 0);
    escreva "trocar2: ";
    escreva trocar2;
    novalinha;
    
    enquanto (g_contador > 0) execute {
        escreva "Contagem regressiva global: ";
        escreva g_contador;
        novalinha;
        g_contador = g_contador - 1;
    }
}