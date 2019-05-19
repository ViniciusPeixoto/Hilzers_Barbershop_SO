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
#include <string.h>

#define LOTACAO_MAXIMA 20		// Quantidade máxima de clientes
#define LUGARES_SOFA 4			// Quantidade de lugares no sofá
#define LUGARES_SALA 16			// Quantidade de lugares na sala
#define NRO_CADEIRAS 3			// Quantidade de cadeiras de barbeiro
#define NRO_BARBEIROS 3			// Quantidade de barbeiros trabalhando
#define NRO_CAIXA 1				// Quantidade de caixas registradoras

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
	Protótipos de funções e métodos usados nesse programa
*/
fila *criaFila(int p_tamanhoFila);
void esperaSala(fila *p_sala, int p_cliente);
void esperaSofa(fila *p_sofa, int p_cliente);
void imprimeTexto(fila *p_acesso);
void avancaFila(fila *p_fila);
int maxInt(int p_valor1, int p_valor2);
void imprimeFrase(char *p_frase, int p_valor);
void *rotinaCliente(void *p_arg);
void *rotinaBarbeiro(void *p_arg);


/*
	A barbearia possui duas condições de espera antes do cliente ser atendido:
	Um sofá, para clientes que estão esperando e serão atendidos em breve, e
	uma sala de espera, para clientes que esperarão mais tempo, em pé.
*/

fila *sofaEspera, *salaEspera, *acessoImprime;

/*
	Os recursos do sistema também são representados por semáforos, uma vez que eles estão disponíveis
	somente para um único barbeiro/cliente.
*/

sem_t barbeiro, corte, cadeira, caixa;			// Ações da barbearia
sem_t cliente, fazPagamento, recebeTroco;		// Ações dos clientes
sem_t imprime, vazio;									// Ações do sistema

/*
	Controle ddo fluxo dentro da barbearia.
*/
int nroClientes = 0;
int espera = 0;

int barbeiroOcioso = 0;

/*
	Frases que serão escritas
*/
char *frase;

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
	imprimeTexto(acessoImprime);
	sem_wait(&imprime);
	frase = "O cliente %d entrou na sala de espera.\n";
	imprimeFrase(frase, p_cliente);
	avancaFila(acessoImprime);
	sem_post(&imprime);
    sem_post(&(p_sala->cabeca));									// Se há espaço na sala, há um cliente para sentar no sofá
}

void esperaSofa(fila *p_sofa, int p_cliente){
    sem_wait(&(p_sofa->cauda));										// Verifica se há espaço no sofá
	imprimeTexto(acessoImprime);
	sem_wait(&imprime);
	frase = "\tO cliente %d sentou no sofa.\n";
	imprimeFrase(frase, p_cliente);
	avancaFila(acessoImprime);
	sem_post(&imprime);
    sem_post(&(p_sofa->cabeca));									// Se há espaço no sofá, há um cliente para ser o próximo a
																	// ser atendido pelo barbeiro
}

