#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define USERNAME_SIZE 50

// Struct để lưu thông tin client
typedef struct {
    int sock;
    int logged_in;
    char login_name[USERNAME_SIZE];
    struct sockaddr_in address;
} Client;

// Mảng lưu các client đang kết nối
Client *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Thêm client vào danh sách
void add_client(Client *client) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == NULL) {
            clients[i] = client;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Xóa client khỏi danh sách
void remove_client(int sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->sock == sock) {
            clients[i] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Gửi message tới client
void send_message(int sock, const char *message) {
    if (send(sock, message, strlen(message), 0) < 0) {
        perror("Send failed");
    }
}

// Broadcast message tới tất cả clients (trừ sender)
void broadcast_message(const char *message, int sender_sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->sock != sender_sock) {
            send_message(clients[i]->sock, message);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Xử lý client
void handle_client(Client *client) {
    char buffer[BUFFER_SIZE];
    int read_size;

    // Gửi welcome message
    send_message(client->sock, "Welcome to BattleShip Server!\n");
    send_message(client->sock, "Commands: LOGIN <username>, LOGOUT, MSG <message>, QUIT\n");

    while ((read_size = recv(client->sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[read_size] = '\0';
        
        // Xóa newline cuối cùng
        char *newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';

        printf("Received from client %d: %s\n", client->sock, buffer);

        // Parse command
        if (strncmp(buffer, "LOGIN ", 6) == 0) {
            char *username = buffer + 6;
            strncpy(client->login_name, username, USERNAME_SIZE - 1);
            client->logged_in = 1;
            
            char response[BUFFER_SIZE];
            snprintf(response, BUFFER_SIZE, "LOGIN_SUCCESS: Welcome %s!\n", username);
            send_message(client->sock, response);
            
            // Broadcast tới các client khác
            snprintf(response, BUFFER_SIZE, "NOTIFICATION: %s has joined the game\n", username);
            broadcast_message(response, client->sock);
            
            printf("Client %d logged in as: %s\n", client->sock, username);
        }
        else if (strcmp(buffer, "LOGOUT") == 0) {
            if (client->logged_in) {
                char response[BUFFER_SIZE];
                snprintf(response, BUFFER_SIZE, "NOTIFICATION: %s has left the game\n", client->login_name);
                broadcast_message(response, client->sock);
                
                client->logged_in = 0;
                strcpy(client->login_name, "");
                send_message(client->sock, "LOGOUT_SUCCESS\n");
            } else {
                send_message(client->sock, "ERROR: Not logged in\n");
            }
        }
        else if (strncmp(buffer, "MSG ", 4) == 0) {
            if (client->logged_in) {
                char *message = buffer + 4;
                char response[BUFFER_SIZE];
                snprintf(response, BUFFER_SIZE, "MESSAGE: %s: %s\n", client->login_name, message);
                broadcast_message(response, client->sock);
                send_message(client->sock, "MESSAGE_SENT\n");
            } else {
                send_message(client->sock, "ERROR: Please login first\n");
            }
        }
        else if (strcmp(buffer, "QUIT") == 0) {
            send_message(client->sock, "BYE\n");
            break;
        }
        else {
            // Echo back cho testing
            char response[BUFFER_SIZE];
            snprintf(response, BUFFER_SIZE, "ECHO: %s\n", buffer);
            send_message(client->sock, response);
        }
    }

    if (read_size == 0) {
        printf("Client %d disconnected\n", client->sock);
    } else if (read_size == -1) {
        perror("recv failed");
    }

    // Cleanup
    if (client->logged_in) {
        char response[BUFFER_SIZE];
        snprintf(response, BUFFER_SIZE, "NOTIFICATION: %s has disconnected\n", client->login_name);
        broadcast_message(response, client->sock);
    }
}

// Thread handler cho mỗi client
void *client_thread(void *arg) {
    Client *client = (Client *)arg;
    
    handle_client(client);
    
    // Cleanup
    close(client->sock);
    remove_client(client->sock);
    free(client);
    
    pthread_exit(NULL);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    // Khởi tạo mảng clients
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = NULL;
    }

    // Tạo socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }
    
    // Set socket options để có thể reuse port
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen error");
        exit(EXIT_FAILURE);
    }
    
    printf("=================================\n");
    printf("BattleShip Server Started!\n");
    printf("Listening on port %d...\n", PORT);
    printf("Max clients: %d\n", MAX_CLIENTS);
    printf("=================================\n");

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket < 0) {
            perror("accept failed");
            continue;
        }

        printf("New connection from %s:%d (socket: %d)\n", 
               inet_ntoa(address.sin_addr), 
               ntohs(address.sin_port),
               new_socket);

        // Tạo client struct
        Client *client = (Client *)malloc(sizeof(Client));
        client->sock = new_socket;
        client->logged_in = 0;
        strcpy(client->login_name, "");
        client->address = address;
        
        // Thêm vào danh sách
        add_client(client);
        
        // Tạo thread mới cho client
        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread, (void *)client) != 0) {
            perror("pthread_create failed");
            close(new_socket);
            remove_client(new_socket);
            free(client);
            continue;
        }
        
        // Detach thread để tự động cleanup
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}
