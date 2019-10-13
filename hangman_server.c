
// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>
#include <arpa/inet.h>
static int count = 0;

char* randomWord(int* size) {
    char ch;
    FILE *fp;
    char* file_name = "name.txt";
    fp = fopen(file_name, "r");
    if(fp==NULL) {
        perror("Error while opening the file\n");
        exit(EXIT_FAILURE);
    }

    char* buffer = NULL;
    size_t len = 0;
    ssize_t read;
    int count = 0; 
    srand(time(0));
    int num = (rand() % 15) + 1;
    while((read = getline(&buffer, &len, fp)) != -1) {
        count++;
        if (count == num) {
            break;
        }
    }
    fclose(fp);

    *size = read - 1;

    char* result = malloc(sizeof(char) * (read));
    for(int i = 0; i < read; i++) {
        result[i] = buffer[i];
    }
    result[read-1] = '\0';
    return result;
}

char* blank_word(int size) {
    char* result = malloc(sizeof(char)*size+1);
    for(int i = 0; i < size; i++) {
        result[i] = '_';
    }
    result[size] = '\0';
    return result;
}

int fill_word(char* p_word, char* word, char c, int n, int* ccount) {

    int correct = 0;

    for(int i = 0; i < n; i++) {
        if(word[i] == c) {
            correct = 1;
            *ccount += 1;
            p_word[i] = c;
        }
    }

    return correct;
}

char* incorrect_list(char* inc_list, char c, int n) {
    //n is the number of items in our new list
    char* temp = inc_list;
    inc_list = malloc(sizeof(char) * (n + 1));
    for(int i = 0; i < n-1; i++) {
        inc_list[i] = temp[i];
    }
    
    inc_list[n-1] = c;
    inc_list[n] = '\0';

    // if(temp != NULL) {
    //     free(temp);
    // }


    return inc_list;
}

void* handle_client(void* args) {
    int valread;
    char buffer[1024] = {0}; //For receiving our client's data

    //Sets up our count so we know how many we are at
    count++;
    char str_count[sizeof(int) * 4 + 1];
    sprintf(str_count, "%d", count);       //TODO: remove later. This is jsut there for print purposes

    char* word = NULL;      //dogmano
    int word_size;          //7
    char* p_word = NULL;    //"_ _ _ _ _ _ _ ";
    int inc_count = 0;
    int correct_count = 0;
    char* inc_list = NULL;
    int max_guess = 6;

    word = randomWord(&word_size);
    p_word = blank_word(word_size);

    printf("Chosen Word: %s\n", word);

    //Reading first value from client. Should be expecting a 0. 
    memset(buffer, 0, 1024);
    valread = read(*(int*)args , buffer, 1);


    //Reading value from client
    // memset(buffer, 0, 1024);
    // valread = read(*(int*)args , buffer, 1024);
    

    //Ending client if our msg is not 0 here. This is in reposne to a "y" to continue from client
    if(buffer[0] != 0) {
        close(*(int*)args);
        count--;
        pthread_exit(0);
    }

    while(1) {

        int msg_size = 3 + word_size + inc_count;
        char* game_msg = malloc(sizeof(char) * (msg_size));
        game_msg[0] = 0;
        game_msg[1] = word_size;
        game_msg[2] = inc_count;
        for(int i = 0; i < word_size; i++) {
            game_msg[3+i] = p_word[i];
        }
        for(int i = 0; i < inc_count; i++) {
            game_msg[3+word_size+i] = inc_list[i];
        }
        
        send(*(int*)args, game_msg, msg_size, 0);
        free(game_msg);

        memset(buffer, 0, 1024);
        valread = read(*(int*)args , buffer, 1);
        if(buffer[0] > 5) {
            count--;
            pthread_exit(0);
        }
        int read_msg_len = buffer[0];
        

        memset(buffer, 0, 1024);
        valread = read(*(int*)args , buffer, read_msg_len);

        int correct = fill_word(p_word, word, buffer[0], word_size, &correct_count);
        if(correct) {
            if(correct_count == word_size) {
                //You Win
                char* win = "You Win!";
                char* win_msg = malloc(sizeof(char) * 9);
                win_msg[0] = 8;
                for(int i = 0; i < 8; i++) {
                    win_msg[i+1] = win[i];
                }
                send(*(int*)args, win_msg, 9, 0);
                //Send back answer
                char* answer = malloc(sizeof(char) * (1 + word_size));
                answer[0] = (char)word_size;
                for(int i = 0; i < word_size; i++) {
                    answer[1+i] = word[i];
                }
                send(*(int*)args, answer, 1+word_size, 0);
                //close connection
                close(*(int*)args);
                //Decrement count
                count--;
                //pthread_exit(0)
                pthread_exit(0);
            }
        } else {
            inc_count++;
            inc_list = incorrect_list(inc_list, buffer[0], inc_count);
            if(inc_count == max_guess) {
                //You lose
                char* lose = "You Lose!";
                char* lose_msg = malloc(sizeof(char) * 10);
                lose_msg[0] = 9;
                for(int i = 0; i < 9; i++) {
                    lose_msg[i+1] = lose[i];
                }
                send(*(int*)args, lose_msg, 10, 0);
                //Send back shit
                char* answer = malloc(sizeof(char) * (1 + word_size));
                answer[0] = word_size;
                printf("SIZE: %d\n", answer[0]);
                for(int i = 0; i < word_size; i++) {
                    answer[1+i] = word[i];
                }
                send(*(int*)args, answer, 1+word_size, 0);
                printf("hello\n");
                close(*(int*)args);
                //Decrement Count
                count--;
                //pthread_exit(0)
                pthread_exit(0);

            }
        }

    }
}


int main(int argc, char const *argv[]) 
{ 
    int server_fd, new_socket; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char buffer[1024] = {0}; 
    
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( atoi(argv[2]) ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 


    while(1) {
        
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) { 
            perror("accept"); 
            exit(EXIT_FAILURE); 
        } 

        if(count < 3) {
            
            pthread_t* tid = malloc(sizeof(pthread_t));
            int* socket = malloc(sizeof(int));
            *socket = new_socket;
            char* welcome = "Welcome to the Game";
            send(new_socket, welcome, strlen(welcome) , 0 ); 
            pthread_create(tid, NULL, &handle_client, socket);
        } else {
            char* overload = "server-overloaded";
            send(new_socket, overload, strlen(overload) , 0 ); 
            close(new_socket);
        }

    }
    printf("Past while loop\n");
    
    return 0; 
} 
