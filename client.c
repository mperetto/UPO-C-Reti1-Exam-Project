#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

/**
 * Decodifica il messaggio inviato dal server, 
 * restituendo il tipo e eventualmente il messaggio personalizzato dal server.
 * 
 * @param		serverMsg					:	messaggio inviato dal server con delimitatori del protocollo.
 * @param		serverResponse		:	stringa personalizzata dal server (senza delimitatori del protocollo).
 * @return	tipologia di messaggio interpretato.
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

	if (returnStatus > 0)
	{
		int esci = 0;
		{
			char *str_OK = "OK START ";
			int i;
			for(i = 0; i < strlen(str_OK); i++){
				if(buffer[i] != str_OK[i]){
					esci = 1;
					break;
				}
			}
			for(; i < strlen(buffer); i++){
				char buff_serv[2];
				buff_serv[1] = '\0';
				buff_serv[0] = buffer[i];
				strcat(server_msg, buff_serv);
			}
			printf("%sfin\n", server_msg);
			printf("%d\n", decodeServerMsg(buffer, ""));
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
			serverResponse = (serverMsg + strlen(protocolMsg[0]));
			printf("%s\n", serverResponse);
		}
	}
	else if(strstr(serverMsg, protocolMsg[1]) != NULL){// OK DATA
		if(strEqual(serverMsg, "OK DATA ", strlen(protocolMsg[1]))){
			msgType = 2;
			serverResponse = (serverMsg + strlen(protocolMsg[1]));
			printf("%s\n", serverResponse);
		}
	}
	else if(strstr(serverMsg, protocolMsg[2]) != NULL){// OK STATS
		msgType = 3;
	}
	else if(strstr(serverMsg, protocolMsg[3]) != NULL){// ERR DATA
		msgType = 4;
	}
	else if(strstr(serverMsg, protocolMsg[4]) != NULL){// ERR STATS
		msgType = 5;
	}
	else if(strstr(serverMsg, protocolMsg[6]) != NULL){// ERR SYNTAX
		msgType = 6;
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