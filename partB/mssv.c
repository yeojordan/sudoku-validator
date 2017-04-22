#include "mssv.h"

pthread_mutex_t mutex; // Mutex
pthread_cond_t use;  // condition for if the global variable is in use

int **buff1, *buff2, *counter, maxDelay, inUse;
Region *regions;

int main (int argc, char* argv[])
{
    // Validate command line parameters
    validateUse(argc, argv);

    // Rename command line parameters
    char* inputFile = argv[1];
    maxDelay = atoi(argv[2]);

    // Variables
    pthread_t threads[11];

    // Generate random maxDelay
    srand((unsigned) time(NULL));
    maxDelay = rand() % maxDelay;

    // Allocate  memory
    initMemory( &buff1, &buff2, &counter, &regions);

    *counter = 0;
    // Read input file
    readFile(inputFile, NINE, NINE, &buff1);

    // Initialise mutex and condition
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&use, NULL);
    inUse = 0;

    // Create threads
    for(int i = 0; i < NUMTHREADS; i++)
    {
       if ( i < NINE) // Initialise region struct for row threads
       {
           regions[i].type = ROW;
       }
       else if( i == NINE) // Initialise region struct for columns thread
       {
           regions[i].type = COL;
       }
       else // Initialise region struct for sub-grids thread
       {
           regions[i].type = SUB_GRID;
       }

       regions[i].position = i;
       resetArray(regions[i].numbers);
       // Create thread
       pthread_create(&(threads[i]), NULL, childManager, &(regions[i]));
       inUse++;
    }

    parentManager(threads); // Parent logic

    cleanMemory(); // Clean up malloc'd memory

}

/******************************************************************************/

/**
 * Initalise memory constructs
 * @param buff1   buffer1 2D array
 * @param buff2   buffer2 1D array
 * @param counter counter variable
 * @param regions Region struct 1D array
 */
void initMemory(int*** buff1, int** buff2, int** counter, Region** regions)
{
    // Initialise 
    *buff1 = (int**) malloc(sizeof(int*)* NINE);
    for (int i = 0; i < NINE; i++)
    {
        (*buff1)[i] = (int*) malloc(sizeof(int)* NINE);
    }
    *buff2 = (int*) malloc(sizeof(int)* NUMTHREADS);
    *counter = (int*) malloc(sizeof(int));
    *regions = (Region*) malloc(sizeof(Region)* NUMTHREADS);
}


/**
 * Free malloc'd memory and destroy mutex and conditions
 */
void cleanMemory()
{
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&use);
    for (int i = 0; i < NINE; i++)
    {
        free(buff1[i]);
    }
    free(buff1);
    free(buff2);
    free(counter);
    free(regions);
}


/**
 * Read the contents of the input file passed as a command line argument
 * @param inputFile File to be read
 * @param rows      Number of rows in matrix
 * @param cols      Number of columns in matrix
 * @param buffer    Matrix to store contents of input file
 */
