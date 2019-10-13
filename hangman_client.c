
// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <ctype.h>
#include <stdlib.h> 
#include <netinet/in.h> 
#include <signal.h>
#include <error.h>
static int sock = 0;

char *strlwr(char *str)
{
  unsigned char *p = (unsigned char *)str;

  while (*p) {
     *p = tolower((unsigned char)*p);
      p++;
  }

  return str;
}

void sigint_handler(int sig) {
    char* close_socket = malloc(sizeof(char) * 1);
    close_socket[0] = 10;
    send(sock , close_socket, 1 , 0 ); 
    close(sock);
    exit(0);
}

   
int main(int argc, char const *argv[]) 
{ 
    
    int valread;              //our sock is our socket id? valread idk
    struct sockaddr_in serv_addr;       //Our serv_addr to hold our PORT and HOST
    char *hello = "Hello from client";  //Our message
    char buffer[1024] = {0};            //Our receive buffer
    char input[80]; 
    int n;
    struct sigaction sa;


    sa.sa_handler = sigint_handler;
    if(sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Sigaction failed");
    }

    //Set up socket with TCP
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
    

    //Set up HOST and PORT into our serv_addr
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2])); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 

    printf("Ready to start game? (y/n): ");
    while(1) {
        //Gets user input and tries to do stuff with it
        memset(input, 0, 80);
        n = 0;
        while ((input[n++] = getchar()) != '\n'); 
        if (!strncmp(input, "y\n", 2)) {
            //Connecting to the server socket. Combines socket with serv_addr
            if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
            { 
                printf("\nConnection Failed \n"); 
                return -1; 
            } 

            //Wait on the server to send an okay or to server overload
            memset(buffer, 0, 1024);
            valread = read( sock , buffer, 1024); 
            printf("%s\n", buffer);     //TODO: move this into server-overloaded later
            if (!strncmp(buffer, "server-overloaded", 17)) {
                return 0;
            }

            //Start the game. Send 0 to server
            char* zero = malloc(sizeof(char) * 1);
            zero[0] = 0;
            send(sock , zero, 1 , 0 ); 

            //All our Message Receive and Handling will be down here
            while(1) {
                memset(buffer, 0, 1024);
                valread = read( sock , buffer, 1); 
                if(buffer[0] == 0) {    //In this case, we are still playing hangman
                    memset(buffer, 0, 1024);
                    valread = read(sock, buffer, 2);
                    int word_len = buffer[0];
                    int num_inc = buffer[1];
                    char* hang_word = malloc(sizeof(char) * (word_len*2));
                    char* inc_list = malloc(sizeof(char) * (num_inc*2));
                    printf("word_len: %d\n", word_len);
                    printf("num_inc: %d\n", num_inc);

                    memset(buffer, 0, 1024);
                    valread = read(sock, buffer, word_len + num_inc);

                    for(int i = 0; i < word_len; i++) {
                        hang_word[i*2] = buffer[i];
                        hang_word[i*2+1] = ' ';
                    }
                    hang_word[word_len*2] = '\0';

                    for(int i = 0; i < num_inc; i++) {
                        inc_list[i*2] = buffer[word_len+i];
                        inc_list[i*2+1] = ' ';
                    }
                    inc_list[num_inc*2] = '\0';
                    printf("%s\n", hang_word);
                    printf("Incorrect Guesses: %s\n", inc_list);

                    while(1) {
                        memset(input, 0, 80);
                        n = 0;
                        printf("Letter to guess: ");
                        while ((input[n++] = getchar()) != '\n'); 
                        strlwr(input);
                        if(n == 2 && input[0] >= 97 && input[0] <=122 ) {
                            char* letter_msg = malloc(sizeof(char) * 2);
                            letter_msg[0] = 1;
                            letter_msg[1] = input[0];
                            send(sock, letter_msg, 2, 0);
                            break;
                        } else {
                            printf("Error! Pelase guess one letter\n");
                        }
                    }

                } else {                //Whatever comes out of this should just get printed. The other end will close the socket. 
                    int msg_len = (int)buffer[0];
                    memset(buffer, 0, 1024); 
                    valread = read(sock, buffer, msg_len);
                    printf("NUM: %d\n", msg_len);
                    
                    char* res_out = malloc(sizeof(char) * (msg_len + 1));
                    for(int i = 0; i < msg_len; i++) {
                        res_out[i] = buffer[i];
                    }
                    res_out[msg_len-1] = '\0';

                    memset(buffer, 0, 1024);
                    valread = read( sock , buffer, 1); 
                    int ans_len = (int)buffer[0]; 

                    memset(buffer, 0, 1024);
                    valread = read( sock , buffer, ans_len); 

                    printf("ans_len: %d\n", ans_len);
                    char* answer = malloc(sizeof(char) * (ans_len*2));
                    printf("ans_len: %d\n", ans_len);
                    for(int i = 0; i < ans_len; i++) {
                        answer[i*2] = buffer[i];                        
                        answer[i*2 + 1] = ' ';
                    }
                    answer[ans_len*2] = '\0';
                    

                    printf("The word was: %s\n", answer);
                    printf("%s\n", res_out);
                    printf("Game Over!\n");
                    return 0;
                }
            }

        } else if (!strncmp(input, "n\n", 2)) {
            char* sd = "shutdown";
            char* close_socket = malloc(sizeof(char)*(9));
            close_socket[0] = 8;
            for(int i = 0; i < 8; i++) {
                close_socket[i+1] = sd[i];
            }

            send(sock, close_socket, 9, 0);
            close(sock);
            return 0;

        } else {
            printf("Invalid Input, Choose 'y' or 'n': ");
        }

    }

    while(1) {
        bzero(input, sizeof(input)); 
        printf("Enter the string : "); 
        n = 0; 
        while ((input[n++] = getchar()) != '\n'); 

        send(sock , input, n , 0 ); 
        printf("Hello message sent\n"); 
        memset(buffer, 0, 1024);
        valread = read( sock , buffer, 1024); 
        printf("stuff: %d\n", valread);
        printf("%s\n",buffer ); 

        if (!strncmp(input, "end", 3)) {
            break;
        }
    }

    return 0; 
} 
