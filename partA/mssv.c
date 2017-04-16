
#include "mssv.h"


int main (int argc, char* argv[])
{

    // Rename command line parameters
    char* inputFile = argv[1];
    int maxDelay = atoi(argv[2]);

    // Validation
    // Ensure correct number of command line parameters
    if (argc != 3)
    {
        printf("Ensure there are the correct number of parameters\n");
        return -1;
    }

    // Ensure maxDelay is positive
    if (maxDelay < 0)
    {
        printf("The maxDelay must be non-negative\n");
        return -1;
    }

    // Variables
    int readStatus;
    int numbers[] = {0,0,0,0,0,0,0,0,0};
    int row, i;

    // File Descriptors
    int buff1FD, buff2FD, counterFD;

    // Shared memory sizes
    size_t buff1Sz = sizeof(int) * 11;
    size_t buff2Sz = sizeof(int) * 9 * 9;
    size_t countSz = sizeof(int) * 1;

    // Shared memory pointers
    int* buff1Ptr;
    int (*buff2Ptr)[NINE][NINE];
    int* countPtr;

    // Matrix
    int truncStat1 = 0;
    int truncStat2 = 0;
    int truncStat3 = 0;

    // Create shared memory
    buff1FD = shm_open("buffer1", O_CREAT | O_RDWR, 0666);
    buff2FD = shm_open("buffer2", O_CREAT | O_RDWR, 0666);
    counterFD = shm_open("counter", O_CREAT | O_RDWR, 0666);

    if ( buff1FD == -1 || buff2FD == -1 || counterFD == -1 )
    {
        fprintf( stderr, "Error creating shared memory blocks\n" );
        return -1;
    }

    // Give shared memory blocks a size
    truncStat1 = ftruncate(buff1FD, buff1Sz);
    truncStat2 = ftruncate(buff2FD, buff2Sz);
    truncStat3 = ftruncate(counterFD, countSz);

    printf("%d %d %d\n", truncStat1, truncStat2, truncStat3);
    // if (truncStat1 == -1 || truncStat2 == -1 || truncStat3 == -1)
    // {
    //     fprintf( stderr, "Error setting size of shared memory\n" );
    //     return -1;
    // }

    // Memory mapping
    buff1Ptr = (int*) mmap(NULL, buff1Sz, PROT_READ | PROT_WRITE, MAP_SHARED, buff1FD, 0);
    buff2Ptr = mmap(NULL, buff2Sz, PROT_READ | PROT_WRITE, MAP_SHARED, buff2FD, 0);
    countPtr = (int*) mmap(NULL, countSz, PROT_READ | PROT_WRITE, MAP_SHARED, counterFD, 0);

    // Initialise counter
    *countPtr = 0;

    // Read input file
    readStatus = readFile(inputFile, NINE, NINE, buff2Ptr);
    int val = (*buff2Ptr)[0][0];
    printf("Test: %d\n", val);
    if (readStatus != 0)
    {
        printf("Error reading contents of file");
        return -1;
    }



    // Check rows
    for (i = 0; i < 9; i++)
    {
        printf("Checking row: %d\n", i+1);
        row = checkRow( numbers, i, 9, 9, 9, buff2Ptr );
        if ( row != 0)
        {
            // Write to log file
            // Add method here
        }
        else
        {
            // Increment valid sub-grid counter
            (*countPtr)++;
            //printf("%d\n", *countPtr);
        }
        resetArray(numbers);
    }
    printf("%d\n", *countPtr);
/*
    // Check Columns
    for (i = 0; i < 9; i++)
    {
        printf("Checking column: %d\n", i+1);
        row = checkCol(buff2Ptr, numbers, 9, i+1 );
        if ( row != 0)
        {
            // Write to log file
            // Add method here
        }
        else
        {

            // Increment valid sub-grid counter
            (*countPtr)++;
            //printf("%d\n", *countPtr);
        }
        resetArray(numbers);
    }
    printf("%d\n", *countPtr);
*/


munmap(buff1Ptr, buff1Sz);
munmap(buff2Ptr, buff2Sz);
munmap(countPtr, countSz);

shm_unlink("buffer1");
shm_unlink("buffer2");
shm_unlink("counter");
}



/******************************************************************************/


// Row is zero based
// Cols starts from 1
int checkRow(int numbers[], int rows, int cols, int x, int y, int (*matrix)[x][y] )
{
    int i,j;
    int val = 0;
    int status = 0;
    for ( i = 0; i < cols; i++)
    {

        // Obtain the value in buffer2

        val = (*matrix)[rows][i];

        //val = matrix[rows*cols+i];
        printf("%d ", val);
        // Increment numbers for each occurrence
        numbers[val-1]++;
    }

    // If row is invalid
    for ( j = 0; j < NINE; j++ )
    {
        if ( numbers[j] != 1)
        {
            //printf("Invalid Row: %d\n", rows+1);
            return (rows+1);
            //status = rows + 1;
        }
        //numbers[j] = 0;
    }

    //printf(" Row: %d\n Status: %d\n", rows+1, status);
    //printf("Valid Row: %d\n", rows+1);
    return status;
}

// Rows start from 1
// Columns start from 1
int checkCol(int* matrix, int numbers[], int rows, int cols)
{
    int i,j;
    int val;
    int status = 0;
    for ( i = 0; i < rows; i++)
    {
        // Obtain the value in buffer2
        val = matrix[rows*i+(cols-1)];
        printf("%d\n", val);
        // Increment numbers for each occurrence
        numbers[val-1]++;
    }

    // If column is invalid
    for ( j = 0; j < 9; j++ )
    {
        if ( numbers[j] != 1)
        {
            //printf("Invalid Row: %d\n", rows+1);
            return (cols);
            //status = rows + 1;
        }
        //numbers[j] = 0;
    }


    return status;
}



void resetArray(int numbers[])
{
    int i;
    for( i = 0; i < 9; i++)
    {
        numbers[i] = 0;
    }
}
