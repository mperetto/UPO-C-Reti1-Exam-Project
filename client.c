#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "list-lib/list.h"

/*
	Server riferimento prof
	IP:		130.192.9.131
	porta:	10010
*/

/**
 * Decodifica il messaggio inviato dal server, 
 * restituendo il tipo e eventualmente il messaggio personalizzato dal server.
 * 
 * @param		serverMsg					:	messaggio inviato dal server con delimitatori del protocollo.
 * @param		serverResponse		:	stringa personalizzata dal server (senza delimitatori del protocollo).
 * @return	tipologia di messaggio interpretato.
 * 			1:	OK START
 * 			2:	OK DATA
 * 			3:	OK STATS
 * 			4:	ERR DATA
 * 			5:	ERR STATS
 * 			6:	ERR SYNTAX
 * 			-1:	fomato sconosciuto
 * 
*/
int decodeServerMsg(char *serverMsg, char *serverResponse);

/**
 * Controlla se le due stringhe hanno i primi n caratteri uguali.
 * 
 * @param		str1	:	stringa 1 da confrontare.
 * @param		str2	:	stringa 2 da confrontare.
 * @return	1 se le stringhe sono uguali 0 altrimenti.
*/
int strEqual(char *str1, char *str2, int nChar);

int main(int argc, char *argv[])
{

	int simpleSocket = 0;
	int simplePort = 0;
	int returnStatus = 0;
	char buffer[512] = "";
	struct sockaddr_in simpleServer;

	if (3 != argc)
	{
		fprintf(stderr, "Errore inserimento parametri\n");
		fprintf(stderr, "Usare: %s <ip_server> <num_porta>\n", argv[0]);
		exit(1);
	}

	/* create a streaming socket      */
	simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (simpleSocket == -1)
	{
		fprintf(stderr, "Errore non posso creare il Socket\n");
		exit(1);
	}

	/* retrieve the port number for connecting */
	simplePort = atoi(argv[2]);

	/* setup the address structure */
	/* use the IP address sent as an argument for the server address  */
	bzero(&simpleServer, sizeof(simpleServer));
	simpleServer.sin_family = AF_INET;
	//inet_addr(argv[2], &simpleServer.sin_addr.s_addr);
	simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
	simpleServer.sin_port = htons(simplePort);

	/*  connect to the address and port with our socket  */
	returnStatus = connect(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

	if (returnStatus != 0)
	{
		fprintf(stderr, "Impossibile connettersi all'indirizzo\n");
		close(simpleSocket);
		exit(1);
	}

	/* get the message from the server   */
	memset(buffer, '\0', sizeof(buffer));
	returnStatus = read(simpleSocket, buffer, sizeof(buffer));

	char server_msg[512] = "";
	int messageType;
	int numDaInserire;
	int valore;

	if (returnStatus > 0)
	{
		int esci = 0;
		
		messageType = decodeServerMsg(buffer, server_msg);
		if(messageType == 1){// ricevuto OK START
			printf("\nMessaggio Server: %s", server_msg);

			upo_List_t lista = upo_list_init();
			printf("\n>>> Il programma permette di calcolare media e varianza su un gruppo di valori inseriti. <<<\n");
			printf(">>> I numeri verranno inviati a gruppi <<<\n");
			printf(">>> Per prima cosa l'utente dovra' selezionare quanti numeri vuole inviare. <<<\n");
			printf(">>> Una volta scelto il numero di valori l'utente dovra' inserire un numero alla volta <<<\n");
			printf(">>> Per terminare l'inserimento e calcolare i risultati sara' necessario rispondere 0 alla domanda 'Quanti numeri vuoi inserire?:' <<<\n\n");

			do{
				
				do{
					printf("Quanti numeri vuoi inserire?(inserire 0 per ottenere i risultati): ");
					scanf("%d", &numDaInserire);
				}while(numDaInserire < 0);

				int i;
				for(i = 0; i < numDaInserire; i++){
					printf("Inserire valore %d: ", i+1);
					scanf("%d", &valore);
					upo_list_add(lista, valore);
				}

				memset(buffer, 0, sizeof(buffer));
				sprintf(buffer, "%d", numDaInserire);

				if(numDaInserire > 0){
					while(upo_list_size(lista) > 0){
						char s[64] = "";
						sprintf(s, " %d", upo_list_remove_head(lista));
						strcat(buffer, s);
					}
					strcat(buffer, "\n");

					write(simpleSocket, buffer, sizeof(buffer));//invio numeri al server
					memset(buffer, 0, sizeof(buffer));
					returnStatus = read(simpleSocket, buffer, sizeof(buffer));//leggo risposta dal server
					messageType = decodeServerMsg(buffer, server_msg);
					int numRicDalServer = atoi(server_msg);
					
					if(messageType == 2 && numRicDalServer != numDaInserire){
						printf("Ops sembra che il server abbia ricevuto un numero diverso di valori da quelli inviati.\n");
						printf("Verra' interrotto l'inserimento dei valori e richiesto il calcolo dei risultati.\n");
						numDaInserire = 0;
					}
					else if(messageType > 3 && messageType <= 6){//Ricevuto messaggio di errore dal server
						printf("Ops il server ha avuto un errore e ha terminato la connessione\n");
						printf("Messaggio ricevuto dal server: %s\n", server_msg);
						close(simpleSocket);
						exit(1);
					}
					else if(messageType < 0){
						printf("Ops non sono in grado di interpretare la risposta del server.\n");
						printf("Verra' interrotta la connessione e chiuso il programma.\n");
						close(simpleSocket);
						exit(1);
					}
				}

			}while(numDaInserire != 0);
			
			strcat(buffer, "\n");
			write(simpleSocket, buffer, sizeof(buffer));//invio al server 0
			memset(buffer, 0, sizeof(buffer));
			returnStatus = read(simpleSocket, buffer, sizeof(buffer));

			messageType = decodeServerMsg(buffer, server_msg);
			if(messageType == 3){//Ricevuto OK STATS
				int numCampioni;
				float media;
				float varianza;
				sscanf(server_msg, "%d %f %f", &numCampioni, &media, &varianza);
				printf("\n\nIl server ha elaborato i valori\n");
				printf("Risultati Ottenuti:\n\n");
				printf("\tNumero di campioni: %d\n", numCampioni);
				printf("\tMedia calcolata: %f\n", media);
				printf("\tVarianza calcolata: %f\n\n", varianza);
			}
			else if(messageType > 3 && messageType <= 6){//Ricevuto messaggio di errore dal server
				printf("Il server ha avuto un errore e ha terminato la connessione\n");
				printf("Messaggio server: %s\n", server_msg);
			}

		}
		else{
			fprintf(stderr, "Errore di connessione con il server: %s\n", server_msg);
		}
		

	}
	else
	{
		fprintf(stderr, "Return Status = %d \n", returnStatus);
	}

	close(simpleSocket);
	return 0;
}

int decodeServerMsg(char *serverMsg, char *serverResponse){
	/*
		Tipi:
		-1 	:	formato sconosciuto
		1		:	OK START
		2		:	OK DATA
		3		:	OK STATS
		4		:	ERR DATA
		5		:	ERR STATS
		6		: ERR SYNTAX
	*/
	int msgType;

	char protocolMsg[6][14] = {
		"OK START ",
		"OK DATA ",
		"OK STATS ",
		"ERR DATA ",
		"ERR STATS ",
		"ERR SYNTAX "
	};

	if(strrchr(serverMsg, '\n') == NULL) return -1;

	if(strstr(serverMsg, protocolMsg[0]) != NULL){// OK START
		if(strEqual(serverMsg, protocolMsg[0], strlen(protocolMsg[0]))){
			msgType = 1;
			//serverResponse = (serverMsg + strlen(protocolMsg[0]));
			strcpy(serverResponse, (serverMsg + strlen(protocolMsg[0])));
			//printf("%s\n", serverResponse);
		}
	}
	else if(strstr(serverMsg, protocolMsg[1]) != NULL){// OK DATA
		if(strEqual(serverMsg, protocolMsg[1], strlen(protocolMsg[1]))){
			msgType = 2;
			strcpy(serverResponse, (serverMsg + strlen(protocolMsg[1])));
			//printf("%s\n", serverResponse);
		}
	}
	else if(strstr(serverMsg, protocolMsg[2]) != NULL){// OK STATS
		if(strEqual(serverMsg, protocolMsg[2], strlen(protocolMsg[2]))){
			msgType = 3;
			strcpy(serverResponse, (serverMsg + strlen(protocolMsg[2])));
			//printf("%s\n", serverResponse);
		}
	}
	else if(strstr(serverMsg, protocolMsg[3]) != NULL){// ERR DATA
		if(strEqual(serverMsg, protocolMsg[3], strlen(protocolMsg[3]))){
			msgType = 4;
			strcpy(serverResponse, (serverMsg + strlen(protocolMsg[3])));
			//printf("%s\n", serverResponse);
		}
	}
	else if(strstr(serverMsg, protocolMsg[4]) != NULL){// ERR STATS
		if(strEqual(serverMsg, protocolMsg[4], strlen(protocolMsg[4]))){
			msgType = 5;
			strcpy(serverResponse, (serverMsg + strlen(protocolMsg[4])));
			//printf("%s\n", serverResponse);
		}
	}
	else if(strstr(serverMsg, protocolMsg[5]) != NULL){// ERR SYNTAX
		if(strEqual(serverMsg, protocolMsg[5], strlen(protocolMsg[5]))){
			msgType = 6;
			strcpy(serverResponse, (serverMsg + strlen(protocolMsg[5])));
			//printf("%s\n", serverResponse);
		}
	}
	else{
		msgType = -1;
	}

	return msgType;
}

int strEqual(char *str1, char *str2, int nChar){

	int i;
	for(i = 0; i < nChar; i++){
		if(str1[i] != str2[i]) return 0;
	}

	return 1;
}