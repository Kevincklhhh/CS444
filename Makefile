CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99

my_shell: my_shell.o
	$(CC) $(CFLAGS) -o $@ $^

my_shell.o: my_shell.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f my_shell.o my_shell