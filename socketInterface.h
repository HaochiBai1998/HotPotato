#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <netdb.h>
#include <time.h>
#define MAX_HOPS_NUM 512
#define NEIGHBOR_NUM 2
#define MAX_IP_SIZE 16
#define CLIENT_MIN_PORT 1024
#define CLIENT_MAX_PORT 4999
enum{
    //error type
    INVALID_RINGMASTER_ARGUMENT_INPUT,
    INVALID_PLAYER_ARGUMENT_INPUT,
    INVALID_PLAYER_NUM,
    INVALID_HOPS_NUM,
    FAIL_SOCKET_CREATION,
    FAIL_SOCKET_BIND,
    FAIL_SOCKET_LISTEN,
    FAIL_SOCKET_SELECT,
    FAIL_SOCKET_ACCEPT,
    FAIL_SOCKET_CONNECTION,
    DIRTY,
    FINISH_CONNECTION,
    INVALID_ACK,
    LEFT_NEIGHBOR,
    RIGHT_NEIGHBOR
};

/**
 * neighbor message is a struct for ring master sending message to each player.
 * The message contains
 * 1. the player's left neighbor IP address, port address.
 * 2. the player's right neighbor IP address, port address.
 * 3. the ID ring master assigns to the player.
 * 4. the total player number
 * */

struct _neighbor_message{
    char leftNeighborIP[MAX_IP_SIZE];
    char rightNeighborIP[MAX_IP_SIZE];
    int leftNeighborPort;
    int rightNeighborPort;
    int ID;
    int playerNumber;
};
typedef struct _neighbor_message neighborMessage;

/**
 * player message is a struct for player sending its listening socket
 * IP and port address to ring master.
 * */

struct _player_message{
    char IP[MAX_IP_SIZE];
    int listeningPort;
};
typedef struct _player_message playerMessage;


/**
 * openConnFileDescripter function creates a connection socket to a
 * particular hostName:portName.
 * */
int openConnFileDescripter(char * hostName, int portName);

/**
 * createSocketAddress function creates a struct sockaddr_in variable
 * with parameter sin_family,ip,portNum.
 * */
struct sockaddr_in createSocketAddress(int sin_family,int ip, int portNum);

/**
 * createSocketAddress function creates a struct sockaddr_in variable
 * with parameter sin_family,ip,portNum.
 * */
int openListenFileDescripter(int * portName,int maxListenCapacity);

/**
 * errorHandler function deals with different error according to input
 * errorType.
 * */
void errorHandler(int errorType);

/**
 * addPlayerSocket adds a playerSocket into a free space of clientSocket.
 * */
int addPlayerSocket(int * clientSocket,int playerSocket,int playerNumber);

/**
 * addIPNumber adds a IP into a free space of playerInfo.
 * */
int addIPNumber(playerMessage* playerInfo,char * IP,int playerNumber);

/**
 * generateRandomInt generates a random integer.
 * */
int generateRandomInt(int left,int right);

/**
 * initClientSocketSet initialize a client socket set with size
 * client number.
 * */
int * initClientSocketSet(int clientNumber);