#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 12345
#define BUFFER_SIZE 1024

char name[32]; // Client's name
int sockfd = 0; // Socket file descriptor
char buff_out[BUFFER_SIZE]; // Outgoing message buffer
char buff_in[BUFFER_SIZE]; // Incoming message buffer
pthread_t send_msg_thread; // Thread for sending messages
pthread_t recv_msg_thread; // Thread for receiving messages

// Trim newline characters from a string
void str_trim_lf(char* arr, int length) {
    for (int i = 0; i < length; i++) {
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

// Thread function for sending messages to the server
void *send_msg_handler(void *arg) {
    while (1) {
        fgets(buff_out, BUFFER_SIZE, stdin); // Read user input
        str_trim_lf(buff_out, strlen(buff_out)); // Trim newline

        if (strcmp(buff_out, "DISCONNECT") == 0) {
            break; // Exit loop on "DISCONNECT" command
        } else {
            send(sockfd, buff_out, strlen(buff_out), 0); // Send message to server
        }

        bzero(buff_out, BUFFER_SIZE); // Clear buffer
    }
    close(sockfd); // Close socket
    exit(0); // Exit thread
}

// Thread function for receiving messages from the server
void *recv_msg_handler(void *arg) {
    while (1) {
        int receive = recv(sockfd, buff_in, BUFFER_SIZE, 0); // Receive message
        if (receive > 0) {
            printf("%s\n", buff_in); // Print received message
        } else if (receive == 0) {
            break; // Exit loop on server disconnect
        }
        memset(buff_in, 0, sizeof(buff_in)); // Clear buffer
    }
    return NULL; // Exit thread
}

// Main function
int main() {
    struct sockaddr_in server_addr;

    // Socket settings
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create socket
    if (sockfd < 0) {
        printf("ERROR: socket\n");
        return EXIT_FAILURE;
    }

    // Server address configuration
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address
    server_addr.sin_port = htons(PORT); // Server port

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        printf("ERROR: connect\n");
        return EXIT_FAILURE;
    }

    // Get the name of the client
    printf("Enter your name: ");
    fgets(name, 32, stdin); // Read user input
    str_trim_lf(name, strlen(name)); // Trim newline

    if (strlen(name) < 2 || strlen(name) >= 32 - 1) {
        printf("Name must be more than one and less than thirty characters.\n");
        return EXIT_FAILURE;
    }

    // Send the name to the server
    send(sockfd, name, 32, 0); // Send name to server

    printf("=== WELCOME TO THE CHATROOM ===\n");

    // Create threads for sending and receiving messages
    pthread_create(&send_msg_thread, NULL, send_msg_handler, NULL);
    pthread_create(&recv_msg_thread, NULL, recv_msg_handler, NULL);

    // Wait for threads to complete
    pthread_join(send_msg_thread, NULL);
    pthread_join(recv_msg_thread, NULL);

    close(sockfd); // Close socket

    return EXIT_SUCCESS; // Exit program
}
