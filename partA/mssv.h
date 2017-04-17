




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


#define NINE 9
#define SUB 3
#define NUMPROCESSES 11
#define FALSE 0
#define TRUE !FALSE

typedef enum {ROW, COL, SUB_REGION} Region_Type;

typedef struct
{
    Region_Type type;
    int positionX;
    int positionY;
    pid_t pid;
    int valid;
} Region;


int readFile(char* inputFile, int rows, int cols, int (*buffer)[rows][cols]);
int checkRow(int numbers[], int rows, int cols, int (*matrix)[NINE][NINE] );
int checkCol(int numbers[], int rows, int cols, int (*matrix)[NINE][NINE] );
int checkSub(int numbers[], int rows, int cols, int (*matrix)[NINE][NINE] );
void writeFile(Region* region, char* format);
void resetArray(int numbers[]);
