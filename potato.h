#ifndef MAX_HOPS_NUM
#define MAX_HOPS_NUM 512
#endif

struct _Potato{
    int hops;
    int pathSize;
    int path[MAX_HOPS_NUM];
};
typedef struct _Potato Potato;

Potato initPotato(int hopsNumber){
    Potato potato;
    potato.hops = hopsNumber;
    potato.pathSize=0;
    return potato;
}