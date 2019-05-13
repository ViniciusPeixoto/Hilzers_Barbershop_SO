#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>


/* Estrutura da fila na forma <cabeca|cauda>
 * (muito comum em linguagens de paradigma logico e
 * funcional. Promove um melhor desempenho, principalmente
 * por possibilitar operacoes como recursividade na cauda)
 */
typedef struct{
    sem_t cabeca;
    sem_t cauda;
} fila;

/* Criacao das filas globais para representar
 * a sala de espera e o sofa de espera. */
fila *sala_de_espera,*sofa;

/* Criacao dos semaforos para o desenvolvimento
 * do codigo e controle das threads */
sem_t barbeiro, cadeira, caixa;
sem_t cliente, faz_pagamento, recebe_troco;

/* Contador de clientes, util para saber se a
 * barbearia nao esta lotada */
int   clientes = 0;

/* Metodo para criacao da fila da forma <cabeca|cauda>
 * inicializando um ponteiro f com tamanho fila e em
 * seguida retornando o ponteiro criado
 */

fila* cria_fila(int tam){
    fila* f = (fila*) malloc(sizeof(fila));
    sem_init(&(f->cabeca), 0, 0);
    sem_init(&(f->cauda),  0, tam);

    return f;
}

/* Metodo para fazer o cliente esperar o desenvolvimento
 * da fila */
void espera_fila(fila* f, int cliente_x){
    sem_wait(&(f->cauda));
    sem_post(&(f->cabeca));
}

/* Metodo para deslocar a fila adiante, toda vez que um novo
 * espaço estiver disponivel */
void avanca_fila(fila* f){
    sem_wait(&(f->cabeca));
    sem_post(&(f->cauda));
}

/* Um dos dois (1/2) metodos principais do codigo. Quando chamado, o
 * metodo define o fluxo de execução da barbearia com auxilio dos
 * semaforos definidos anteriormente */
void* execucao_cliente(void *arg, int limite){
    int cliente_x = *(int *) arg;

    if(clientes>=20)
        printf("Barbearia cheia. %d indo embora da barbearia \n", cliente_x);

    clientes++;

    espera_fila(sala_de_espera, cliente_x);
    printf("Cliente %d entrando na sala de espera\n", cliente_x);

    espera_fila(sofa, cliente_x);
    printf("Cliente %d sentando no sofa\n", cliente_x);

    avanca_fila(sala_de_espera);

    sem_wait(&cadeira);
    printf("Cliente %d sentando na cadeira do barbeiro\n", cliente_x);
    sleep(3);

    avanca_fila(sofa);

    sem_post(&cliente);
    sem_wait(&barbeiro);
    printf("Cliente %d cortando o cabelo\n", cliente_x);
    sleep(3);

    sem_post(&faz_pagamento);
    printf("Cliente %d fazendo pagamento\n", cliente_x);
    sem_wait(&recebe_troco);
    printf("Cliente %d recebendo troco\n", cliente_x);

    clientes--;

    printf("Cliente %d indo embora\n", cliente_x);
    sem_post(&cadeira);
}

/* Um dos dois (2/2) metodos principais do codigo. Quando chamado, o
 * metodo define o fluxo de execução da barbearia por parte do barbeiro */
void* execucao_barbeiro(void* arg){
    int barbeiro_x = *(int *) arg;

    while(1){
        sem_wait(&cliente);
        sem_post(&barbeiro);
        printf("\t\t\tBarbeiro %d cortando o cabelo\n", barbeiro_x);
        sleep(3);

        sem_wait(&faz_pagamento);
        sem_wait(&caixa);
        printf("\t\t\tBarbeiro %d usando o caixa\n", barbeiro_x);
        sleep(3);
        sem_post(&caixa);

        printf("\t\t\tBarbeiro %d dando o troco\n", barbeiro_x);
        sem_post(&recebe_troco);

        printf("\t\t\tBarbeiro %d livre para o proximo\n", barbeiro_x);
    }
}

int main(int argc, char *argv[]){

    int qtn_clientes = atoi(argv[1]);
    int qtn_sofa     = atoi(argv[2]);
    int qtn_sala     = atoi(argv[3]);
    int qtn_barbcad  = atoi(argv[4]);
    int qtn_caixa    = atoi(argv[5]);

    sem_init(&cadeira,0,qtn_barbcad);
    sem_init(&barbeiro,0,0);
    sem_init(&cliente,0,0);
    sem_init(&faz_pagamento,0,0);
    sem_init(&caixa,0,qtn_caixa);
    sem_init(&recebe_troco,0,0);

    sala_de_espera = cria_fila(qtn_sala);
    sofa = cria_fila(qtn_sofa);

    pthread_t _cliente[qtn_clientes];
    pthread_t _barbeiro[qtn_barbcad];

    int i_cliente[qtn_clientes];

    int i;
    for(i=0; i<qtn_clientes; i++){
        i_cliente[i]=i;
        pthread_create(&_cliente[i], 0, (void *) execucao_cliente, &i_cliente[i]);
    }
    for(i=0;i<qtn_barbcad;i++)
        pthread_create(&_barbeiro[i], 0, (void *) execucao_barbeiro,&i_cliente[i]);

    while(clientes!=0);

    free(sala_de_espera);
    free(sofa);

    return 0;
}
