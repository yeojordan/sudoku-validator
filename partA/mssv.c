#include "mssv.h"

int main (int argc, char* argv[])
{
    // Validate command line parameters
    validateUse(argc, argv);

    // Rename command line parameters
    char* inputFile = argv[1];
    int maxDelay = atoi(argv[2]);

    // Variables
    int i, pid, processNum, numbers[] = {0,0,0,0,0,0,0,0,0};
    Region* region;
    sem_t semMutex, semParent, *semaphores;

    // File Descriptors
    int buff1FD, buff2FD, counterFD, semFD, regionFD, resFD;

    // Shared memory pointers
    int *buff2Ptr, *countPtr, *resourceCount, (*buff1Ptr)[NINE][NINE];

    // Generate random maxDelay
    srand((unsigned) time(NULL));

printf("MAXDELAY :%d\n", maxDelay);
    maxDelay = rand() % maxDelay;

printf("MAXDELAY :%d\n", maxDelay);

    // Create shared memory
    initMemory( &buff1FD, &buff2FD, &counterFD, &semFD, &regionFD, &resFD);

    // Map shared memory to pointers
    mapMemory(&buff1FD, &buff2FD, &counterFD, &semFD, &regionFD, &resFD,
                &buff1Ptr, &buff2Ptr, &countPtr, &semaphores, &region,
                    &resourceCount);

    // Initialise semaphores
    if ((sem_init(&semMutex, 1, 1)== 1) ||(sem_init(&semParent, 1, 1) == 1))
    {
        fprintf(stderr, "Could not initialise semaphores\n");
        exit(1);
    }

    semaphores[0] = semMutex;
    semaphores[1] = semParent;

    // Initialise parameters
    *countPtr = 0;
    pid = -1;
    processNum = 0;

    // Read input file
    readFile(inputFile, NINE, NINE, buff1Ptr);

    // Parent aquires lock of resourceCount
    sem_wait(&(semaphores[1])); // Lock child

    *resourceCount = 0;

    // Create child processes for
    while( processNum < 11 && pid != 0 )
    {
        signal(SIGCHLD, SIG_IGN); // Kill zombie processNum-1
        pid = fork();

        // Allow the parent to increment shared variable count
        if ( pid > 0)
        {
            /*
            DEBUGGING
            printf("Child ID: %d, processNum: %d\n", pid, processNum);
            */
            *resourceCount = *resourceCount + 1;
            /*
            printf("Parent's resourceCount: %dn", *resourceCount);
             */
        }
        processNum++;
    }

    if( pid == 0) // Child process
    {
        childManager(region, semaphores, buff1Ptr, buff2Ptr, countPtr,
                            resourceCount,  processNum, numbers, maxDelay );
    }
    else if ( pid > 0) // Parent process
    {
        parentManager(region, semaphores, countPtr, resourceCount);

        // Clean up shared memory
        cleanMemory(&buff1Ptr, &buff2Ptr, &countPtr, &semaphores,
                       &region, &resourceCount, buff1FD, buff2FD, counterFD,
                            semFD, regionFD, resFD);
    }
    else // Unsuccessful child process creation attempt
    {
        fprintf(stderr, "Unable to create child processes. Please run \"killall mssv\"\n");
    }
}



/******************************************************************************/

/**
 * Read the contents of the input file passed as a command line argument
 * @param inputFile File to be read
 * @param rows      Number of rows in matrix
 * @param cols      Number of columns in matrix
 * @param buffer    Matrix to store contents of input file
 */
void readFile(char* inputFile, int rows, int cols, int (*buffer)[rows][cols])
{
    FILE* inStrm;
    int i, j;

    inStrm = fopen(inputFile, "r"); // Open file for reading

    if (inStrm == NULL) // Check file opened correctly
    {
        perror("Error opening file for reading\n");
        exit(1);
    }

    // Store contents of file in 2D array
    for( i = 0; i < rows; i++ )
    {
        for ( j = 0; j < cols; j++ )
        {
            fscanf( inStrm, "%d", &(*(buffer))[i][j] );

            // DEBUGGING
            printf("%d ", (*buffer)[i][j]);
        }
        printf("\n");
    }

    fclose(inStrm); // Close file
}

/**
 * Write the invalid regions to log file
 * @param region Sub region
 * @param format String to be written
 */
void writeFile(Region* region, char* format)
{
    char* filename = "logfile";
    FILE* outFile;
    int val;

    outFile = fopen(filename, "a"); // Open file for appending
    if (outFile == NULL) // Check file opened correctly
    {
        perror("Error opening file for writing\n");
        exit(1);
    }

    fprintf(outFile, "process ID-%d: %s",region->pid, format);

    fclose(outFile); // Close file
}

