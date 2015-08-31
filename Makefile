all:
	gcc -g -o golcli golcli.c -lcurses
clean:
	rm golcli

