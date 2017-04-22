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

    pthread_mutex_init(&mutex, NULL); 
    pthread_cond_init(&use, NULL);
    inUse = 0;
     
    // Create threads
    for(int i = 0; i < NUMTHREADS; i++)
    {
       if ( i < NINE)
       {
           regions[i].type = ROW;
       }
       else if( i == NINE)
       {
           regions[i].type = COL;
       }
       else
       {
           regions[i].type = SUB_GRID;    
       }
       
       regions[i].position = i;
       resetArray(regions[i].numbers);
       pthread_create(&(threads[i]), NULL, childManager, &(regions[i]));
       inUse++;
    }
    
    parentManager(threads);
    
    cleanMemory();

}

/******************************************************************************/

void initMemory(int*** buff1, int** buff2, int** counter, Region** regions)
{
    *buff1 = (int**) malloc(sizeof(int*)* NINE);
    for (int i = 0; i < NINE; i++)
    {
        (*buff1)[i] = (int*) malloc(sizeof(int)* NINE);   
    }
    *buff2 = (int*) malloc(sizeof(int)* NUMTHREADS);
    *counter = (int*) malloc(sizeof(int));
    *regions = (Region*) malloc(sizeof(Region)* NUMTHREADS);
}



 
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




void readFile(char* inputFile, int rows, int cols, int***buffer )
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

    fprintf(outFile, "process ID-%d: %s",region->tid, format);

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

void parentManager(pthread_t threads[] )
{
    char *type, *message;
    int done = FALSE;
	int position;

    pthread_mutex_lock(&mutex);
    while ( inUse > 0 ) // While children executing wait
    {
        pthread_cond_wait(&use, &mutex);
    }

    pthread_cond_signal(&use);
    pthread_mutex_unlock(&mutex);
    
   
    printf("CHILDREN DONE\n");

    for(int ii = 0; ii < NUMTHREADS; ii++)
    {    
        pthread_mutex_lock(&mutex);
        
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







void* childManager(void* args )
{
    char format[500];
    int numValid;
    Region* region = ((Region*)(args));
    int threadNum = region->position;

	    if( region->type == ROW )
        {   
            
            // Check rows
            for (int i = 0; i < NINE; i++)
            {
                // Position starts at 0
                region->numbers[((buff1)[threadNum][i])-1]++;
            }
            
            sleep(maxDelay);
            pthread_mutex_lock(&mutex); // Lock mutex
            
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

            buff2[threadNum] = numValid;

            *counter = *counter + numValid;
            
            pthread_mutex_unlock(&mutex); // Unlock mutex

        }
        else if( region->type == COL )
        {
            sprintf(format, "column ");
            // Check cols
	        int validCol = 0;
	        for ( int nn = 0; nn < NINE; nn++)
	        {
	            for(int ii = 0; ii < NINE; ii++)
   	            {
                    region->numbers[((buff1)[ii][nn])-1]++;
	            }

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
            
            region->count = validCol;
            region->tid = pthread_self();
            if(validCol != 9)
            {
                writeFile((region), format);
            }

            // Update buffer2
            numValid = region->count;

            buff2[threadNum] = validCol;

            // Update counter
            *counter = *counter + validCol;

            pthread_mutex_unlock(&mutex); // Unlock mutex
        }
        else if( region->type == SUB_GRID )
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
			                region->numbers[((buff1)[ll][mm])-1]++;

		                }
		            }

	    	        if ( checkValid(region->numbers) == TRUE )
	    	        {
		                validSub++;
	   	            }
                    else
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
            region->count= validSub;
            region->tid = pthread_self();

            if(validSub != 9)
            {
                writeFile((region), format);
            }

            buff2[threadNum] = validSub;

            // Update counter
            *counter = *counter + validSub;

            pthread_mutex_unlock(&mutex); // Unlock mutex

        }

        // Child signals it is finished by incremented resourceCount
        pthread_mutex_lock(&mutex);
        
        while( inUse == 0)
        {
            pthread_cond_wait(&use, &mutex);    
        }
        
        inUse--;
        if (inUse == 0)
        {
            pthread_cond_signal(&use);    
        }
        
        pthread_mutex_unlock(&mutex);
        pthread_detach(pthread_self());
       
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

