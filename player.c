#include "socketInterface.h"
#include "potato.h"
void waitConnectionFromLeftRightNeighbor(int * clientSocket,int listenSocket,int *max_fd,neighborMessage buffer){
    fd_set fdSet;
    while(clientSocket[0]==0 || clientSocket[1]==0) {
        FD_ZERO(&fdSet);
        FD_SET(listenSocket, &fdSet);
        *max_fd = listenSocket;
        for (int i = 0; i < NEIGHBOR_NUM; ++i) {
            int sd = clientSocket[i];
            if (sd > 0) {
                *max_fd = *max_fd < sd ? sd : *max_fd;
                FD_SET(sd, &fdSet);
            }
        }
        int connection = select(*max_fd + 1, &fdSet, NULL, NULL, NULL);

        if(connection <0){
            errorHandler(FAIL_SOCKET_SELECT);
        }

        if(FD_ISSET(listenSocket,&fdSet)){
            struct sockaddr_in address;
            int addressSize=sizeof(address);
            int newSocket = accept(listenSocket, (struct sockaddr *)&address,(socklen_t * )&addressSize);

            int buffer;
            recv(newSocket,&buffer,sizeof(buffer),MSG_WAITALL);
            *max_fd = *max_fd < newSocket ? newSocket : *max_fd;
            if(newSocket<0){
                errorHandler(FAIL_SOCKET_ACCEPT);
            }
            if(buffer == LEFT_NEIGHBOR){
                clientSocket[0] = newSocket;
            }
            else{
                clientSocket[1] = newSocket;
            }
        }
    }
}
Potato waitPotato(int ringMasterSocket, int *clientSocket,
                int leftNeighborSocket,int rightNeighborSocket,int max_fd){
    fd_set fdSet;
    FD_ZERO(&fdSet);
    FD_SET(ringMasterSocket, &fdSet);
    FD_SET(leftNeighborSocket, &fdSet);
    FD_SET(rightNeighborSocket, &fdSet);
    Potato potato;
    if(select(max_fd + 1, &fdSet, NULL, NULL, NULL)<0){
        errorHandler(FAIL_SOCKET_SELECT);
    }
    if(FD_ISSET(ringMasterSocket,&fdSet)) {
        recv(ringMasterSocket,&potato,sizeof(potato),MSG_WAITALL);
    }
    else if(FD_ISSET(leftNeighborSocket,&fdSet)){
        recv(leftNeighborSocket,&potato,sizeof(potato),MSG_WAITALL);
    }
    else if(FD_ISSET(rightNeighborSocket,&fdSet)) {
        recv(rightNeighborSocket, &potato, sizeof(potato), MSG_WAITALL);
    }
    return potato;
}
void updatePotatoInfo(Potato * potato,int myID){
    potato->path[potato->pathSize] = myID;
    potato->pathSize++;
    potato->hops --;
}

void sendPotatoToRandomClient(int * clientSocket,Potato potato,int ID,int playerNumber){
    int chooseNeighbor = generateRandomInt(0,1);
    ID = ID + (chooseNeighbor == 0 ? -1:1) ;
    if(ID < 0){
        ID += playerNumber;
    }
    else if(ID == playerNumber){
        ID = 0;
    }
    printf("Sending potato to %d\n",ID);
    int socketToSend = chooseNeighbor == 0? clientSocket[0]:clientSocket[1];
    send(socketToSend, &potato, sizeof(potato), 0);
}
void closeAllSocket(int * clientSocket,int leftNeighborSocket,int rightNeighborSocket,int ringMasterSocket){
    close(clientSocket[0]);
    close(clientSocket[1]);
    close(leftNeighborSocket);
    close(rightNeighborSocket);
    close(ringMasterSocket);
}

int main(int argc, char * argv[]) {
    srand((unsigned)time(NULL));
    if (argc != 3) {
        errorHandler(INVALID_PLAYER_ARGUMENT_INPUT);
    }
    char *machineName = argv[1];
    int portNumber = atoi(argv[2]);
    int max_fd = 0; int listenPort = 0;
    //connect to ring master.
    int ringMasterSocket = openConnFileDescripter(machineName,portNumber);
    int * clientSocket = initClientSocketSet(NEIGHBOR_NUM);

    //connect to ring master.
    int myListenSocket =
            openListenFileDescripter(&listenPort,NEIGHBOR_NUM);

    max_fd = max_fd < myListenSocket ? myListenSocket : max_fd;

    //send player's listening port to ring master
    send(ringMasterSocket,&listenPort,sizeof(listenPort),0);

    //wait for ring master's message about neighbors' information
    neighborMessage buffer;
    recv(ringMasterSocket, &buffer, sizeof(buffer),MSG_WAITALL);

    int myID = buffer.ID;
    int playerNumber = buffer.playerNumber;

    printf("Connected as player %d out of %d total players\n",myID,playerNumber);
    //connect with left and right neighbor's socket
    int leftNeighborSocket = openConnFileDescripter(buffer.leftNeighborIP,buffer.leftNeighborPort);
    int rightNeighborSocket = openConnFileDescripter(buffer.rightNeighborIP,buffer.rightNeighborPort);


    //send message to left and right neighbor indicates its relative position
    int msg = LEFT_NEIGHBOR;
    send(rightNeighborSocket,&msg,sizeof(msg),0);
    msg = RIGHT_NEIGHBOR;
    send(leftNeighborSocket,&msg,sizeof(msg),0);


    //wait left and right neighbor connect to my socket
    waitConnectionFromLeftRightNeighbor(clientSocket,myListenSocket,&max_fd,buffer);

    //send message to ring master indicates finish connection
    int finishConnection = FINISH_CONNECTION;
    send(ringMasterSocket,&finishConnection,sizeof(finishConnection),0);

    while(1){
        //wait potato receive
        Potato potato =
                waitPotato(ringMasterSocket,clientSocket,leftNeighborSocket,rightNeighborSocket,max_fd);
        if(potato.hops > 1) {
            updatePotatoInfo(&potato,myID);
            //send to randomly selected neighbor
            sendPotatoToRandomClient(clientSocket,potato,myID,playerNumber);
        }
        else {
            //if hops < 0, indicates close connection , if hops = 0, send back to master
            if(potato.hops == 1){
                printf("I'm it\n");
                updatePotatoInfo(&potato,myID);
                send(ringMasterSocket, &potato, sizeof(potato), 0);
                continue;
            }
            closeAllSocket(clientSocket,leftNeighborSocket,rightNeighborSocket,ringMasterSocket);
            break;
        }
    }
    return EXIT_SUCCESS;
}