/**
 * Set each index to zero
 * @param numbers Array to be reset
 */
void resetArray(int numbers[])
{
    for (int i = 0; i < NINE; i++)
    {
        numbers[i] = 0;
    }
}

/**
 * Check if the contents of the array has any value other than one
 * @param  numbers Array to be checked
 * @return         Status of array being valid or not
 */
int checkValid(int numbers[])
{
    for (int j = 0; j < NINE; j++)
    {
        if ( numbers[j] != 1)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/**
 * Handles the routine for the parent process. Outputs the result to the screen
 * @param region        Array containing each region struct
 * @param semaphores    Array of all semaphores
 * @param countPtr      Pointer to shared memory counter
 * @param resourceCount Status of number of child processes executing
 */
void parentManager(Region *region, sem_t *semaphores, int* countPtr,
                        int* resourceCount)
{
    char *type, *message;
    sem_post(&(semaphores[1])); // Unlock child
    int done = FALSE;
	int position;

    while( !done ) // Wait for all children to finish executing
    {
        //printf("Parent Waiting for Children\n");
        sem_wait(&(semaphores[1])); // Lock child
        sem_wait(&(semaphores[0])); // Lock mutex
        if ( *resourceCount == 0)
        {
            done = TRUE;
            printf("DONE\n");
        }
        sem_post(&(semaphores[0])); // Unlock mutex
        sem_post(&(semaphores[1])); // Unlock child

    }

    for(int ii = 0; ii < 11; ii++)
    {
        sem_wait(&(semaphores[0])); //Lock mutex
        if (region[ii].type == ROW)
        {
            type = "row";
		    position = region[ii].positionX;
            if ( region[ii].valid == TRUE)
	        {
	            printf("Validation result from process ID-%d: %s %d is valid\n",
                                      	region[ii].pid,type, position);
          	}
	      	else
		    {
        		printf("Validation result from process ID-%d: %s %d is invalid\n",
		                   		region[ii].pid, type, position);
	        }

            /*
            type = "invalid"

            if (region[ii].valid == TRUE)
            {
                type = "valid"
            }
            printf("Validation result from process ID-%d: row %d is %s\n",
                                    region[ii].pid, position, type);
             */

    	}
        else if (region[ii].type == COL)
        {
            type = "column";
          	position = region[ii].positionX;
      		printf("Validation result from process ID-%d: %d out of 9 columns are valid\n",region[ii].pid, region[ii].positionX);
        }
        else
        {
            type = "sub-grid";
	        position = region[ii].positionX;

	        printf("Validation result from process ID-%d: %d out of 9 sub-grids are valid\n",region[ii].pid, region[ii].positionX);

        }

        sem_post(&(semaphores[0])); //Unlock mutex
    }


	if (*countPtr == 27)
	{
	     message = "valid";
	}
	else
	{
	    message = "invalid";
	}

	printf("There are %d valid sub-grids, and thus the solution is %s\n", *countPtr, message);
}


/**
 * Routine for child processes. Check the validity of sub region.
 * @param region        Sub-region struct for each process
 * @param semaphores    Array of semaphores
 * @param buff1Ptr      Pointer to buffer1 in shared memory
 * @param buff2Ptr      Pointer to buffer2 in shared memory
 * @param countPtr      Pointer to counter in shared memory
 * @param resourceCount Pointer to resourceCount in shared memory
 * @param processNum    Child process number
 * @param numbers       Array of numbers to check validity of sub region
 * @param maxDelay      Delay for each process
 */
void childManager(Region *region, sem_t *semaphores, int (*buff1Ptr)[NINE][NINE],
                    int *buff2Ptr, int* countPtr, int* resourceCount,
                        int processNum, int *numbers, int maxDelay )
{
    char format[500];
    int numValid;

	    if( processNum <= 9) // Check a row in buffer1
        {
            for (int i = 0; i < NINE; i++)
            {
                // Update numbers array
                numbers[((*buff1Ptr)[processNum-1][i])-1]++;
            }

            sleep(maxDelay); // Sleep
            sem_wait(&(semaphores[0])); //Lock mutex

            // Update region struct
            region[processNum-1].type = ROW;
            region[processNum-1].positionX = processNum;
            region[processNum-1].pid = getpid();
            region[processNum-1].valid = checkValid(numbers);

            numValid = 0;
            if (region[processNum-1].valid == TRUE)
            {
                numValid = 1;
            }
            else // Write to log file
            {
                sprintf(format, "row %d is invalid\n", processNum);
                writeFile(&(region[processNum-1]), format);
            }

            buff2Ptr[processNum-1] = numValid; // Update buffer2

            *countPtr = *countPtr + numValid; // Update counter

            sem_post(&(semaphores[0])); // Unlock child

        }
        else if(processNum == 10) // Check all columns
        {
            sprintf(format, "column ");

	        int validCol = 0;
	        for ( int nn = 0; nn < NINE; nn++) // Iterate through each column
	        {
	            for(int ii = 0; ii < NINE; ii++) // Iterate through each row
   	            {
                    numbers[(*buff1Ptr)[ii][nn]-1]++; // Update numbers array
	            }

                if ( checkValid( numbers) == TRUE )
	            {
		            validCol++;
	            }
                else
                {
                    sprintf(format + strlen(format), "%d, ", nn+1);
                }

		        resetArray(numbers);
	        }

            sleep(maxDelay);
	        sprintf(format + strlen(format), "are invalid\n");
			sem_wait(&(semaphores[0])); //Lock mutex

            // Update region struct
            region[processNum-1].type = COL;
            region[processNum-1].positionX = validCol;
            region[processNum-1].pid = getpid();
            if(validCol != 9)
            {
                writeFile(&(region[processNum-1]), format);
            }

            numValid = region[processNum-1].positionX;

            buff2Ptr[processNum-1] = validCol; // Update buffer2

            *countPtr = *countPtr + validCol; // Update counter

            sem_post(&(semaphores[0])); // Unlock mutex
        }
        else if( processNum == 11) // Check sub-grids
        {
            sprintf(format, "sub-grid ");

            int validSub = 0;

            // Iterate through each of the 9 3x3 sub-grid
            for ( int jj = 0; jj < 3; jj++)
            {
                for (int kk = 0; kk < 3; kk++)
	            {
		            for (int ll = jj*3; ll < jj*3+3; ll++)
    		        {
		                for (int mm = kk*3; mm < kk*3+3; mm++)
		                {
			                numbers[(*buff1Ptr)[ll][mm]-1]++;
		                }
		            }

	    	        if ( checkValid(numbers) == TRUE )
	    	        {
		                validSub++;
	   	            }
                    else // Update string for log file
                    {
                        sprintf(format+strlen(format), "[%d..%d, %d..%d], ",
                                    jj+1, jj+3, kk+1, kk+3);

                    }
		            resetArray(numbers);
	            }

            }

            sleep(maxDelay);
		    sprintf(format+strlen(format), "is invalid\n");
            sem_wait(&(semaphores[0])); //Lock mutex

            // Update region struct
            region[processNum-1].type = SUB_REGION;
            region[processNum-1].positionX = validSub;
            region[processNum-1].pid = getpid();

            if(validSub != 9) // Write to log file
            {
                writeFile(&(region[processNum-1]), format);
            }

            buff2Ptr[processNum-1] = validSub; // Update buffer 2

            *countPtr = *countPtr + validSub; // Update counter

            sem_post(&(semaphores[0])); // Unlock mutex
        }

        // Child signals it is finished by incremented resourceCount
        sem_wait(&(semaphores[1])); // Lock child
        sem_wait(&(semaphores[0])); // Lock mutex
            //printf("I'm done! pid-%d resCount = %d\n", getpid(), (*resourceCount)-1);
            *resourceCount = *resourceCount - 1;
        sem_post(&(semaphores[0])); // Unlock mutex
        sem_post(&(semaphores[1])); // Unlock child

}

/**
 * Initalise shared memory constructs
 * @param buff1FD   File descriptor for buffer1
 * @param buff2FD   File descriptor for buffer2
 * @param counterFD File descriptor for counter
 * @param semFD     File descriptor for semaphores
 * @param regionFD  File descriptor for regions
 * @param resFD     File descriptor for resourceCount
 */
void initMemory( int* buff1FD, int* buff2FD, int* counterFD, int* semFD,
                    int* regionFD, int* resFD)
{

    // Create shared memory
    *buff1FD = shm_open("buffer1", O_CREAT | O_RDWR, 0666);
    *buff2FD = shm_open("buffer2", O_CREAT | O_RDWR, 0666);
    *counterFD = shm_open("counter", O_CREAT | O_RDWR, 0666);
    *semFD = shm_open("semaphores", O_CREAT | O_RDWR, 0666);
    *regionFD = shm_open("region", O_CREAT | O_RDWR, 0666);
    *resFD = shm_open("resources", O_CREAT | O_RDWR, 0666);

    // Check shared memory was created correctly
    if ( *buff1FD == -1 || *buff2FD == -1 || *counterFD == -1 || *semFD == -1 ||
            *regionFD == -1 || *resFD == -1 )
    {
        fprintf( stderr, "Error creating shared memory blocks\n" );
        exit(1);
    }

    // Give shared memory blocks a size
    ftruncate(*buff1FD, sizeof(int) * NINE * NINE);
    ftruncate(*buff2FD, sizeof(int) * 11);
    ftruncate(*counterFD, sizeof(int));
    ftruncate(*semFD, sizeof(sem_t) * 2 );
    ftruncate(*regionFD, sizeof(Region)*11);
    ftruncate(*resFD, sizeof(int));
}

/**
 * Map shared memory to addresses
 * @param buff1FD       File descriptor for buffer1
 * @param buff2FD       File descriptor for buffer2
 * @param counterFD     File descriptor for counter
 * @param semFD         File descriptor for semaphores
 * @param regionFD      File descriptor for regions
 * @param resFD         File descriptor for resourceCount
 * @param buff1Ptr      Pointer to buffer1 in shared memory
 * @param buff2Ptr      Pointer to buffer2 in shared memory
 * @param countPtr      Pointer to counter in shared memory
 * @param semaphores    Array of semaphores
 * @param region        Array of region structs
 * @param resourceCount Pointer to resourceCount in shared memory
 */
void mapMemory(int* buff1FD, int* buff2FD, int* counterFD, int* semFD,
                  int* regionFD, int* resFD, int (**buff1Ptr)[NINE][NINE], int (**buff2Ptr),
                        int** countPtr, sem_t** semaphores, Region** region, int** resourceCount)
{
    // Memory mapping
    *buff2Ptr = (int*) mmap(NULL, sizeof(int)*NINE*NINE, PROT_READ | PROT_WRITE, MAP_SHARED, *buff2FD, 0);
    *buff1Ptr = mmap(NULL, sizeof(int)*11, PROT_READ | PROT_WRITE, MAP_SHARED, *buff1FD, 0);
    *countPtr = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, *counterFD, 0);
    *semaphores = mmap(NULL, sizeof(sem_t) * 2, PROT_READ | PROT_WRITE, MAP_SHARED, *semFD, 0);
    *region = mmap(NULL, sizeof(Region)* 11, PROT_READ | PROT_WRITE, MAP_SHARED, *regionFD, 0);
    *resourceCount = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, *resFD, 0);
}

