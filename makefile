assembler: clean main.o structLabels.o 1st_run.o 2nd_run.o utilities.o
	gcc -g -ansi -Wall -pedantic  main.o 1st_run.o 2nd_run.o utilities.o structLabels.o -lm -o assembler

main.o: main.c prototypes.h assembler.h externs.h structs.h utilities.h
	gcc -g -c -ansi -Wall -pedantic  main.c -o main.o

structLabels.o: structLabels.c prototypes.h assembler.h externs.h structs.h utilities.h
	gcc -g -c -ansi -Wall -pedantic  structLabels.c -o structLabels.o

utilities.o: utilities.c prototypes.h assembler.h externs.h structs.h utilities.h
	gcc -g -c -ansi -Wall -pedantic  utilities.c -o utilities.o

2nd_run.o: 2nd_run.c prototypes.h assembler.h externs.h structs.h
	gcc -g -c -ansi -Wall -pedantic  2nd_run.c -o 2nd_run.o

1st_run.o: 1st_run.c prototypes.h assembler.h externs.h structs.h
	gcc -g -c -ansi -Wall -pedantic  1st_run.c -o 1st_run.o

clean:
	-rm -f *.o