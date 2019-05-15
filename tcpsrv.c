/***************************************/
/*              TCP server             */
/***************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFSIZE 1024                      // buffer size
#define PORT_NO 2001              // port number
#define error(a,b) fprintf(stderr, a, b)  // error 'function'

int main(int argc, char *argv[] ) { 	// arg count, arg vector

    /* Declarations */
    int fd;	        	           		// socket endpt
    int player1,player2;                        	// socket endpt
    int flags;                      	// rcv flags
    struct sockaddr_in server;      	// socket name (addr) of server
    struct sockaddr_in client;	     	// socket name of client
    int server_size;                	// length of the socket addr. server
    int client_size;                	// length of the socket addr. client
    int bytes1,bytes2;		           					// length of buffer
    int rcvsize1, rcvsize2;                    	// received bytes
    int trnmsize1, trnmsize2;                   	// transmitted bytes
    int err;                        	// error code
    char on;                        	//
    char buffer1[BUFSIZE+1],buffer2[BUFSIZE+1],szamok[5];	     		// datagram dat buffer area

    /* Initialization */
    on                     = 1;
    flags                  = 0;
    bytes1=bytes2                  = BUFSIZE;
    server_size            = sizeof server;
    client_size            = sizeof client;
    server.sin_family      = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; // konstans ami a saját címemre mutat
    server.sin_port        = htons(PORT_NO);  //
    /* Creating socket */
    fd = socket(AF_INET, SOCK_STREAM, 0 );
    if (fd < 0) {
        error("%s: Socket creation error\n",argv[0]);
        exit(1);
    }

    /* Setting socket options */
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof on);
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof on);

    /* Binding socket */
    err = bind(fd, (struct sockaddr *) &server, server_size);
    if (err < 0) {
        error("%s: Cannot bind to the socket\n",argv[0]);  //kiiratás
        exit(2);
    }
    int korok = atoi(argv[1]);
    /* Listening */
    err = listen(fd, 10);
    if (err < 0) {
        error("%s: Cannot listen to the socket\n",argv[0]);
        exit(3);
    }
    printf("Várakozás a playerekre:\n");
    /* Accepting connection request */
    player1 = accept(fd, (struct sockaddr *) &client, &client_size); //kliens csatlakozási szándékának elfogadása
    printf("Player1 joined!\n");
    if (player1 < 0 ) {
        error("%s: Cannot accept on socket\n",argv[0]);
        exit(4);
    }
    strcpy(buffer1,"Te vagy a Player1");
    bytes1=strlen(buffer1)+1;
    send(player1,buffer1,bytes1,flags);
    player2 = accept(fd, (struct sockaddr *) &client, &client_size);
    printf("Player2 joined!\n");

    if (player2 < 0 ) {
        error("%s: Cannot accept on socket\n",argv[0]);
        exit(4);
    }
    strcpy(buffer2,"Te vagy a Player2");
    bytes2=strlen(buffer2)+1;
    send(player2,buffer2,bytes2,flags);


    bytes1 = bytes2 = strlen(argv[1])+1;
    send(player1,argv[1],bytes1,flags);
    send(player2,argv[1],bytes2,flags);
    /* Receiving data from the client */
    int i=0, player1_gy=0,player2_gy=0;
    int dontetlen=1;
    int player1_valasz, player2_valasz;
    char allas[BUFSIZE+1], player1nev[20]="Player1: ", player2nev[20]=" Player2: ";
    bytes1= bytes2 = 256;
    while(i<korok) {
        memset(buffer1,0,BUFSIZE+1);
        memset(buffer2,0,BUFSIZE+1);
        rcvsize1 = recv( player1, buffer1, bytes1, flags );
        rcvsize2 = recv( player2, buffer2, bytes2, flags );// max fogadható méret=bytes
        if (rcvsize1 < 0 || rcvsize2 < 0) {
            error("%s: Cannot receive from the socket\n",argv[0]);
            exit(5);
        }

        /* User interface */
        printf("Player1 Message: %s \n", buffer1);
        printf("Player2 Message: %s \n", buffer2);
        dontetlen = strcmp(buffer1,buffer2);
        if(dontetlen == 0)
        {   player1_gy++;
            player2_gy++;
        }
        if(!strcmp(buffer1,"feladom")||!strcmp(buffer2,"feladom"))
            goto feladom;
        if(!strcmp(buffer1,"ko"))
            player1_valasz=0;
        else if(!strcmp(buffer1,"papir"))
            player1_valasz=1;
        else
            player1_valasz=2;

        if(!strcmp(buffer2,"ko"))
            player2_valasz=0;
        else if(!strcmp(buffer2,"papir"))
            player2_valasz=1;
        else
            player2_valasz=2;
        if(player1_valasz==0 && player2_valasz==2)
            player1_gy++;
        else if(player2_valasz==0 && player1_valasz==2)
            player2_gy++;

        if(player1_valasz==1 && player2_valasz==0)
            player1_gy++;
        if(player2_valasz==1 && player1_valasz==0)
            player2_gy++;
        
        if(player1_valasz==2 && player2_valasz==1 )
        player1_gy++;
        else
            if(player2_valasz==2 && player1_valasz==1 )
                player2_gy++;
        /* Sending acknowledgement to the client */
        sprintf(buffer1,"Transmit is OK.");
        sprintf(buffer2,"Transmit is OK.");//sztringbe ír ele
        bytes1 = strlen(buffer1) + 1;
        bytes2 = strlen(buffer2) + 1;
        trnmsize1 = send(player1, buffer1, bytes1, flags);
        trnmsize2 = send(player2, buffer2, bytes2, flags);	//elküldött bájtok db száma
        if (trnmsize1 < 0 || trnmsize2 < 0) {
            error("%s: Cannot send data to the client.\n",argv[0]);
            exit(6);
        }
        /* User interface */
        //printf ("Acknowledgement has been sent to the client.\n");
        i++;
        
        sprintf(allas,"Az állás: ");
        strcat(allas,player1nev);
        sprintf(szamok,"%d",player1_gy);
        strcat(allas,szamok);
        strcat(allas,player2nev);
        sprintf(szamok,"%d",player2_gy);
        strcat(allas,szamok);
        
        bytes1 = strlen(allas)+1;
        send(player1,allas,bytes1,flags);
        send(player2,allas,bytes1,flags);
    }

    if(player1_gy>player2_gy)
    {strcpy(buffer1,"Gratulálok, te nyertél!");
        strcpy(buffer2,"Sajnos vesztettél!");
        bytes1=strlen(buffer1)+1;
        bytes2=strlen(buffer2)+1;
    send(player1,buffer1,bytes1,flags);
    send(player2,buffer2,bytes2,flags);
        
    }
    else if(player1_gy<player2_gy)
    {
        strcpy(buffer2,"Gratulálok, te nyertél!");
        strcpy(buffer1,"Sajnos vesztettél!");
        bytes1=strlen(buffer1)+1;
        bytes2=strlen(buffer2)+1;
    send(player1,buffer1,bytes1,flags);
    send(player2,buffer2,bytes2,flags);
    }
    else
    {
        strcpy(buffer1,"Dontetlen lett!");
        bytes1=strlen(buffer1)+1;
        send(player1,buffer1,bytes1,flags);
        send(player2,buffer1,bytes1,flags);
        
    }
feladom: if(!strcmp(buffer1,"feladom"))
{
    strcpy(buffer1,"Feladtad!");
    strcpy(buffer2,"Az ellenfeled feladta!");
    bytes1 = strlen(buffer1)+1;
    bytes2 = strlen(buffer2)+1;
    send(player1,buffer1,bytes1,flags);
    send(player2,buffer2,bytes2,flags);
}else if(!strcmp(buffer2,"feladom")){
    strcpy(buffer2,"Feladtad!");
    strcpy(buffer1,"Az ellenfeled feladta!");
    bytes1 = strlen(buffer1)+1;
    bytes2 = strlen(buffer2)+1;
    send(player1,buffer1,bytes1,flags);
    send(player2,buffer2,bytes2,flags);
}
    /* Closing sockets and quit */
    close(player1);
    close(player2);
    close(fd);
    exit(0);
}

