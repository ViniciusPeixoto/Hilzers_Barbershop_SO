#################################################################################################
#                                                                                               #
# %%%% % %%%% %%% %%%% %   % %%%% %%%%   %%%% %%% %%%% %%%% %%%% %%%% % %%%% %   % %%%% % %%%%% #
# %    % %     %  %    %% %% %  % %      %  % % % %    %  % %  % %    % %  % %%  % %  % % %     #
# %%%% % %%%%  %  %%%% % % % %%%% %%%%   %  % %%% %%%% %%%% %%%% %    % %  % % % % %%%% % %%%%% #
#    % %    %  %  %    %   % %  %    %   %  % %   %    % %  %  % %    % %  % %  %% %  % %     % #
# %%%% % %%%%  %  %%%% %   % %  % %%%%   %%%% %   %%%% %  % %  % %%%% % %%%% %   % %  % % %%%%% #
#                                                                                               #
#################################################################################################

Este arquivo contém as instruções necessárias para a execução do código relativo ao Problema da
Barbearia de Hilzer.
-------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------

1. Requisitos

Para a execução você precisará:

	- Um computador ou máquina virtual com sistema operacional Unix/Linux (POSIX)
	- GNU Compiler Collection (GCC) instalado no sistema operacional
	- Bibliotecas semaphore.h, pthread.h, unistd.h
	- O arquivo do código-fonte (hilzer.c)
	
2. Compilando o código

Para compilar o código, abra o terminal na mesma pasta em que o arquivo está localizado. Você
pode fazer isso tanto usando o botão direito do mouse ou o comando cd.
Com o terminal aberto no diretório em que o código-fonte está localizado, entre com o seguinte
comando

			gcc  hilze.c -o barbershop -lpthread

Isso irá gerar o objeto "barbershop" no mesmo diretório.

3. Executando o algoritmo

Com o objeto montado e o terminal ainda aberto no mesmo diretótio, digite o comando

			./barbershop <número de clientes>

Subistitua <número de clientes> pela quantidade de clientes que você deseja colocar na simulação.
O número deve ser inteiro e maior que 0.
Caso não seja passado argumentos, será exibido um pequeno guia de ajuda a como usar o programa.

4. Resultados esperados

O programa irá imprimir uma série de frases que indicam o andamento do algoritmo.
Atente-se para a indentação. Quanto mais a direita a frase for impressa, mais interna à barbearia
é a ação descrita