void imprimeTexto(fila *p_acesso){
	sem_wait(&(p_acesso->cauda));
	sem_post(&(p_acesso->cabeca));
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
		imprimeTexto(acessoImprime);
		sem_wait(&imprime);
		frase = "A barbearia esta cheia. O cliente %d vai embora.\n";
		imprimeFrase(frase, v_clienteAtual);
		avancaFila(acessoImprime);
		sem_post(&imprime);
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
	sem_wait(&vazio);
	espera++;
	sem_post(&vazio);
	
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
	sem_wait(&vazio);
	espera--;
	sem_post(&vazio);
	
	imprimeTexto(acessoImprime);
	sem_wait(&imprime);
	frase = "\t\tO cliente %d esta sendo atendido agora.\n";
	imprimeFrase(frase, v_clienteAtual);
	avancaFila(acessoImprime);
	sem_post(&imprime);
	
	imprimeTexto(acessoImprime);
	sem_wait(&imprime);
	frase = "\t\tO cliente %d esta sentado na cadeira do barbeiro.\n";
	imprimeFrase(frase, v_clienteAtual);
	avancaFila(acessoImprime);
	sem_post(&imprime);
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
	imprimeTexto(acessoImprime);
	sem_wait(&imprime);
	frase = "\t\tO cliente %d esta tendo seu cabelo cortado.\n";
	imprimeFrase(frase, v_clienteAtual);
	avancaFila(acessoImprime);
	sem_post(&imprime);
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
	imprimeTexto(acessoImprime);
	sem_wait(&imprime);
	frase = "\t\tO cliente %d quer pagar pelo servico.\n";
	imprimeFrase(frase, v_clienteAtual);
	avancaFila(acessoImprime);
	sem_post(&imprime);
	
	/*
		Depois de pagar, ele aguarda o barbeiro usar a caixa registradora para guardar o pagamento e pegar o troco.
	*/
	sem_wait(&recebeTroco);
	imprimeTexto(acessoImprime);
	sem_wait(&imprime);
	frase = "\t\tO cliente %d recebeu seu troco.\n";
	imprimeFrase(frase, v_clienteAtual);
	avancaFila(acessoImprime);
	sem_post(&imprime);
	
	/*
		Finalmente o cliente desocupa a cadeira e vai embora da barbearia.
	*/
	sem_post(&cadeira);
	imprimeTexto(acessoImprime);
	sem_wait(&imprime);
	frase = "\t\tO cliente %d esta indo embora.\n";
	imprimeFrase(frase, v_clienteAtual);
	avancaFila(acessoImprime);
	sem_post(&imprime);
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

		/*
			Se não há clientes no sofá, o barbeiro dorme
		*/
		if(espera == 0){
			imprimeTexto(acessoImprime);
			sem_wait(&imprime);
			frase = "\t\t\tNao ha clientes. O barbeiro %d vai dormir.\n";
			imprimeFrase(frase, v_barbeiroAtual);
			avancaFila(acessoImprime);
			sem_post(&imprime);
		}
		sem_wait(&vazio);
		barbeiroOcioso++;
		sem_post(&vazio);

		sem_wait(&cliente);			// Há um cliente para ser atendido?
		
		sem_wait(&vazio);
		barbeiroOcioso--;
		sem_post(&vazio);

		imprimeTexto(acessoImprime);
		sem_wait(&imprime);
		frase = "\t\t\tO barbeiro %d está atendendo.\n";
		imprimeFrase(frase, v_barbeiroAtual);
		avancaFila(acessoImprime);
		sem_post(&imprime);
		/*
			O barbeiro iniciará o corte em quem está na cadeira
		*/
		sem_wait(&corte);
		imprimeTexto(acessoImprime);
		sem_wait(&imprime);
		frase = "\t\t\tO barbeiro %d está cortando cabelo.\n";
		imprimeFrase(frase, v_barbeiroAtual);
		avancaFila(acessoImprime);
		sem_post(&imprime);
		sleep(10);
		
		/*
			O barbeiro que terminar o corte está livre para receber pagamentos
		*/
		sem_wait(&barbeiro);			// Há um barbeiro para receber o pagamento?
		sem_wait(&fazPagamento);		// Há clientes que fizeram pagamentos?
		imprimeTexto(acessoImprime);
		sem_wait(&imprime);
		frase = "\t\t\tO barbeiro %d está recebendo pagamento.\n";
		imprimeFrase(frase, v_barbeiroAtual);
		avancaFila(acessoImprime);
		sem_post(&imprime);
		
		/*
			O barbeiro que estiver disponível irá receber pelo corte realizado por um cliente.
			Como um barbeiro acabou de ficar disponível após o serviço prestado, é provável que
			ele atenda o pagamento, mas não é necessáriamente sempre ele.
			Somente um barbeiro pode usar a caixa registradora por vez
		*/
		sem_wait(&caixa);		// Solicita acesso à caixa registradora
		imprimeTexto(acessoImprime);
		sem_wait(&imprime);
		frase = "\t\t\tO barbeiro %d está usando a caixa registradora.\n";
		imprimeFrase(frase, v_barbeiroAtual);
		avancaFila(acessoImprime);
		sem_post(&imprime);
		sleep(5);				// Simulação de que as ações levam algum tempo para serem feitas
		sem_post(&caixa);		// Libera o uso da caixa registradora
		
		/*
			O barbeiro retorna da caixa registradora e vai passar o troco para o cliente
		*/
		imprimeTexto(acessoImprime);
		sem_wait(&imprime);
		frase = "\t\t\tO barbeiro %d está passando troco.\n";
		imprimeFrase(frase, v_barbeiroAtual);
		avancaFila(acessoImprime);
		sem_post(&imprime);
		sem_post(&recebeTroco);
		sleep(5);
		
		/*
			Ao final, o barbeiro está disponível para receber outro cliente.
		*/
		imprimeTexto(acessoImprime);
		sem_wait(&imprime);
		frase = "\t\t\tO barbeiro %d esta livre.\n";
		imprimeFrase(frase, v_barbeiroAtual);
		avancaFila(acessoImprime);
		sem_post(&imprime);
		
	}
}

/*
	Função que retorna o maior valor de dois inteiros
*/
int maxInt(int p_valor1, int p_valor2){
	if (p_valor1 > p_valor2) {
		return p_valor1;
	}

	return p_valor2;
}

/*
	Método que imprime uma frase
*/
void imprimeFrase(char *p_frase, int p_valor){
	printf(p_frase, p_valor);
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
	sem_init(&cadeira,0,NRO_CADEIRAS);
    sem_init(&barbeiro,0,0);
    sem_init(&cliente,0,0);
    sem_init(&fazPagamento,0,0);
    sem_init(&caixa,0,NRO_CAIXA);
    sem_init(&recebeTroco,0,0);
	sem_init(&imprime,0,1);
	sem_init(&vazio, 0,1);
	
	/*
		As filas de semáforos para a sala e o sofá são criadas
		Seus tamanhos dependem do número de LUGARES_SALA e LUGARES_SOFA
		Os clientes só avançam nessa fila quando o primeiro da fila sair, liberando um novo espaço no final da fila.
		A fila de acesso à impressão é criada com o tamanho máximo de acessos simultâneos à impressão, isto é, se todos
		os clientes e todos os barbeiros pedirem para imprimir algo ao mesmo tempo.
	*/
	salaEspera = criaFila(LUGARES_SALA);
	sofaEspera = criaFila(LUGARES_SOFA);
	acessoImprime = criaFila(quantidadeClientes+NRO_BARBEIROS);
	
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
	while(nroClientes != 0 || barbeiroOcioso != NRO_BARBEIROS);		// Aguarda todos os clientes serem atendidos
	/*
		Destrutores de semáforos, desalocando as memórias que eles ocupam
	*/
	sem_destroy(&cadeira);
    sem_destroy(&barbeiro);
    sem_destroy(&cliente);
    sem_destroy(&fazPagamento);
    sem_destroy(&caixa);
    sem_destroy(&recebeTroco);
	sem_destroy(&imprime);
	sem_destroy(&vazio);

	/*
		Destrutores de filas, desalocando as memórias que elas ocupam
	*/
	free(salaEspera);
	free(sofaEspera);
	
	return 0;
	
}