/**
 * Validate command line parameters
 * @param argc number of parameters
 * @param argv command line parameters
 */
void validateUse(int argc, char* argv[])
{
    // Ensure correct number of command line parameters
    if (argc != 3)
    {
        printf("Ensure there are the correct number of parameters\n");
        exit(1);
    }

    // Ensure maxDelay is positive
    if ( atoi(argv[2]) < 0)
    {
        printf("The maxDelay must be non-negative\n");
        exit(1);
    }
}

/**
 * Close and destroy, semaphores and shared memory constructs
 * @param buff1Ptr      Pointer to buffer1 in shared memory
 * @param buff2Ptr      Pointer to buffer2 in shared memory
 * @param countPtr      Pointer to counter in shared memory
 * @param semaphores    Array of semaphores
 * @param region        Array of region structs
 * @param resourceCount Pointer to resourceCount in shared memory
 * @param buff1FD       File descriptor for buffer1
 * @param buff2FD       File descriptor for buffer2
 * @param counterFD     File descriptor for counter
 * @param semFD         File descriptor for semaphores
 * @param regionFD      File descriptor for regions
 * @param resFD         File descriptor for resourceCount
 */
void cleanMemory(int (**buff1Ptr)[NINE][NINE], int **buff2Ptr, int** countPtr,
                         sem_t **semaphores, Region **region,
                            int** resourceCount, int buff1FD, int buff2FD,
                                int counterFD, int semFD, int regionFD,
                                    int resFD )
{
    // Close semaphores
    sem_close(&((*semaphores)[0]));
    sem_close(&((*semaphores)[1]));

    // Destroy semaphores
    sem_destroy(&((*semaphores)[0]));
    sem_destroy(&((*semaphores)[1]));

    // Clean up shared memory
    shm_unlink("buffer1");
    shm_unlink("buffer2");
    shm_unlink("counter");
    shm_unlink("semaphores");
    shm_unlink("region");
    shm_unlink("resources");

    // Close file descriptors
    close(buff1FD);
    close(buff2FD);
    close(counterFD);
    close(semFD);
    close(regionFD);
    close(resFD);

    // Unmap memory
    munmap(*buff1Ptr, sizeof(int)*NINE*NINE);
    munmap(*buff2Ptr, sizeof(int)*11);
    munmap(*countPtr, sizeof(int));
    munmap(*semaphores, sizeof(sem_t)*2);
    munmap(*region, sizeof(Region)*11);
    munmap(*resourceCount, sizeof(int));

    // Unlink shared memory constructs
    shm_unlink("buffer1");
    shm_unlink("buffer2");
    shm_unlink("counter");
    shm_unlink("semaphores");
    shm_unlink("region");
    shm_unlink("resources");

}
