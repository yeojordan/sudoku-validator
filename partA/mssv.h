

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "fileIO.h"

int checkRow(int* matrix, int numbers[], int rows, int cols);
int checkCol(int* matrix, int numbers[], int rows, int cols);
void resetArray(int numbers[]);
