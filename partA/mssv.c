
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
    int childIDs[11];

    int row, i, j;
    Region* region;
    sem_t semFull, semEmpty, semMutex, *semaphores;
    int processNum = 0;
    int pid;
    int numValid;

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
    while( processNum < 11 && pid != 0 )
    {

        signal(SIGCHLD, SIG_IGN);
        pid = fork();
    
        // Store child's pid in array
        if ( pid > 0)
        {
            printf("Child ID: %d, processNum: %d\n", pid, processNum);
            childIDs[processNum] = pid;    
        }
        processNum++;


    }

    // Child
    if( pid == 0)
    {	
	    if( processNum <= 9)
        {
            char start[] = "row ";
            char format[100];
            char numRow[1];
            char end[] = " is invalid\n";
             
            // Check rows
            for (i = 0; i < NINE; i++)
            {

                //row = checkRow( numbers, i, 9, buff1Ptr );
                numbers[((*buff1Ptr)[processNum-1][i])-1]++;

            }


        //printf("%d\n", *countPtr);
                        //acquire locks
                        sem_wait(&(semaphores[2]));//Empty lock
                        sem_wait(&(semaphores[0]));//Mutex lock
                        

                        region->type = ROW;
                        region->positionX = processNum;
                        region->positionY = 0;
                        region->pid = getpid();
                        region->valid = checkValid(numbers);
                        // Update buffer2
                        numValid = 0;
                        if (region->valid == TRUE)
                        {  
                            numValid = 1;  
                        }
                        else // Write to log file
                        {
                            sprintf(format, "row %d is invalid\n", processNum);
                        printf("Initial format string: %s\n", format); 
                            writeFile(region, format);
                            
                        }
                       
                        buff2Ptr[processNum-1] = numValid;

                        *countPtr = *countPtr + numValid;

                        sem_post(&(semaphores[0]));
                        // Update counter
                        sem_post(&(semaphores[1]));
			//kill(getpid(), SIGTERM);
                        //resetArray(numbers);
        }
  //  }
    // Parent

    // Create child for column
/*    if( pid > 0)
    {
	pid = fork();
	processNum++;
    }
*/    
/*    if( pid == 0)
    {
*/
        else if(processNum == 10)
        {
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
                        //put into subtotal along with PID
    
                        //

		        resetArray(numbers);

                        //release locks
	        }

	    
			            sem_wait(&(semaphores[2]));//Empty lock
                        sem_wait(&(semaphores[0]));//Mutex lock
	   		
                        region->type = COL;
                        region->positionX = validCol;
                        region->pid = getpid();

                        // Update buffer2
                        numValid = region->positionX;
                       
                        buff2Ptr[processNum-1] = validCol;

                        // Update counter
                        *countPtr = *countPtr + validCol;
		                
                        sem_post(&(semaphores[0]));
                        sem_post(&(semaphores[1]));
//		kill(getpid(), SIGTERM);
        }


/*    if (pid > 0)
    {
        pid = fork();
        processNum++;
    }
*/
/*    if (pid == 0)
    {
*/	
        else if( processNum == 11)
        {

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
		            resetArray(numbers);
	            }

            }
    
		
			            sem_wait(&(semaphores[2]));//Empty lock
                        sem_wait(&(semaphores[0]));//Mutex lock
                        //put into subtotal along with PID
                        //


                        region->type = SUB_REGION;
                        region->positionX = validSub;
                        region->pid = getpid();
                        // Update buffer2
                        
                       
                        buff2Ptr[processNum-1] = validSub;

                        // Update counter
                        *countPtr = *countPtr + validSub;
                        //release locks

                        sem_post(&(semaphores[0]));	
                        sem_post(&(semaphores[1]));

		//	kill(getpid(), SIGTERM);
        }
    }





    if ( pid > 0)
    {
	int count=0;
	  //  for(int jj = 0; jj < 11; jj++)
      //  {
       //     waitpid(childIDs[jj], NULL, 0);
            printf("Parent Waiting for Children\n");    
      //  }

        for(int ii = 0; ii < 11; ii++)
        {
            
            char* type;
	    int position;

            //wait
            sem_wait(&(semaphores[1]));//Lock full
            sem_wait(&(semaphores[0]));//Lock mutex

            if (region->type == ROW)
            {
                type = "row";
		position = region->positionX;

		validRegion(&count, region);
		if ( region->valid == TRUE)
		{
			printf("Validation result from process ID-%d: %s %d is valid\n",
                    		region->pid,type, position);
            	}
		else
		{
			printf("Validation result from process ID-%d: %s %d is invalid\n",
				region->pid, type, position);
		}

	    }
            else if (region->type == COL)
            {
                type = "column";
		validRegion(&count, region);
		position = region->positionX;

		printf("Validation result from process ID-%d: %d out of 9 columns are valid\n",region->pid, region->positionX);
            }
            else
            {
                type = "sub-grid";
		validRegion(&count, region);
		position = region->positionX;

		printf("Validation result from process ID-%d: %d out of 9 sub-grids are valid\n",region->pid, region->positionX);
		
            }

            sem_post(&(semaphores[0]));//Unlock mutex
            sem_post(&(semaphores[2]));//Unlock empty
        }

	char* message;

	if (*countPtr == 27)
	{
	     message = "valid";
	}
	else
	{
	    message = "invalid";
	}

	printf("There are %d valid sub-grids, and thus the solution is %s\n", *countPtr, message);
printf("Children are finished\n");
    }

    // Clean up shared memory
    munmap(buff1Ptr, buff1Sz);
    munmap(buff2Ptr, buff2Sz);
    munmap(countPtr, countSz);
    munmap(semaphores, sizeof(sem_t)*3);
    munmap(region, sizeof(Region));
 
    shm_unlink("buffer1");
    shm_unlink("buffer2");
    shm_unlink("counter");
    shm_unlink("semaphores");
    shm_unlink("region");
}



/******************************************************************************/


void validRegion(int* count, Region* region)
{
    if ( region->type == ROW )
    {
	if ( region->valid == TRUE)
	{
		*count = *count + 1;
	} 	
    }
    else
    {
	*count = *count + region->positionX;
    }


	
}


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


int readFile(char* inputFile, int rows, int cols, int (*buffer)[rows][cols])
{
    FILE* inStrm;
    int i, j;

    inStrm = fopen(inputFile, "r");

    if (inStrm == NULL)
    {
        perror("Error opening file for reading\n");
        return -1;
    }
    printf("%d, %d\n", rows, cols);
    for( i = 0; i < rows; i++ )
    {

        for ( j = 0; j < cols; j++ )
        {
            fscanf( inStrm, "%d", &(*(buffer))[i][j] );

                    printf("%d ", (*buffer)[i][j]);
        }
        printf("\n");
    }

    fclose(inStrm);

    return 0;
}


void writeFile(Region* region, char* format)
{
    char* filename = "logfile";
    FILE* outFile;
    int val;

    outFile = fopen(filename, "w");
    if (outFile == NULL)
    {
        perror("Error opening file for writing\n");
        exit(1);
    }

    fprintf(outFile, "process ID-%d: %s\n",region->pid, format);   
            
}
