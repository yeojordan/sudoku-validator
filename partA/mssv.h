

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "fileIO.h"

#define NINE 9
#define SUB 3

int checkRow(int numbers[], int rows, int cols, int (*matrix)[NINE][NINE] );
int checkCol(int numbers[], int rows, int cols, int (*matrix)[NINE][NINE] );
int checkSub(int numbers[], int rows, int cols, int (*matrix)[NINE][NINE] );
void resetArray(int numbers[]);
