#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define PORT 12345
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

typedef struct {
    struct sockaddr_in address;
    int sockfd;
    char name[32];
    int is_active; // Flag to track if username is already active
} client_t;

client_t *clients[MAX_CLIENTS]; // Array to hold client connections
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for thread-safe operations

volatile sig_atomic_t running = 1; // Global flag to control server's main loop

// Function prototypes
void handle_sigint(int sig);
void str_trim_lf(char* arr, int length);
void add_client(client_t *cl);
void remove_client(int sockfd);
void send_message(char *s, int exclude_sockfd);
void *handle_client(void *arg);
void print_client_addr(struct sockaddr_in addr);
int is_username_available(char *username);

// Signal handler for SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    printf("\nSIGINT received. Terminating server...\n");
    exit(EXIT_SUCCESS); // Terminate immediately upon SIGINT
}

// Trim newline characters from a string
void str_trim_lf(char* arr, int length) {
    for (int i = 0; i < length; i++) {
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

// Add a client to the clients array
void add_client(client_t *cl) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i]) {
            clients[i] = cl;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Remove a client from the clients array
void remove_client(int sockfd) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i]) {
            if (clients[i]->sockfd == sockfd) {
                clients[i] = NULL;
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Send a message to all clients except the one specified
void send_message(char *s, int exclude_sockfd) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i]) {
            if (clients[i]->sockfd != exclude_sockfd) {
                if (send(clients[i]->sockfd, s, strlen(s), 0) < 0) {
                    perror("ERROR: send");
                    break;
                }
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Check if a username is available
int is_username_available(char *username) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && clients[i]->is_active && strcmp(clients[i]->name, username) == 0) {
            return 0; // Username is already taken
        }
    }
    return 1; // Username is available
}

// Handle communication with a client
void *handle_client(void *arg) {
    char buff_out[BUFFER_SIZE];
    char name[32];
    int leave_flag = 0;
    client_t *cli = (client_t *)arg;

    // Receive client's name and validate
    if (recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) < 2 || strlen(name) >= 32 - 1 || !is_username_available(name)) {
        printf("Invalid username or username already taken\n");
        leave_flag = 1;
    } else {
        strcpy(cli->name, name);
        cli->is_active = 1; // Mark username as active
        sprintf(buff_out, "%s has joined\n", cli->name);
        printf("%s", buff_out);
        send_message(buff_out, cli->sockfd);
    }

    bzero(buff_out, BUFFER_SIZE);

    // Main client handling loop
    while (!leave_flag && running) {
        int receive = recv(cli->sockfd, buff_out, BUFFER_SIZE, 0);
        if (receive > 0) {
            if (strcmp(buff_out, "terminate") == 0) {
                running = 0; // Trigger server termination
                break;
            }
            if (strlen(buff_out) > 0) {
                send_message(buff_out, cli->sockfd);
                str_trim_lf(buff_out, strlen(buff_out));
                printf("%s -> %s\n", buff_out, cli->name);
            }
        } else if (receive == 0 || strcmp(buff_out, "DISCONNECT") == 0) {
            sprintf(buff_out, "%s has left\n", cli->name);
            printf("%s", buff_out);
            send_message(buff_out, cli->sockfd);
            leave_flag = 1;
        } else {
            printf("ERROR: -1\n");
            leave_flag = 1;
        }

        bzero(buff_out, BUFFER_SIZE);
    }

    // Clean up client resources
    close(cli->sockfd);
    remove_client(cli->sockfd);
    free(cli);
    pthread_detach(pthread_self());

    return NULL;
}

// Print client address (IP:Port)
void print_client_addr(struct sockaddr_in addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
    printf("%s:%d", ip, ntohs(addr.sin_port));
}

// Main function
int main() {
    int sockfd, newsockfd;
    struct sockaddr_in server_addr, cli_addr;
    pthread_t tid;

    // Register signal handler for SIGINT (Ctrl+C)
    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        perror("Error registering signal handler");
        return EXIT_FAILURE;
    }

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR: socket");
        return EXIT_FAILURE;
    }

    // Prepare server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket to server address
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR: bind");
        return EXIT_FAILURE;
    }

    // Listen for incoming connections
    listen(sockfd, 10);

    printf("Server started on port %d\n", PORT);

    // Main server loop
    while (running) {
        socklen_t clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);

        // Check if server should exit
        if (!running) {
            break; // Exit loop if SIGINT received
        }

        if (newsockfd < 0) {
            perror("ERROR: accept");
            break; // Exit loop if accept fails
        }

        // Check for max clients
        int clients_count = 0;
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i]) {
                clients_count++;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        if (clients_count == MAX_CLIENTS) {
            printf("Max clients reached. Rejected: ");
            print_client_addr(cli_addr);
            printf(":%d\n", cli_addr.sin_port);
            close(newsockfd);
            continue; // Continue accepting other clients
        }

        // Allocate memory for new client and initialize
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        if (!cli) {
            perror("ERROR: malloc");
            close(newsockfd);
            continue; // Continue accepting other clients
        }
        cli->address = cli_addr;
        cli->sockfd = newsockfd;
        strcpy(cli->name, "Anonymous");
        cli->is_active = 0; // Initialize username as inactive

        // Add client to the queue and handle in a new thread
        add_client(cli);
        pthread_create(&tid, NULL, &handle_client, (void*)cli);

        // Reduce CPU usage
        sleep(1);
    }

    // Clean up remaining clients and close server socket
    close(sockfd);

    return EXIT_SUCCESS;
}
