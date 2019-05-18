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

#define LOTACAO_MAXIMA 20	// Quantidade máxima de clientes
#define LUGARES_SOFA 4		// Quantidade de lugares no sofá
#define LUGARES_SALA 16		// Quantidade de lugares na sala de espera
#define NRO_CADEIRAS 3		// Quantidade de cadeiras de barbeiro
#define NRO_BARBEIROS 3		// Quantidade de barbeiros
#define NRO_CAIXA 1			// Quantidade de caixas registradoras

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

sem_t barbeiro, corte, cadeira, caixa;			// Ações da barbearia
sem_t cliente, fazPagamento, recebeTroco;		// Ações dos clientes

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
	printf("O cliente %d entrou na sala de espera\n", p_cliente);		// Se houver, o cliente entra na sala de espera
    sem_post(&(p_sala->cabeca));									// Se há espaço na sala, há um cliente para sentar no sofá
}

void esperaSofa(fila *p_sofa, int p_cliente){
    sem_wait(&(p_sofa->cauda));										// Verifica se há espaço no sofá
	printf("O cliente %d sentou no sofa\n", p_cliente);				// Se houver, o cliente senta no sofá
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
		O valor numérico vem da chamada de criação da thread.
	*/
	int v_clienteAtual = *(int *) p_arg;
	
	/* 
		Um cliente só pode entrar na barbearia se há espaço suficiente para ele esperar sua vez.
		Caso contrário, ele vai embora.
	*/
	if (nroClientes >= 20){
		printf("A barbearia esta cheia e nao cabe mais clientes. Vou embora.\n");
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
	sem_post(&cliente);			// Há um cliente esperando atendimento
	
	/*
		O cliente deve se sentar em uma das 3 cadeiras. Qual delas não importa.
	*/
	sem_wait(&cadeira);
	
	sem_wait(&barbeiro);		// Há um barbeiro para atendê-lo?
	
	/*
		Com o barbeiro disponível, temos então o serviço começando a ser feito.
	*/
	printf("O cliente %d esta sendo atendido agora\n", v_clienteAtual);
	sleep(3);		// Simulação de que as ações levam algum tempo para serem feitas
	
	printf("O cliente %d esta sentado na cadeira do barbeiro", v_clienteAtual);
	sleep(3);		// Simulação de que as ações levam algum tempo para serem feitas
	
	/*
		Como o cliente tomou seu lugar na cadeira do barbeiro agora, então o sofá tem um lugar vago. Então
		os clientes movem a fila do sofá para frente. Isso define qual é o cliente que será o próximo a ser
		atendido, bem como libera um espaço no sofá para alguém sentar.
	*/
	avancaFila(sofaEspera);
	
	/*
		Com tudo definido, o serviço começa a ser feito.
	*/
	sem_post(&corte);
	printf("O cliente %d esta tendo seu cabelo cortado\n", v_clienteAtual);
	sleep(10);		// Simulação de que as ações levam algum tempo para serem feitas
	
	/*
		Após o corte realizado, o barbeiro está livre para seus outros afazeres
	*/
	sem_post(&barbeiro);
	
	/*
		Porém, o cliente ainda precisa realizar o pagamento pelo serviço. Portanto ele oferece o pagamento
		para o primeiro barbeiro que estiver disponível. Lembrando que qualquer barbeiro pode receber o pagamento.
	*/
	sem_post(&fazPagamento);
	printf("O cliente %d quer pagar pelo servico\n", v_clienteAtual);
	
	/*
		Depois de pagar, ele aguarda o barbeiro usar a caixa registradora para guardar o pagamento e pegar o troco.
	*/
	sem_wait(&recebeTroco);
	printf("O cliente %d recebeu seu troco\n", v_clienteAtual);
	
	/*
		Finalmente o cliente desocupa a cadeira e vai embora da barbearia.
	*/
	sem_post(&cadeira);
	printf("O cliente %d esta indo embora\n", v_clienteAtual);
	nroClientes--;
}

/*
	Método para tratar como os clientes se portam dentro da  barbearia
*/
void *rotinaBarbeiro(void *p_arg){
	
	/*
		O barbeiro atual será representado pelo valor numérico passado aqui.
		O valor numérico vem da chamada de criação da thread.
	*/
	int v_barbeiroAtual = *(int *) p_arg;
	
	/*
		Os barbeiros sempre estarão fazendo algo enquanto há clientes dentro da barbearia.
		Por isso, suas funções estarão dentro de um condicional de loop infinito.
	*/
	while(1){
		sem_post(&barbeiro);		// Há um barbeiro para atendimento
		sem_wait(&cliente);			// Há um cliente para ser atendido?
		
		/*
			O corte de cabelo começa a ser feito pelo barbeiro
		*/
		sem_wait(&corte);
		printf("O barbeiro %d está cortando cabelo\n", v_barbeiroAtual);
		sleep(10);
		
		sem_wait(&barbeiro);			// Há um barbeiro para receber o pagamento?
		
		/*
			O barbeiro que estiver disponível irá receber pelo corte realizado por um cliente.
			Como um barbeiro acabou de ficar disponível após o serviço prestado, é provável que
			ele atenda o pagamento, mas não é necessáriamente sempre ele.
		*/
		sem_wait(&fazPagamento);
		sem_wait(&caixa);				// Acesso à caixa registradora
		printf("O barbeiro %d está usando a caixa registradora\n", v_barbeiroAtual);
		sleep(5);			// Simulação de que as ações levam algum tempo para serem feitas
		sem_post(&caixa);				// Libera o uso da caixa registradora
		
		/*
			Após usar a caixa registradora, o barbeiro dá o troco para o cliente
		*/
		printf("O barbeiro %d está passando troco\n", v_barbeiroAtual);
		sem_post(&recebeTroco);
		
		/*
			O barbeiro estará livre para atender o próximo cliente
		*/
		printf("O barbeiro %d esta livre\n", v_barbeiroAtual);
	}
}

/*
	Função que avalia dois inteiros e retorna o maior deles
*/
int maxInt(int p_valor1, int p_valor2){
	
	if (p_valor1 > p_valor2) {
		return p_valor1;
	}
	
	return p_valor2;
}

/*
	Programa principal
*/
int main(int argc, char *argv[]){
	
	/*
		Como argumento deve ser passado quantos clientes querem usar a barbearia
	*/
	if (argc > 2) {
		printf("Muitos argumentos passados. Erro!\n");
		return -1;
	}
	int quantidadeClientes = atoi(argv[1]);
	
	
	/*
		Realiza-se a inicialização dos semáforos que controlarão o fluxo de tarefas
	*/
	sem_init(&cadeira,0,NRO_CADEIRAS);		// Só é possível usar NRO_CADEIRAS de cadeiras de barbeiro
    sem_init(&barbeiro,0,0);				// Os barbeiros são controlados por threads de barbeiro
    sem_init(&cliente,0,0);					// Os clientes são controlados por threads de clientes
    sem_init(&fazPagamento,0,0);			// Um pagamento só pode ser feito quando um cliente quiser pagar
    sem_init(&caixa,0,NRO_CAIXA);			// Só é possível usar NRO_CAIXA de caixas registradoras
    sem_init(&recebeTroco,0,0);				// O troco só pode ser passado quando um barbeiro quiser passar
	
	/*
		As filas de semáforos para a sala e o sofá são criadas
		Seus tamanhos dependem do número de LUGARES_SALA e LUGARES_SOFA
		Os clientes só avançam nessa fila quando o primeiro da fila sair, liberando um novo espaço no final da fila.
	*/
	salaEspera = criaFila(LUGARES_SALA);
	sofaEspera = criaFila(LUGARES_SOFA);
	
	/*
		Os clientes e barbeiros serão tratados como threads, aqui organizados em vetores
		Isso se faz porque as ações dos clientes e dos barbeiros ocorrem em paralelo
		Por exemplo, um barbeiro pode estar cortando o cabelo enquanto outro está passando troco, bem como um cliente
		pode estar em pé na sala de espera enquanto outro está tendo seu cabelo cortado.
	*/
	pthread_t vetorClientes[quantidadeClientes];
	pthread_t vetorBarbeiros[NRO_BARBEIROS];
	
	/*
		Cria os valores dos identificadores dos clientes e dos barbeiros
	*/
	int identificadores[maxInt(quantidadeClientes, NRO_BARBEIROS)];
	
	for (int i = 0; i < quantidadeClientes; i++) {
		identificadores[i] = i+1;					// Os valores de identificadores vão de 1 até quantidadeClientes
		pthread_create(&vetorClientes[i], 0, (void *) rotinaCliente, &identificadores[i]);
	}
	
	for (int i = 0; i < NRO_BARBEIROS; i++) {
		identificadores[i] = i+1;					// Os valores de identificadores vão de 1 até NRO_BARBEIROS
		pthread_create(&vetorBarbeiros[i], 0, (void *) rotinaBarbeiro, &identificadores[i]);
	}
	
	sleep(1);	// Fornece um tempo para que o primeiro cliente entre na loja e quebre a condição de saída abaixo
	
	while(nroClientes != 0);	// Aguarda todos os clientes serem atendidos
	
	/*
		Destrutores de semáforos, desalocando as memórias que eles ocupam
	*/
	sem_destroy(&cadeira);
    sem_destroy(&barbeiro);
    sem_destroy(&cliente);
    sem_destroy(&fazPagamento);
    sem_destroy(&caixa);
    sem_destroy(&recebeTroco);
	
	/*
		Destrutores de filas, desalocando as memórias que elas ocupam
	*/
	free(salaEspera);
	free(sofaEspera);
	
	return 0;
	
}