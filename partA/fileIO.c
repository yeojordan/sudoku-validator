
#include "fileIO.h"


int readFile(char* inputFile, int rows, int cols, int (*buffer)[rows][cols])
{
    FILE* inStrm;
    int rowOff, i, j, val;

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

// write to logfile
// int writeFile( )
