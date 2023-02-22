CC = gcc
CFLAGS = -Wall -Wextra

my_shell: my_shell.o
	$(CC) $(CFLAGS) -o $@ $^

my_shell.o: my_shell.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f my_shell.o my_shell