/*
	Trabalho de Sistemas Operacionais 100153-5 semestre 2019/1
	
	Grupo 6
	Amanda Lima Ribeiro - 743504
	Jean Araujo - 620394
	Rodrigo Sato Gomes - 619809
	Vinicius de Oliveira Peixoto - 628263
	
	Hilzer’s Barbershop Problem
*/

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

/*
	Definição da estrutura FILA.
	Esta estrutura será usada para ordenar os clientes da barbearia.
*/
typedef struct{
	/*
		Os semáforos servem para permitir o funcionamento correto das filas, já que
		elas só tem fluxo de clientes no começo, quando o primeiro da fila tem a sua vez, e no final,
		quando um novo cliente entra na fila.
	*/
    sem_t cabeca;
    sem_t cauda;
} fila;

/*
	A barbearia possui duas condições de espera antes do cliente ser atendido:
	Um sofá, para clientes que estão esperando e serão atendidos em breve, e
	uma sala de espera, para clientes que esperarão mais tempo, em pé.
*/

fila *sofaEspera, *salaEspera;

/*
	Os recursos do sistema também são representados por semáforos, uma vez que eles estão disponíveis
	somente para um único barbeiro/cliente.
*/

sem_t barbeiro, cadeira, caixa;
sem_t cliente, fazPagamento;

/*
	Controle de clientes dentro da barbearia.
*/

int nroClientes = 0;

/*
	Método para criação e inicialização das filas.
*/

fila* criaFila(int tamanhoFila){
    fila* f = (fila*) malloc(sizeof(fila));
    sem_init(&(f->cabeca), 0, 0);				// Inicialmente, ninguém está como primeiro da fila
    sem_init(&(f->cauda),  0, tamanhoFila);		// A fina é definida com um tamanho fixo

    return f;
}

/*
	Métodos para o desenvolvimento da fila
*/

void esperaSala(fila* f, int cliente){
    sem_wait(&(f->cauda));										// Verifica se há espaço na sala de espera
	printf("O cliente &d entrou na sala de espera", cliente);	// Se houver, o cliente entra na sala de espera
    sem_post(&(f->cabeca));										// Se há espaço na sala, há um cliente para sentar no sofá
}

void esperaSofa(fila* f, int cliente){
    sem_wait(&(f->cauda));										// Verifica se há espaço no sofá
	printf("O cliente &d sentou no sofa", cliente);				// Se houver, o cliente senta no sofá
    sem_post(&(f->cabeca));										// Se há espaço no sofá, há um cliente para ser o próximo a
																// ser atendido pelo barbeiro
}

void avancaFila(fila* f){
    sem_wait(&(f->cabeca));										// Verifica se o primeiro da fila pode passar para o próximo estágio
    sem_post(&(f->cauda));										// Libera um espaço a mais no final da fila em questão
}

/*
	Métodos principais do programa.
	Aqui serão feitas as ações que darão andamento a todo o processo da barbearia.
*/

/*
	Método para tratar como os clientes se portam dentro da  barbearia
*/