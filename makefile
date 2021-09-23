shell : shell.o operations.o parser.o
	gcc shell.o operations.o parser.o -o shell

shell.o: shell.c operations.h
	gcc -c shell.c

operations.o: operations.c operations.h parser.h
	gcc -c operations.c

parser.o: parser.c parser.h
	gcc -c parser.c
	
clean:
	rm *.o

wipe:
	rm *.o
	rm shell
	clear
