#include "socketInterface.h"
#include "potato.h"
void receivePlayerInfo(playerMessage* playerInfo,int * clientSocket,int playerNumber){
    for(int i=0;i<playerNumber;++i){
        recv(clientSocket[i],(void *)&playerInfo[i].listeningPort,sizeof(playerInfo[i].listeningPort),MSG_WAITALL);
    }
}

//argument error checking. if no error finds, store portNumber, player number, hops number.
void argumentErrorCheck(int argc, char ** argv,
                        int * portNumber,int *playerNumber,int *hopsNumber){
    if(argc!=4){
        errorHandler(INVALID_RINGMASTER_ARGUMENT_INPUT);
    }
    *portNumber=atoi(argv[1]);
    *playerNumber=atoi(argv[2]);
    *hopsNumber=atoi(argv[3]);

    if(*hopsNumber>MAX_HOPS_NUM||*hopsNumber<0){
        errorHandler(INVALID_HOPS_NUM);
    }
    if(*playerNumber<=1){
        errorHandler(INVALID_PLAYER_NUM);
    }
}
//print ringmaster information: title, player number, hop number.
void printRingMasterInfo(int playerNumber,int hopsNumber){
    printf("Potato Ringmaster\n");
    printf("Players = %d\n",playerNumber);
    printf("Hops = %d\n",hopsNumber);
}
playerMessage * initPlayerInfoSet(int playerNumber){
    playerMessage* playerInfo = malloc(playerNumber * sizeof(*playerInfo));
    for(int i=0;i<playerNumber;++i){
        playerInfo[i].listeningPort=0;
        for(int j=0;j<MAX_IP_SIZE;++j){
            playerInfo[i].IP[j]=DIRTY;
        }
    }
    return playerInfo;
}
//keep checking incoming connection to listenSocket until reach player number.
//Record player information and create client socket
void waitAllPlayerConnect(int playerNumber,int listenSocket,int * clientSocket,playerMessage * playerInfo){
    int currPlayerConn = 0;
    fd_set playerfdSet;
    int max_fd=0;
    while(currPlayerConn < playerNumber) {
        FD_ZERO(&playerfdSet);
        FD_SET(listenSocket, &playerfdSet);
        max_fd = listenSocket;
        for (int i = 0; i < playerNumber; ++i) {
            int sd = clientSocket[i];
            if (sd > 0) {
                max_fd = max_fd < sd ? sd : max_fd;
                FD_SET(sd, &playerfdSet);
            }
        }

        int connection = select(max_fd + 1, &playerfdSet, NULL, NULL, NULL);
        if(connection <0){
            errorHandler(FAIL_SOCKET_SELECT);
        }
        if(FD_ISSET(listenSocket,&playerfdSet)){
            struct sockaddr_in address;
            int addressSize=sizeof(address);
            int playerSocket = accept(listenSocket, (struct sockaddr *)&address,(socklen_t * )&addressSize);
            if(playerSocket<0){
                errorHandler(FAIL_SOCKET_ACCEPT);
            }
            ++currPlayerConn;
            addPlayerSocket(clientSocket,playerSocket,playerNumber);
            addIPNumber(playerInfo,inet_ntoa(address.sin_addr),playerNumber);
        }
    }
}
//send each player's neighbor's information to player.

void sendNeighborPlayerInfo(int playerNumber,int * clientSocket,playerMessage * playerInfo){
    for(int i=0;i<playerNumber;++i){
        neighborMessage msg;
        msg.ID = i;
        int leftID = (i == 0) ? playerNumber-1 : i-1;
        int rightID = (i == playerNumber-1) ? 0 : i+1;
        msg.leftNeighborPort = playerInfo[leftID].listeningPort;
        msg.rightNeighborPort = playerInfo[rightID].listeningPort;
        strcpy(msg.leftNeighborIP,playerInfo[leftID].IP);
        strcpy(msg.rightNeighborIP,playerInfo[rightID].IP);
        msg.playerNumber = playerNumber;
        send(clientSocket[i],(void *)&msg,sizeof(msg),0);
    }
}
//wait Ack from each player indicates player finish connection with its neighbors.

