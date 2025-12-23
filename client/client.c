#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#define CPP_SERVER_IP "172.18.36.171"  // Thay bằng IP của C++ Server (ví dụ: "192.168.1.100")
#define CPP_SERVER_PORT 8080
#define LOCAL_SERVER_PORT 9000  // C Client làm server cho Node.js
#define BUFFER_SIZE 4096
#define MAX_CLIENTS 10

int cpp_sock = -1;  // Socket kết nối đến C++ server
int local_server_fd = -1;  // Socket server cho Node.js clients
int node_clients[MAX_CLIENTS] = {0};  // Danh sách Node.js clients
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int running = 1;

// Signal handler
void handle_signal(int sig) {
    fprintf(stderr, "\n[C_CLIENT] Received signal %d, shutting down...\n", sig);
    running = 0;
    
    if (cpp_sock >= 0) {
        shutdown(cpp_sock, SHUT_RDWR);
    }
    if (local_server_fd >= 0) {
        shutdown(local_server_fd, SHUT_RDWR);
    }
}

// Broadcast message to all Node.js clients
void broadcast_to_nodejs(const char *message) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (node_clients[i] > 0) {
            send(node_clients[i], message, strlen(message), MSG_NOSIGNAL);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Thread nhận dữ liệu từ C++ server
void *receive_from_cpp_thread(void *arg) {
    char buffer[BUFFER_SIZE];
    int read_size;
    
    while (running && (read_size = recv(cpp_sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[read_size] = '\0';
        
        fprintf(stderr, "[C_CLIENT] Received from C++ server: %.*s\n", 50, buffer);
        
        // Forward to all Node.js clients
        broadcast_to_nodejs(buffer);
    }
    
    if (read_size == 0) {
        fprintf(stderr, "[C_CLIENT] C++ server disconnected\n");
    } else if (read_size < 0 && running) {
        fprintf(stderr, "[C_CLIENT] Recv error from C++ server: %s\n", strerror(errno));
    }
    
    running = 0;
    return NULL;
}

// Gửi message đến C++ server
void send_to_cpp_server(const char *message) {
    pthread_mutex_lock(&send_mutex);
    
    if (cpp_sock >= 0 && running) {
        fprintf(stderr, "[C_CLIENT] Sent to C++ server: %.*s\n", 50, message);
        int sent = send(cpp_sock, message, strlen(message), MSG_NOSIGNAL);
        if (sent < 0) {
            fprintf(stderr, "[C_CLIENT] Send error to C++ server: %s\n", strerror(errno));
        }
    }
    
    pthread_mutex_unlock(&send_mutex);
}

// Thread xử lý 1 Node.js client
void *handle_nodejs_client(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    int read_size;
    
    while (running && (read_size = recv(client_sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[read_size] = '\0';
        
        // Forward to C++ server
        send_to_cpp_server(buffer);
    }
    
    // Remove from list
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (node_clients[i] == client_sock) {
            node_clients[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    close(client_sock);
    return NULL;
}

// Thread accept Node.js clients
void *accept_nodejs_clients(void *arg) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    while (running) {
        int client_sock = accept(local_server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) {
            if (running) {
                fprintf(stderr, "[C_CLIENT] Accept error: %s\n", strerror(errno));
            }
            continue;
        }
        
        // Add to list
        pthread_mutex_lock(&clients_mutex);
        int added = 0;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (node_clients[i] == 0) {
                node_clients[i] = client_sock;
                added = 1;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
        
        if (!added) {
            fprintf(stderr, "[C_CLIENT] Max clients reached, rejecting connection\n");
            close(client_sock);
            continue;
        }
        
        // Spawn thread to handle this client
        pthread_t tid;
        int *sock_ptr = malloc(sizeof(int));
        *sock_ptr = client_sock;
        pthread_create(&tid, NULL, handle_nodejs_client, sock_ptr);
        pthread_detach(tid);
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in cpp_server_addr, local_server_addr;
    pthread_t cpp_recv_tid, accept_tid;
    char *server_ip = CPP_SERVER_IP;
    int server_port = CPP_SERVER_PORT;
    int local_port = LOCAL_SERVER_PORT;
    
    // Cho phép override từ command line
    // Usage: ./client [local_port] [server_ip] [server_port]
    // Ví dụ: ./client 9001
    // Ví dụ: ./client 9001 192.168.1.100
    // Ví dụ: ./client 9001 192.168.1.100 8080
    if (argc >= 2) {
        local_port = atoi(argv[1]);
    }
    if (argc >= 3) {
        server_ip = argv[2];
    }
    if (argc >= 4) {
        server_port = atoi(argv[3]);
    }
    
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGPIPE, SIG_IGN);
    
    fprintf(stderr, "[C_CLIENT] Starting C client bridge...\n");
    fprintf(stderr, "[C_CLIENT] Local TCP server will listen on port %d\n", local_port);
    
    // 1. Kết nối tới C++ server (port 8080)
    cpp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (cpp_sock < 0) {
        fprintf(stderr, "[C_CLIENT] Socket creation failed: %s\n", strerror(errno));
        return 1;
    }
    
    memset(&cpp_server_addr, 0, sizeof(cpp_server_addr));
    cpp_server_addr.sin_family = AF_INET;
    cpp_server_addr.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_ip, &cpp_server_addr.sin_addr) <= 0) {
        fprintf(stderr, "[C_CLIENT] Invalid C++ server address\n");
        close(cpp_sock);
        return 1;
    }
    
    fprintf(stderr, "[C_CLIENT] Connecting to C++ server %s:%d...\n", server_ip, server_port);
    
    if (connect(cpp_sock, (struct sockaddr *)&cpp_server_addr, sizeof(cpp_server_addr)) < 0) {
        fprintf(stderr, "[C_CLIENT] Connection to C++ server failed: %s\n", strerror(errno));
        close(cpp_sock);
        return 1;
    }
    
    fprintf(stderr, "[C_CLIENT] Connected to C++ server successfully\n");
    
    // 2. Tạo TCP server để accept Node.js clients (port 9000)
    local_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (local_server_fd < 0) {
        fprintf(stderr, "[C_CLIENT] Local server socket creation failed: %s\n", strerror(errno));
        close(cpp_sock);
        return 1;
    }
    
    int opt = 1;
    setsockopt(local_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    memset(&local_server_addr, 0, sizeof(local_server_addr));
    local_server_addr.sin_family = AF_INET;
    local_server_addr.sin_addr.s_addr = INADDR_ANY;
    local_server_addr.sin_port = htons(local_port);
    
    if (bind(local_server_fd, (struct sockaddr *)&local_server_addr, sizeof(local_server_addr)) < 0) {
        fprintf(stderr, "[C_CLIENT] Bind failed on port %d: %s\n", local_port, strerror(errno));
        close(cpp_sock);
        close(local_server_fd);
        return 1;
    }
    
    if (listen(local_server_fd, 10) < 0) {
        fprintf(stderr, "[C_CLIENT] Listen failed: %s\n", strerror(errno));
        close(cpp_sock);
        close(local_server_fd);
        return 1;
    }
    
    fprintf(stderr, "[C_CLIENT] TCP server listening on port %d for Node.js clients\n", local_port);
    
    // 3. Start threads
    pthread_create(&cpp_recv_tid, NULL, receive_from_cpp_thread, NULL);
    pthread_create(&accept_tid, NULL, accept_nodejs_clients, NULL);
    
    fprintf(stderr, "[C_CLIENT] Bridge running. Press Ctrl+C to exit.\n");
    
    // Wait for threads
    pthread_join(cpp_recv_tid, NULL);
    pthread_join(accept_tid, NULL);
    
    // Cleanup
    fprintf(stderr, "[C_CLIENT] Shutting down...\n");
    
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (node_clients[i] > 0) {
            close(node_clients[i]);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    if (cpp_sock >= 0) close(cpp_sock);
    if (local_server_fd >= 0) close(local_server_fd);
    
    fprintf(stderr, "[C_CLIENT] Exited cleanly\n");
    return 0;
}
