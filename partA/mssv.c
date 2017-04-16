
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
    int row, i, j;
    Region* region;
    sem_t semFull, semEmpty, semMutex, *semaphores;
    int processNum = 0;
    int pid;

    // File Descriptors
    int buff1FD, buff2FD, counterFD, semFD, regionFD;

    // Shared memory sizes
    size_t buff1Sz = sizeof(int) * 11;
    size_t buff2Sz = sizeof(int) * 9 * 9;
    size_t countSz = sizeof(int) * 1;
    size_t semSz = sizeof(sem_t);
    size_t regionSz = sizeof(Region);

    // Shared memory pointers
    int* buff2Ptr;
    int (*buff1Ptr)[NINE][NINE];
    int* countPtr;


    // Matrix
    int truncStat1 = 0;
    int truncStat2 = 0;
    int truncStat3 = 0;

    // Create shared memory
    buff1FD = shm_open("buffer1", O_CREAT | O_RDWR, 0666);
    buff2FD = shm_open("buffer2", O_CREAT | O_RDWR, 0666);
    counterFD = shm_open("counter", O_CREAT | O_RDWR, 0666);
    semFD = shm_open("semaphores", O_CREAT | O_RDWR, 0666);
    regionFD = shm_open("region", O_CREAT | O_RDWR, 0666);


    if ( buff1FD == -1 || buff2FD == -1 || counterFD == -1 || semFD == -1 ||
            regionFD == -1 )
    {
        fprintf( stderr, "Error creating shared memory blocks\n" );
        return -1;
    }

    // Give shared memory blocks a size
    truncStat1 = ftruncate(buff1FD, buff1Sz);
    truncStat2 = ftruncate(buff2FD, buff2Sz);
    truncStat3 = ftruncate(counterFD, countSz);
    ftruncate( semFD, sizeof(sem_t) * 3 );
    ftruncate( regionFD, sizeof(Region));

    printf("%d %d %d\n", truncStat1, truncStat2, truncStat3);
    // if (truncStat1 == -1 || truncStat2 == -1 || truncStat3 == -1)
    // {
    //     fprintf( stderr, "Error setting size of shared memory\n" );
    //     return -1;
    // }

    // Memory mapping
    buff2Ptr = (int*) mmap(NULL, buff2Sz, PROT_READ | PROT_WRITE, MAP_SHARED, buff2FD, 0);
    buff1Ptr = mmap(NULL, buff1Sz, PROT_READ | PROT_WRITE, MAP_SHARED, buff1FD, 0);
    countPtr = (int*) mmap(NULL, countSz, PROT_READ | PROT_WRITE, MAP_SHARED, counterFD, 0);
    semaphores = mmap(NULL, sizeof(sem_t) * 3, PROT_READ | PROT_WRITE, MAP_SHARED, semFD, 0);
    region = mmap(NULL, sizeof(Region), PROT_READ | PROT_WRITE, MAP_SHARED, regionFD, 0);


    if ((sem_init(&semMutex, 1, 1)== 1)
        || (sem_init(&semFull, 1, 0) == 1)
            ||(sem_init(&semEmpty, 1, 1) == 1))
    {
        fprintf(stderr, "Could not initialise semaphores\n");
        exit(1);
    }

    semaphores[0] = semMutex;
    semaphores[1] = semFull;
    semaphores[2] = semEmpty;


    // Initialise counter
    *countPtr = 0;
row = 0;
    // Read input file
    readStatus = readFile(inputFile, NINE, NINE, buff1Ptr);
    if (readStatus != 0)
    {
        printf("Error reading contents of file");
        return -1;
    }

    pid = -1;
    processNum = 0;

    // Create child processes for rows
    while( processNum < NINE && pid != 0 )
    {
        printf("MAKING CHILDREN %d!\n", processNum);

        signal(SIGCHLD, SIG_IGN);
        pid = fork();
        processNum++;


    }

    // Child
    if( pid == 0)
    {
        // Check rows
        for (i = 0; i < NINE; i++)
        {
            printf("Checking row: %d\n", i+1);
            //row = checkRow( numbers, i, 9, buff1Ptr );

            numbers[((*buff1Ptr)[processNum][i])-1]++;

        }


        //printf("%d\n", *countPtr);
printf("Waiting for process %d\n", processNum );
                        //acquire locks
                        sem_wait(&(semaphores[2]));//Empty lock
                        sem_wait(&(semaphores[0]));//Mutex lock
                        //put into subtotal along with PID
                        //
printf("Finished waiting for %d\n", processNum );



                        region->type = ROW;
                        region->positionX = processNum;
                        region->positionY = 0;
                        region->pid = getpid();
                        region->valid = checkValid(numbers);
                    printf("Process ID: %d - Row: %d - Valid: %d\n", region->pid, processNum, region->valid);
                        //release locks
                        sem_post(&(semaphores[0]));
                        sem_post(&(semaphores[1]));

                        //resetArray(numbers);
    }
    // Parent
    else if ( pid > 0)
    {

        for(int ii = 0; ii < NINE; ii++)
        {
printf("HI\n");

            //wait
            sem_wait(&(semaphores[1]));//Lock full
            sem_wait(&(semaphores[0]));//Lock mutex

            printf("Process ID-%d: row %d is valid\n",
                    region->pid, (region->positionX)+1 );


            sem_post(&(semaphores[0]));//Unlock mutex
            sem_post(&(semaphores[2]));//Unlock empty
        }
    }

    // Create child for column
    // if( pid == 0)
    // {
    //
    // }

    // Create child for sub regions
    // if(pid == 0)
    // {
    //
    // }

/*
    // Check rows
    for (i = 0; i < 9; i++)
    {
        printf("Checking row: %d\n", i+1);
        row = checkRow( numbers, i, 9, buff1Ptr );
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

    // Check Columns
    for (i = 0; i < 9; i++)
    {
        printf("Checking column: %d\n", i+1);
        row = checkCol(numbers, 9, i+1, buff1Ptr);
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

    // Check sub grids
    for (i = 0; i < SUB; i++)
    {

        for (j = 0; j < SUB; j++)
        {
            printf("Checking subgrid %d, %d\n", i*SUB, j*SUB);
            row = checkSub(numbers, i*SUB, j*SUB, buff1Ptr);

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
    }

    printf("%d\n", *countPtr);
*/
    // Clean up shared memory
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
int checkRow(int numbers[], int rows, int cols, int (*matrix)[NINE][NINE] )
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
    printf("\n");
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
int checkCol(int numbers[], int rows, int cols, int (*matrix)[NINE][NINE] )
{
    int i,j;
    int val;
    int status = 0;
    for ( i = 0; i < rows; i++)
    {
        // Obtain the value in buffer2

        val = (*matrix)[cols-1][i];
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

int checkSub(int numbers[], int rows, int cols, int (*matrix)[NINE][NINE] )
{
    int i, j, val, status = 0;

    for( i = rows; i < rows + SUB; i++)
    {
        for( j = cols; j < cols + SUB; j++)
        {
            val = (*matrix)[i][j];

            printf("%d ", val);
            numbers[val-1]++;
        }
        printf("\n");
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

int checkValid(int numbers[])
{
    int j;
    for ( j = 0; j < 9; j++ )
    {
        if ( numbers[j] != 1)
        {
            //printf("Invalid Row: %d\n", rows+1);
            return (FALSE);
            //status = rows + 1;
        }
        //numbers[j] = 0;
    }

    return TRUE;
}