void waitPlayerAck(int playerNumber,int * clientSocket){
    int Ack = 0;
    for(int i=0;i<playerNumber;++i){
        recv(clientSocket[i], &Ack, sizeof(Ack),MSG_WAITALL);
        if(Ack != FINISH_CONNECTION){
            errorHandler(INVALID_ACK);
        }
        printf("Player %d is ready to play\n",i);
    }
}

//send potato to a randomly selected player
void sendPotatoToRandomPlayer(Potato potato,int playerNumber
                              ,int * clientSocket,int listenSocket){
    int randomPlayer = generateRandomInt(0,playerNumber-1);
    printf("Ready to start the game, sending potato to player %d\n",randomPlayer);
    send(clientSocket[randomPlayer],&potato,sizeof(potato),0);
}
//wait potato send back
Potato waitPotatoBack(int playerNumber,int listenSocket,int * clientSocket){
    Potato potato;
    fd_set playerfdSet;
    int max_fd = 0;
    FD_ZERO(&playerfdSet);
    FD_SET(listenSocket,&playerfdSet);
    max_fd = listenSocket ;
    for(int i=0;i<playerNumber;++i){
        FD_SET(clientSocket[i],&playerfdSet);
        max_fd = max_fd <clientSocket[i]?clientSocket[i]:max_fd;
    }
    if (select(max_fd + 1, &playerfdSet, NULL, NULL, NULL) < 0) {
        errorHandler(FAIL_SOCKET_SELECT);
    }
    if (FD_ISSET(listenSocket, &playerfdSet)) {
        recv(listenSocket, &potato, sizeof(potato), MSG_WAITALL);
    }
    else{
        for(int i=0;i<playerNumber;++i){
            if(FD_ISSET(clientSocket[i], &playerfdSet)) {
                recv(clientSocket[i], &potato, sizeof(potato), MSG_WAITALL);
            }
        }
    }
    return potato;
}
//print out potato route
void printPotatoPath(Potato potato){
    printf("Trace of potato:\n");
    for (int i = 0; i < potato.pathSize; ++i) {
        if (i != 0) {
            printf(",");
        }
        printf("%d", potato.path[i]);
    }
    printf("\n");
}
//notifiy each player the game is over

void notifyClientCloseGame(int playerNumber,int * clientSocket){
    Potato potato = initPotato(-1);
    for(int i=0;i<playerNumber;++i){
        send(clientSocket[i],&potato,sizeof(potato),0);
    }
}
//close all the socket of ringmaster
void closeAllSocket(int * clientSocket,int listenSocket,int playerNumber ){
    for(int i=0;i<playerNumber;++i){
        close(clientSocket[i]);
    }
    close(listenSocket);
}
int main(int argc, char ** argv){
    srand((unsigned)time(NULL));
    int portNumber,playerNumber,hopsNumber;
    argumentErrorCheck(argc,argv,
                       &portNumber,&playerNumber,&hopsNumber);

    //print ringmaster information: title, player number, hop number.
    printRingMasterInfo(playerNumber,hopsNumber);

    int * clientSocket = initClientSocketSet(playerNumber);
    playerMessage * playerInfo = initPlayerInfoSet(playerNumber);

    //create a listen socket
    int listenSocket= openListenFileDescripter(&portNumber,playerNumber);

    //keep checking incoming connection to listenSocket until reach player number.
    //Record player information and create client socket
    waitAllPlayerConnect(playerNumber,listenSocket,clientSocket,playerInfo);

    //receive player listening port information and update playerInfo
    receivePlayerInfo(playerInfo,clientSocket,playerNumber);

    //send each player's neighbor's information to player.
    sendNeighborPlayerInfo(playerNumber,clientSocket,playerInfo);

    //wait Ack from each player indicates player finish connection with its neighbors.
    waitPlayerAck(playerNumber,clientSocket);

    Potato potato = initPotato(hopsNumber);
    //send potato to a randomly selected player
    sendPotatoToRandomPlayer(potato,playerNumber,clientSocket,listenSocket);

    //wait potato send back
    potato = waitPotatoBack(playerNumber,listenSocket,clientSocket);

    //print out potato route
    printPotatoPath(potato);

    //notifiy each player the game is over
    notifyClientCloseGame(playerNumber,clientSocket);
    //close all the socket of ringmaster
    closeAllSocket(clientSocket,listenSocket,playerNumber);

    free(clientSocket);
    free(playerInfo);
    return 0;
}