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
	int serverSocket = 0;
	int socketPort = 0;
	int returnStatus = 0;
	struct sockaddr_in sockAddrServer;
	char buffer[512];

	if (argc != 2)
	{
		fprintf(stderr, "Errore inserimento parametri\n");
		fprintf(stderr, "Usare: %s <numero_porta>\n", argv[0]);
		exit(1);
	}

	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (serverSocket == -1)
	{
		fprintf(stderr, "Impossibile creare il Socket\n");
		exit(1);
	}
	else
	{
		fprintf(stderr, "Socket creato correttamente\n");
	}

	/* retrieve the port number for listening */
	socketPort = atoi(argv[1]);

	/* setup the address structure */
	/* use INADDR_ANY to bind to all local addresses  */
	memset(&sockAddrServer, '\0', sizeof(sockAddrServer));
	sockAddrServer.sin_family = AF_INET;
	sockAddrServer.sin_addr.s_addr = htonl(INADDR_ANY);
	sockAddrServer.sin_port = htons(socketPort);

	/*  bind to the address and port with our socket  */
	returnStatus = bind(serverSocket, (struct sockaddr *)&sockAddrServer, sizeof(sockAddrServer));

	if (returnStatus == 0)
	{
		fprintf(stderr, "Bind completo\n");
	}
	else
	{
		fprintf(stderr, "Impossibile eseguire il Bind sull'indirizzo\n");
		close(serverSocket);
		exit(1);
	}

	/* lets listen on the socket for connections      */
	returnStatus = listen(serverSocket, 5);

	if (returnStatus == -1)
	{
		fprintf(stderr, "Impossibile ascoltare sul socket\n");
		close(serverSocket);
		exit(1);
	}

	while (1)
	{

		struct sockaddr_in clientName = {0};
		int simpleChildSocket = 0;
		int clientNameLength = sizeof(clientName);

		/* Attesa connessioni */

		simpleChildSocket = accept(serverSocket, (struct sockaddr *)&clientName, &clientNameLength);

		if (simpleChildSocket == -1)
		{

			fprintf(stderr, "Impossibile accettare connessioni\n");
			close(serverSocket);
			exit(1);
		}

		memset(buffer, '\0', sizeof(buffer));
		strcpy(buffer, "OK START Connessione attiva, Attendo i dati\n");
		write(simpleChildSocket, buffer, strlen(buffer));
		
		/*	inizializzo i valori	*/
		valori valClient;
		valClient.media = 0;
		valClient.sommaVal = 0;
		valClient.sommaValQuad = 0;
		valClient.totValRicevuti = 0;
		valClient.varianza = 0;

		int receivedNumber;

		int esci = 0;//Vale 1 quando si verifica un errore o viene richiesto il calcolo delle statistiche in seguito la conn viene chiusa
		while (!esci)
		{
			memset(buffer, '\0', sizeof(buffer));
			returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));

			char *chrFound;
			chrFound = strrchr(buffer, '\n');

			if (chrFound == NULL)
			{
				memset(buffer, '\0', sizeof(buffer));
				strcpy(buffer, "ERR SYNTAX Formato messaggio errato.\n");
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
				else
				{
					char *delim = " ";
					char *ptr = strtok(buffer, delim);

					if(ptr != NULL){
						int totValues = atoi(ptr);
						ptr = strtok(NULL, delim);

						int ValuesCounter = 0;
						while(ptr != NULL){
							ValuesCounter++;
							receivedNumber = atoi(ptr);
							valClient.sommaVal = valClient.sommaVal + receivedNumber;
							valClient.sommaValQuad = valClient.sommaValQuad + (receivedNumber*receivedNumber);
							valClient.totValRicevuti++;
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
								if(valClient.totValRicevuti < 2){
									memset(buffer, '\0', sizeof(buffer));
									sprintf(buffer, "ERR STATS Non posso calcolare la varianza con %d dato ricevuto\n", valClient.totValRicevuti);
									write(simpleChildSocket, buffer, strlen(buffer));
									esci = 1;
								}
								else{
									memset(buffer, '\0', sizeof(buffer));
									valClient.media = (float)valClient.sommaVal / (float)valClient.totValRicevuti;
									valClient.varianza = ((float)valClient.sommaValQuad - ((float)valClient.totValRicevuti)*(valClient.media*valClient.media))/((float)valClient.totValRicevuti - 1);
									sprintf(buffer, "OK STATS %d %f %f\n", valClient.totValRicevuti, valClient.media, valClient.varianza);
									write(simpleChildSocket, buffer, strlen(buffer));
									esci = 1;
								}
							}
						
						}
					}
				}
			}
		}

		close(simpleChildSocket);
	}

	close(serverSocket);
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