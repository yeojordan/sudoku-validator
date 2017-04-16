

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

int checkRow(int numbers[], int rows, int cols, int x, int y, int (*matrix)[x][y] );
int checkCol(int* matrix, int numbers[], int rows, int cols);
void resetArray(int numbers[]);
