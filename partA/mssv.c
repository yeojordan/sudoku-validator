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
    sem_wait(&(semaphores[1]));

    *resourceCount = 0;

    // Create child processes for rows
    while( processNum < 11 && pid != 0 )
    {
        signal(SIGCHLD, SIG_IGN);
        pid = fork();

        // Store child's pid in array
        if ( pid > 0)
        {
            printf("Child ID: %d, processNum: %d\n", pid, processNum);
            *resourceCount = *resourceCount + 1;
            printf("Parent's resourceCount: %d\n", *resourceCount);
        }
        processNum++;
    }

    if( pid == 0) // Child process
    {
        childManager(region, semaphores, buff1Ptr, buff2Ptr, countPtr,
                            resourceCount,  processNum, numbers );
    }
    else if ( pid > 0) // Parent process
    {
        parentManager(region, semaphores, countPtr, resourceCount);


        // Clean up shared memory
        cleanMemory(&buff1Ptr, &buff2Ptr, &countPtr, &semaphores,
                       &region, &resourceCount );
    }
    else // Unsuccessful child process creation attempt
    {
        fprintf(stderr, "Unable to create child processes. Please run \"killall mssv\"\n");
    }
}



/******************************************************************************/

void readFile(char* inputFile, int rows, int cols, int (*buffer)[rows][cols])
{
    FILE* inStrm;
    int i, j;

    inStrm = fopen(inputFile, "r");

    if (inStrm == NULL)
    {
        perror("Error opening file for reading\n");
        exit(1);
    }
    printf("%d, %d\n", rows, cols);
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

    fclose(inStrm);


}


void writeFile(Region* region, char* format)
{
    char* filename = "logfile";
    FILE* outFile;
    int val;

    outFile = fopen(filename, "a");
    if (outFile == NULL)
    {
        perror("Error opening file for writing\n");
        exit(1);
    }

    fprintf(outFile, "process ID-%d: %s",region->pid, format);

    fclose(outFile);
}

void resetArray(int numbers[])
{
    for (int i = 0; i < NINE; i++)
    {
        numbers[i] = 0;
    }
}


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

void parentManager(Region *region, sem_t *semaphores, int* countPtr,
                        int* resourceCount)
{
    char *type, *message;
    sem_post(&(semaphores[1]));
    int done = FALSE;
	int position;

    while( !done )
    {
        printf("Parent Waiting for Children\n");
        sem_wait(&(semaphores[1]));
        sem_wait(&(semaphores[0]));
        if ( *resourceCount == 0)
        {
            done = TRUE;
            printf("DONE\n");
        }
        sem_post(&(semaphores[0]));
        sem_post(&(semaphores[1]));

    }


    for(int ii = 0; ii < 11; ii++)
    {



        sem_wait(&(semaphores[0]));//Lock mutex
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

        sem_post(&(semaphores[0]));//Unlock mutex
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







void childManager(Region *region, sem_t *semaphores, int (*buff1Ptr)[NINE][NINE],
                    int *buff2Ptr, int* countPtr, int* resourceCount,
                        int processNum, int *numbers )
{
    char format[500];
    int numValid;

	    if( processNum <= 9)
        {

            // Check rows
            for (int i = 0; i < NINE; i++)
            {

                //row = checkRow( numbers, i, 9, buff1Ptr );
                numbers[((*buff1Ptr)[processNum-1][i])-1]++;

            }

            sem_wait(&(semaphores[0]));//Mutex lock


            region[processNum-1].type = ROW;
            region[processNum-1].positionX = processNum;
            region[processNum-1].pid = getpid();
            region[processNum-1].valid = checkValid(numbers);
            // Update buffer2
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

            buff2Ptr[processNum-1] = numValid;

            *countPtr = *countPtr + numValid;

            sem_post(&(semaphores[0]));

        }
        else if(processNum == 10)
        {
            sprintf(format, "column ");
            // Check cols
	        int validCol = 0;
	        for ( int nn = 0; nn < NINE; nn++)
	        {
	            for(int ii = 0; ii < NINE; ii++)
   	            {
                    numbers[(*buff1Ptr)[ii][nn]-1]++;
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

	        sprintf(format + strlen(format), "are invalid\n");
			sem_wait(&(semaphores[0]));//Empty lock

            region[processNum-1].type = COL;
            region[processNum-1].positionX = validCol;
            region[processNum-1].pid = getpid();
            if(validCol != 9)
            {
                writeFile(&(region[processNum-1]), format);
            }
            // Update buffer2
            numValid = region[processNum-1].positionX;

            buff2Ptr[processNum-1] = validCol;

            // Update counter
            *countPtr = *countPtr + validCol;

            sem_post(&(semaphores[0]));


        }
        else if( processNum == 11)
        {
            sprintf(format, "sub-grid ");
            // Check rows
            int validSub = 0;
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
                    else
                    {
                        sprintf(format+strlen(format), "[%d..%d, %d..%d], ",
                                    jj+1, jj+3, kk+1, kk+3);

                    }
		            resetArray(numbers);
	            }

            }

		    sprintf(format+strlen(format), "is invalid\n");
            sem_wait(&(semaphores[0]));//Mutex lock
            region[processNum-1].type = SUB_REGION;
            region[processNum-1].positionX = validSub;
            region[processNum-1].pid = getpid();

            if(validSub != 9)
            {
                writeFile(&(region[processNum-1]), format);
            }

            buff2Ptr[processNum-1] = validSub;

            // Update counter
            *countPtr = *countPtr + validSub;
            //release locks

            sem_post(&(semaphores[0]));

        }


        // Child signals it is finished by incremented resourceCount
        sem_wait(&(semaphores[1]));
        sem_wait(&(semaphores[0]));
            //printf("I'm done! pid-%d resCount = %d\n", getpid(), (*resourceCount)-1);
            *resourceCount = *resourceCount - 1;
        sem_post(&(semaphores[0]));
        sem_post(&(semaphores[1]));

}

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

void validateUse(int argc, char* argv[])
{


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


void cleanMemory(int (**buff1Ptr)[NINE][NINE], int **buff2Ptr, int** countPtr,
                         sem_t **semaphores, Region **region, int** resourceCount )
{

    // Clean up shared memory
    munmap(*buff1Ptr, sizeof(int)*NINE*NINE);
    munmap(*buff2Ptr, sizeof(int)*11);
    munmap(*countPtr, sizeof(int));
    munmap(*semaphores, sizeof(sem_t)*2);
    munmap(*region, sizeof(Region)*11);
    munmap(*resourceCount, sizeof(int));

    shm_unlink("buffer1");
    shm_unlink("buffer2");
    shm_unlink("counter");
    shm_unlink("semaphores");
    shm_unlink("region");
    shm_unlink("resources");

}
