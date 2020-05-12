#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <time.h>

int main(int argc, char const *argv[])
{
    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;
    char buffer[512];// <Esito> <Tipo> <Contenuto>

    const char ERR_MESSAGES[3][128] = {
        "ERR SYNTAX Formato messaggio errato, (manca ritorno a capo)\n",
        "ERR DATA Numero dati fornito non corrisponde con dati trasmessi\n",
        "ERR STATS Non posso calcolare la varianza"
    };

    if (2 != argc) {

        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);

    }

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1) {

        fprintf(stderr, "Could not create a socket!\n");
        exit(1);

    }
    else {
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
    returnStatus = bind(simpleSocket,(struct sockaddr *)&simpleServer,sizeof(simpleServer));

    if (returnStatus == 0) {
	    fprintf(stderr, "Bind completed!\n");
    }
    else {
        fprintf(stderr, "Could not bind to address!\n");
        close(simpleSocket);
        exit(1);
    }

    /* lets listen on the socket for connections      */
    returnStatus = listen(simpleSocket, 5);

    if (returnStatus == -1) {
        fprintf(stderr, "Cannot listen on socket!\n");
	    close(simpleSocket);
        exit(1);
    }

    while (1){

        struct sockaddr_in clientName = { 0 };
        int simpleChildSocket = 0;
        int clientNameLength = sizeof(clientName);

        /* wait here */

        simpleChildSocket = accept(simpleSocket,(struct sockaddr *)&clientName, &clientNameLength);

        if (simpleChildSocket == -1) {

            fprintf(stderr, "Cannot accept connections!\n");
            close(simpleSocket);
            exit(1);

        }

        memset(buffer, '\0', sizeof(buffer));
        strcpy(buffer, "OK START Connessione attiva, Attendo i dati\n");
        write(simpleChildSocket, buffer, strlen(buffer));

        memset(buffer, '\0', sizeof(buffer));
        returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));

        char *chrFound;
        chrFound = strrchr(buffer, '\n');

        if(chrFound == NULL){
            memset(buffer, '\0', sizeof(buffer));
            strcpy(buffer, ERR_MESSAGES[0]);
            write(simpleChildSocket, buffer, strlen(buffer));
            close(simpleChildSocket);
        }
        else{
            *chrFound = '\0';
            
        }

        //write(simpleChildSocket, MESSAGE, strlen(MESSAGE));
        //returnStatus = read(simpleSocket, buffer, sizeof(buffer));
        close(simpleChildSocket);

    }

    close(simpleSocket);
    return 0;
}