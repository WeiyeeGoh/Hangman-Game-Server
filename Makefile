all: hangman_client hangman_server

hangman_server: hangman_server.c
	gcc -o hangman_server hangman_server.c -pthread

hangman_client: hangman_client.c
	gcc -o hangman_client hangman_client.c -pthread

clean: 
	rm -f hangman_client hangman_server *.o