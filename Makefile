CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic -pthread -lrt

proj2: proj2.o processLogic.o 
	$(CC) $(CFLAGS) $^ -o $@

proj2.o: proj2.c 
	$(CC) $(CFLAGS) -c $< -o $@

processLogic.o: processLogic.c processLogic.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o proj2

run:
	./proj2 20 20 100 100