void readFile(char* inputFile, int rows, int cols, int***buffer )
{
    FILE* inStrm;
    int i, j;

    inStrm = fopen(inputFile, "r"); // Open file for reading

    if (inStrm == NULL) // Check file opened correctly
    {
        perror("Error opening file for reading\n");
        exit(1);
    }
    printf("%d, %d\n", rows, cols);

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

    fprintf(outFile, "process ID-%d: %s",region->tid, format);

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
 * Handles the routine for the parent. Outputs the result to the screen
 * @param threads ID of child threads
 */
void parentManager(pthread_t threads[] )
{
    char *type, *message;
    int done = FALSE;
	int position;

    pthread_mutex_lock(&mutex); // Lock mutex
    while ( inUse > 0 ) // Wait while children are executing
    {
        pthread_cond_wait(&use, &mutex);
    }

    pthread_cond_signal(&use);
    pthread_mutex_unlock(&mutex); // Unlock mutex


    printf("CHILDREN DONE\n");

    for(int ii = 0; ii < NUMTHREADS; ii++)
    {
        pthread_mutex_lock(&mutex); // Lock mutex

        if (regions[ii].type == ROW)
        {
            type = "row";
		    position = regions[ii].position;
            if ( regions[ii].valid == TRUE)
	        {
	            printf("Validation result from thread ID%d: %s %d is valid\n",
                                      	regions[ii].tid,type, position+1);
          	}
	      	else
		    {
        		printf("Validation result from thread ID%d: %s %d is invalid\n",
		                   		regions[ii].tid, type, position+1);
	        }

    	    }
        else if (regions[ii].type == COL)
        {
            type = "column";
      		printf("Validation result from thread ID%d: %d out of 9 columns are valid\n",regions[ii].tid, regions[ii].count);
        }
        else
        {
            type = "sub-grid";
	        printf("Validation result from process ID%d: %d out of 9 sub-grids are valid\n",regions[ii].tid, regions[ii].count);

        }

        pthread_mutex_unlock(&mutex);
    }


	if (*counter == 27)
	{
	     message = "valid";
	}
	else
	{
	    message = "invalid";
	}

	printf("There are %d valid sub-grids, and thus the solution is %s\n", *counter, message);

}

/**
 * Routine for child threads. Check the validity of sub region.
 * @param args Void pointer to Region struct for the child
 */
void* childManager(void* args )
{
    char format[500];
    int numValid;
    Region* region = ((Region*)(args));
    int threadNum = region->position;

	    if( region->type == ROW ) // Check row in buffer1
        {

            // Check rows
            for (int i = 0; i < NINE; i++)
            {
                // Update numbers array
                region->numbers[((buff1)[threadNum][i])-1]++;
            }

            sleep(maxDelay); // Sleep
            pthread_mutex_lock(&mutex); // Lock mutex

            // Update region struct
            region->tid = pthread_self();
            region->valid = checkValid(region->numbers);

            // Update buffer2
            numValid = 0;
            if (region->valid == TRUE)
            {
                numValid = 1;
                region->count = numValid;
            }
            else // Write to log file
            {
                region->count = numValid;
                sprintf(format, "row %d is invalid\n", threadNum+1);
                writeFile((region), format);
            }

            buff2[threadNum] = numValid; // Update buffer2

            *counter = *counter + numValid; // Update counter

            pthread_mutex_unlock(&mutex); // Unlock mutex

        }
        else if( region->type == COL ) // Check all columns
        {
            sprintf(format, "column ");

	        int validCol = 0;
	        for ( int nn = 0; nn < NINE; nn++) // Iterate through each column
	        {
	            for(int ii = 0; ii < NINE; ii++) // Iterate through each row
   	            {
                    // Update numbers array
                    region->numbers[((buff1)[ii][nn])-1]++;
	            }

                // Check if the column is valid
                if ( checkValid( region->numbers) == TRUE )
	            {
		            validCol++;
	            }
                else
                {
                    sprintf(format + strlen(format), "%d, ", nn+1);
                }

		        resetArray(region->numbers);

	        }

            sleep(maxDelay);
	        sprintf(format + strlen(format), "are invalid\n");
            pthread_mutex_lock(&mutex); // Lock mutex

            // Update region struct
            region->count = validCol;
            region->tid = pthread_self();
            if(validCol != 9)
            {
                writeFile((region), format);
            }

            numValid = region->count;

            buff2[threadNum] = validCol; // Update buffer2

            *counter = *counter + validCol; // Update counter

            pthread_mutex_unlock(&mutex); // Unlock mutex
        }
        else if( region->type == SUB_GRID ) // Check all sub-grids
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
                            // Update numbers array
			                region->numbers[((buff1)[ll][mm])-1]++;
		                }
		            }

	    	        if ( checkValid(region->numbers) == TRUE )
	    	        {
		                validSub++;
	   	            }
                    else // Update string for log file
                    {
                        sprintf(format+strlen(format), "[%d..%d, %d..%d], ",
                                    jj+1, jj+3, kk+1, kk+3);

                    }
		            resetArray(region->numbers);
	            }

            }

            sleep(maxDelay);
		    sprintf(format+strlen(format), "is invalid\n");
            pthread_mutex_lock(&mutex); // Lock mutex

            // Update region struct
            region->count= validSub;
            region->tid = pthread_self();

            if(validSub != 9)
            {
                writeFile((region), format);
            }

            buff2[threadNum] = validSub; // Update buffer2

            *counter = *counter + validSub; // Update counter

            pthread_mutex_unlock(&mutex); // Unlock mutex

        }

        // Child signals it is finished by incremented resourceCount
        pthread_mutex_lock(&mutex); // Lock mutex

        while( inUse == 0)
        {
            pthread_cond_wait(&use, &mutex);
        }
        inUse--; // Decrease count of child processes running
        if (inUse == 0)
        {
            pthread_cond_signal(&use);
        }
        pthread_mutex_unlock(&mutex); // Unlock mutex
        pthread_detach(pthread_self()); // Release resources

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
