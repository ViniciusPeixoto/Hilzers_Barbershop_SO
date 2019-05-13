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

#DEFINE LOTACAO_MAXIMA 20
#DEFINE LUGARES_SOFA 4
#DEFINE LUGARES_SALA 16
#DEFINE NRO_CADEIRAS 3
#DEFINE NRO_CAIXA 1

/*
	Com o objetivo de deixar a leitura do código mais clara, deste ponto em diante, variáveis usadas
	como parâmetros de funções terão seus nomes iniciados com p_, enquanto variáveis criadas com o
	intuito de armazenar um valor referente a continuidade do programa terão seus nomes iniciados com v_.
	Assim, uma variável p_cliente será usada somente dentro de uma função, enquanto uma variável v_cliente
	terá seu valor avaliado pelo código principal e servirá de algum propósito para o resultado final.
*/

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

fila *criaFila(int p_tamanhoFila){
    fila* v_filaCriada = (fila*) malloc(sizeof(fila));		// Alocação de memória para a fila a ser criada
	
    sem_init(&(v_filaCriada->cabeca), 0, 0);				// Inicialmente, ninguém está como primeiro da fila
    sem_init(&(v_filaCriada->cauda),  0, p_tamanhoFila);	// A fila é definida com um tamanho fixo

    return v_filaCriada;
}

/*
	Métodos para o desenvolvimento da fila
*/

void esperaSala(fila *p_sala, int p_cliente){
    sem_wait(&(p_sala->cauda));										// Verifica se há espaço na sala de espera
	printf("O cliente &d entrou na sala de espera", p_cliente);		// Se houver, o cliente entra na sala de espera
    sem_post(&(p_sala->cabeca));									// Se há espaço na sala, há um cliente para sentar no sofá
}

void esperaSofa(fila *p_sofa, int p_cliente){
    sem_wait(&(p_sofa->cauda));										// Verifica se há espaço no sofá
	printf("O cliente &d sentou no sofa", p_cliente);				// Se houver, o cliente senta no sofá
    sem_post(&(p_sofa->cabeca));									// Se há espaço no sofá, há um cliente para ser o próximo a
																	// ser atendido pelo barbeiro
}

void avancaFila(fila *p_fila){
    sem_wait(&(p_fila->cabeca));									// Verifica se o primeiro da fila pode passar para o próximo estágio
    sem_post(&(p_fila->cauda));										// Libera um espaço a mais no final da fila em questão
}

/*
	Métodos principais do programa.
	Aqui serão feitas as ações que darão andamento a todo o processo da barbearia.
*/

/*
	Método para tratar como os clientes se portam dentro da  barbearia
*/

void *rotinaCliente(void *p_arg){
	
	/*
		O cliente atual será representado pelo valor numérico passado aqui.
		O valor numérico vem da chamada de criação da thread
	*/
	int v_clienteAtual = *(int *) p_arg;
	
	/* 
		Um cliente só pode entrar na barbearia se há espaço suficiente para ele esperar sua vez.
		Caso contrário, ele vai embora.
	*/
	if (nroClientes >= 20){
		printf("A barbearia esta cheia e nao cabe mais clientes. Vou embora.");
		return NULL;
	}
	
	/*
		Caso haja espaço, o numero de clientes ativamente na barbearia aumenta.
	*/
	nroClientes++;
	
	/*
		Uma vez que o cliente conseguiu entrar na barbearia, a primeira coisa que ele vai fazer é
		ocupar um espaço na sala de espera.
	*/
	esperaSala(salaEspera, v_clienteAtual);
	
	/*
		Ao entrar na sala de espera, a fila é organizada de forma que o primeiro a entrar vai ocupar
		o primeiro lugar da fila. O segundo vai esperar o primeiro lugar da vila vagar para ocupá-lo.
		Como a operação da fila só recai sobre o primeiro, todos os outros que chegarem depois irão
		esperar sua vez, respeitando a ordem de chegada.
		Isso garante que o cliente só irá passar para a próxima linha caso ele tenha conseguido ser
		o primeiro da fila, isto é, todos os outros antes deles já passaram para o próximo passo antes
		dele.
	*/
	
	/*
		Uma vez que o cliente é a pessoa que está esperando em pé por mais tempo, ele tenta ocupar um
		lugar no sofá.
	*/
	esperaSofa(sofaEspera, v_clienteAtual);
	
	/*
		Com a mesma ideia da sala de espera, somente a pessoa que está sentada a mais tempo, ou seja,
		o primeiro da fila, terá o direito de se levantar e ser atendido. Enquanto isso, aqueles que
		estão sentados no sofá, em ordem de chegada, verificam se a posição de primeiro da fila está vaga.
	*/
	
	/*
		Como no último passo, o cliente conseguiu se sentar, isso significa que um lugar na sala de espera
		está vago, principalmente por se tratar do primeiro lugar, ou seja, a próxima pessoa a se sentar no
		sofá. Portanto, a fila deve andar um passo para frente, permitindo alguém a ocupar o primeiro lugar
		da sala de espera e liberar um lugar no fim da fila da sala de espera.
	*/
	avancaFila(salaEspera);
	
	/*
		Após esperar no sofá, o cliente vai esperar que um barbeiro esteja disponívei para atendê-lo.
		Portanto, ele aguarda que um dos barbeiros o chame.
	*/
	sem_wait(&barbeiro);
}