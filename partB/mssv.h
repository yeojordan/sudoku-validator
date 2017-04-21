#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#define NINE 9
#define SUB 3
#define NUMTHREADS 11
#define FALSE 0
#define TRUE !FALSE

typedef enum {ROW, COL, SUB_REGION} Region_Type;

typedef struct
{
    Region_Type type;
    int positionX;
    pthread_t tid;
    int valid;
    int numbers[NINE];// = {0,0,0,0,0,0,0,0,0};

} Region;


void readFile(char* inputFile, int rows, int cols, int***buffer);
void writeFile(Region* region, char* format);
void resetArray(int numbers[]);
int checkValid(int numbers[]);
void parentManager(Region *region, sem_t *semaphores, int* countPtr,
                        int* resourceCount );
void childManager(Region *region, sem_t *semaphores,
                    int (*buff1Ptr)[NINE][NINE], int *buff2Ptr, int* countPtr,
                        int* resourceCount, int processNum, int *numbers, 
                            int maxDelay );
void initMemory( int*** buff1, int** buff2, int** counter);
void mapMemory(int* buff1FD, int* buff2FD, int* counterFD, int* semFD,
                    int* regionFD, int* resFD, int (**buff1Ptr)[NINE][NINE],
                        int (**buff2Ptr), int** countPtr, sem_t** semaphores,
                            Region** region, int** resourceCount);
void validateUse(int argc, char* argv[]);
void cleanMemory(void);


