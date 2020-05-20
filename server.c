#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

/**
 *	controlla se il messaggio sia sintatticamente corretto
 *
 * 	@param message			: messaggio da controllare.
 * 	@param errorMessage		: messaggio di errore da restituire
 * 
 * 	@return					:	0 se è presente un'errore di sintassi 1 altrimenti
*/
int checkMessageSyntax(const char *message, char *errorMessage);

typedef struct valori{
	int totValRicevuti;
	int sommaVal;
	int sommaValQuad;
	float media;
	float varianza;
}valori;

int main(int argc, char const *argv[])
{
	int simpleSocket = 0;
	int simplePort = 0;
	int returnStatus = 0;
	struct sockaddr_in simpleServer;
	char buffer[512]; // <Esito> <Tipo> <Contenuto>

	const char ERR_MESSAGES[3][128] = {
			"ERR SYNTAX Formato messaggio errato, (manca ritorno a capo)\n",
			"ERR DATA Numero dati fornito non corrisponde con dati trasmessi\n",
			"ERR STATS Non posso calcolare la varianza\n"};

	if (2 != argc)
	{

		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
		exit(1);
	}

	simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (simpleSocket == -1)
	{

		fprintf(stderr, "Could not create a socket!\n");
		exit(1);
	}
	else
	{
		fprintf(stderr, "Socket created!\n");
	}

	/* retrieve the port number for listening */
	simplePort = atoi(argv[1]);

	/* setup the address structure */
	/* use INADDR_ANY to bind to all local addresses  */
	memset(&simpleServer, '\0', sizeof(simpleServer));
	simpleServer.sin_family = AF_INET;
	simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
	simpleServer.sin_port = htons(simplePort);

	/*  bind to the address and port with our socket  */
	returnStatus = bind(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

	if (returnStatus == 0)
	{
		fprintf(stderr, "Bind completed!\n");
	}
	else
	{
		fprintf(stderr, "Could not bind to address!\n");
		close(simpleSocket);
		exit(1);
	}

	/* lets listen on the socket for connections      */
	returnStatus = listen(simpleSocket, 5);

	if (returnStatus == -1)
	{
		fprintf(stderr, "Cannot listen on socket!\n");
		close(simpleSocket);
		exit(1);
	}

	while (1)
	{

		struct sockaddr_in clientName = {0};
		int simpleChildSocket = 0;
		int clientNameLength = sizeof(clientName);

		/* wait here */

		simpleChildSocket = accept(simpleSocket, (struct sockaddr *)&clientName, &clientNameLength);

		if (simpleChildSocket == -1)
		{

			fprintf(stderr, "Cannot accept connections!\n");
			close(simpleSocket);
			exit(1);
		}

		memset(buffer, '\0', sizeof(buffer));
		strcpy(buffer, "OK START Connessione attiva, Attendo i dati\n");
		write(simpleChildSocket, buffer, strlen(buffer));
		
		//inizializzo valori
		valori valClient;
		valClient.media = 0;
		valClient.sommaVal = 0;
		valClient.sommaValQuad = 0;
		valClient.totValRicevuti = 0;
		valClient.varianza = 0;

		int receivedNumber;

		int esci = 0;//Vale 1 quando si verifica un errore in seguito la conn viene chiusa
		while (!esci)
		{
			memset(buffer, '\0', sizeof(buffer));
			returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));

			char *chrFound;
			chrFound = strrchr(buffer, '\n');

			if (chrFound == NULL)
			{
				memset(buffer, '\0', sizeof(buffer));
				strcpy(buffer, ERR_MESSAGES[0]);
				write(simpleChildSocket, buffer, strlen(buffer));
				esci = 1;
			}
			else
			{
				*chrFound = '\0';

				char errorMessage[512];
				if(!checkMessageSyntax(buffer, errorMessage)){
					memset(buffer, '\0', sizeof(buffer));
					strcpy(buffer, errorMessage);
					write(simpleChildSocket, buffer, strlen(buffer));
					esci = 1;
				}

				if (!esci)
				{
					char *delim = " ";
					char *ptr = strtok(buffer, delim);

					if(ptr != NULL){
						int totValues = atoi(ptr);
						//printf("%s %d\n", ptr, totValues);
						ptr = strtok(NULL, delim);

						int ValuesCounter = 0;
						while(ptr != NULL){
							ValuesCounter++;
							//printf("Value: %s, Tot: %d\n", ptr, ValuesCounter);
							receivedNumber = atoi(ptr);
							valClient.sommaVal = valClient.sommaVal + receivedNumber;
							valClient.sommaValQuad = valClient.sommaValQuad + (receivedNumber*receivedNumber);
							valClient.totValRicevuti++;
							printf("somma: %d\nsommaQuad: %d\ntotRic: %d\n", valClient.sommaVal, valClient.sommaValQuad, valClient.totValRicevuti);
							ptr = strtok(NULL, delim);
						}
						
						if(ValuesCounter != totValues){
							if(ValuesCounter < totValues){
								memset(buffer, '\0', sizeof(buffer));
								strcpy(buffer, "ERR DATA Errore valori trasmessi inferiori a valori dichiarati\n");
								write(simpleChildSocket, buffer, strlen(buffer));
							}
							else if(ValuesCounter > totValues){
								memset(buffer, '\0', sizeof(buffer));
								strcpy(buffer, "ERR DATA Errore valori trasmessi superiori a valori dichiarati\n");
								write(simpleChildSocket, buffer, strlen(buffer));
							}
							esci = 1;
						}
						else{
							if(totValues != 0){
								memset(buffer, '\0', sizeof(buffer));
								sprintf(buffer, "OK DATA %d\n", totValues);
								write(simpleChildSocket, buffer, strlen(buffer));
							}
							else{
								memset(buffer, '\0', sizeof(buffer));
								sprintf(buffer, "OK STATS <tot val elab> <media> <varianza>\n");
								write(simpleChildSocket, buffer, strlen(buffer));
							}
						
						}
					}
				}
			}
		}

		close(simpleChildSocket);
	}

	close(simpleSocket);
	return 0;
}

int checkMessageSyntax(const char *message, char *errorMessage) {
	int i;
	char chrPrev = 'a';
	for (i = 0; i < strlen(message); i++)
	{
		if (chrPrev == message[i] && message[i] == ' ')
		{
			memset(errorMessage, '\0', sizeof(errorMessage));
			strcpy(errorMessage, "ERR SYNTAX Errore formato messaggio errato\n");
			
			return 0;
		}
		else if(message[i] != ' '){
			if(!isdigit(message[i])){
				memset(errorMessage, '\0', sizeof(errorMessage));
				strcpy(errorMessage, "ERR DATA Errore puoi inviare solo numeri\n");
				return 0;
			}
		}
		
		chrPrev = message[i];
	}
	return 1;
}