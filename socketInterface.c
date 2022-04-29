//
// Created by bhc on 2022/3/20.
//

#include "socketInterface.h"
void errorHandler(int errorType){
    if(errorType==INVALID_RINGMASTER_ARGUMENT_INPUT){
        perror("Usage: RingMaster <port_num> <num_players> <num_hops>\n");
        exit(EXIT_FAILURE);
    }
    else if(errorType==INVALID_PLAYER_ARGUMENT_INPUT){
        perror("Usage: Player <machine_name> <port_number>\n");
        exit(EXIT_FAILURE);
    }
    else if (errorType==INVALID_PLAYER_NUM){
        perror("The number of player should be greater than one!\n");
        exit(EXIT_FAILURE);
    }
    else if (errorType==INVALID_HOPS_NUM){
        perror("The number of hops should be greater than or equal to 0 and less than or equal to 512!\n");
        exit(EXIT_FAILURE);
    }
    else if (errorType==FAIL_SOCKET_CREATION){
        perror("Socket creation failed!\n");
        exit(EXIT_FAILURE);
    }
    else if (errorType==FAIL_SOCKET_BIND){
        perror("Socket bind port failed!\n");
        exit(EXIT_FAILURE);
    }
    else if (errorType==FAIL_SOCKET_LISTEN){
        perror("Socket bind port failed!\n");
        exit(EXIT_FAILURE);
    }
    else if (errorType==FAIL_SOCKET_SELECT){
        perror("Socket select failed!\n");
        exit(EXIT_FAILURE);
    }
    else if (errorType==FAIL_SOCKET_ACCEPT){
        perror("Socket accept failed!\n");
        exit(EXIT_FAILURE);
    }
    else if (errorType==FAIL_SOCKET_CONNECTION){
        perror("Socket connection failed!\n");
        exit(EXIT_FAILURE);
    }
    else if (errorType==INVALID_ACK){
        perror("Socket accept failed!\n");
        exit(EXIT_FAILURE);
    }

}

struct sockaddr_in createSocketAddress(int sin_family,int ip, int portNum){
    struct sockaddr_in socketAddress;
    socketAddress.sin_family = sin_family;
    socketAddress.sin_addr.s_addr = ip;
    socketAddress.sin_port = htons(portNum);
    return socketAddress;
}

int openConnFileDescripter(char * hostName, int portName){
    //create a new socket
    int newSocket;
    if(!(newSocket = socket(AF_INET,SOCK_STREAM,0))) {
        errorHandler(FAIL_SOCKET_CREATION);
    }
    struct sockaddr_in addr ;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(hostName);
    addr.sin_port = htons(portName);
    //request for connection
    if(connect(newSocket, (struct sockaddr *) &addr, sizeof(addr))<0){
        errorHandler(FAIL_SOCKET_CREATION);
    };
    return newSocket;
}

int openListenFileDescripter(int * portNumber, int maxListenCapacity){
    int newSocket;
    int opt = 1;
    struct sockaddr_in socketAddress=createSocketAddress(AF_INET,INADDR_ANY,*portNumber);
    //create socket
    if(!(newSocket = socket(AF_INET,SOCK_STREAM,0))) {
        errorHandler(FAIL_SOCKET_CREATION);
    }
    if(setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < 0 ){
        errorHandler(FAIL_SOCKET_CREATION);
    }
    //bind with port
    if(*portNumber==0){
        //randomly generate port number if input 0
        do{
            * portNumber = generateRandomInt(CLIENT_MIN_PORT,CLIENT_MAX_PORT);
            socketAddress.sin_port = htons(*portNumber);
        }while(bind(newSocket, (struct sockaddr *)&socketAddress, sizeof(socketAddress))<0);
    }
    else{
        if(bind(newSocket, (struct sockaddr *)&socketAddress, sizeof(socketAddress))<0){
            errorHandler(FAIL_SOCKET_BIND);
        }
    }
    struct sockaddr_in sockaddrInfo;
    int size=sizeof(sockaddrInfo);
    getsockname(newSocket, (struct sockaddr *)&sockaddrInfo, (socklen_t *)&size);
    //listen for connection
    if (listen(newSocket,maxListenCapacity) < 0){
        errorHandler(FAIL_SOCKET_LISTEN);
    }
    return newSocket;
}


int addPlayerSocket(int * clientSocket,int playerSocket,int playerNumber) {
    for (int i = 0; i < playerNumber; ++i) {
        if (clientSocket[i] == 0) {
            clientSocket[i] = playerSocket;
            return 1;
        }
    }
    return 0;
}
int addIPNumber(playerMessage* playerInfo,char * IP,int playerNumber){
    for (int i = 0; i < playerNumber; ++i) {
        if (playerInfo[i].IP[0]==DIRTY) {
            memcpy(playerInfo[i].IP,IP,sizeof(playerInfo[i].IP));
        }
    }
    return 0;
}
int generateRandomInt(int left,int right){
    return left+rand()%(right-left+1);
}

int * initClientSocketSet(int clientNumber){
    int * clientSocket = malloc(clientNumber * sizeof(int));
    for(int i=0;i<clientNumber;++i){
        clientSocket[i]=0;
    }
    return clientSocket;
}