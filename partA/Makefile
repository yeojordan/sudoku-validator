# Makefile For Sudoku Solution Validator
# COMP2006 Assignment
# Last Modified: 12/04/17
# Jordan Yeo - 17727626

# MAKE VARIABLES
EXEC1 = mssv
OBJ1 = mssv.o 
CFLAGS = -std=c99 -pthread -D _XOPEN_SOURCE=500 -lrt
CC = gcc


# RULES + DEPENDENCIES
$(EXEC1) : $(OBJ1)
	$(CC) $(OBJ1) -o $(EXEC1) $(CFLAGS)

mssv.o : mssv.c mssv.h #fileIO.h
	$(CC) -c mssv.c $(CFLAGS)

#fileIO.o : fileIO.c fileIO.h
#	$(CC) -c fileIO.c $(CFLAGS)

clean:
	rm -f $(EXEC1) $(OBJ1) logfile

run:
	./$(EXEC1) ../testFiles/specTest.txt 10
