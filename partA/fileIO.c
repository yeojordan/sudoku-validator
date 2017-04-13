
#include "fileIO.h"


int readFile(char* inputFile, int* buffer)
{
    FILE* inStrm;
    int rowOff, i, j;

    inStrm = fopen(inputFile, "r");

    if (inStrm == NULL)
    {
        perror("Error opening file for reading\n");
        return -1;
    }

    for( i = 0; i < 9; i++ )
    {
        rowOff = i * 9;
        for ( j = 0; j < 9; j++ )
        {
            fscanf( inStrm, "%d", &buffer[rowOff + j] );
                    printf("%d ", buffer[rowOff + j]);
        }
        printf("\n");
    }

    fclose(inStrm);

    return 0;
}
