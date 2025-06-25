car e_maior_que_10(int numero) {
    car resposta;

    se (numero <= 10 ) entao {
        resposta = 'S';
    }
    senao {
        resposta = 'N';
    }

    retorne resposta;
}

programa {
    int a, b, c;
    car chamada_func;

    a = 1;
    b = 2;
    c = a + b;
    chamada_func = e_maior_que_10(c);

    se (chamada_func == 'S') entao {
        escreva "é maior que 10";
        escreva c;
    }
    senao {
        escreva "não é maior que 10";
        escreva c;
    }
}