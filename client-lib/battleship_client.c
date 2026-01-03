#include "battleship_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>  // For getaddrinfo (DNS resolution)

BattleshipClient* client_create() {
    BattleshipClient* client = (BattleshipClient*)malloc(sizeof(BattleshipClient));
    if (!client) {
        perror("malloc");
        return NULL;
    }
    
    client->socket_fd = -1;
    client->connected = 0;
    client->recv_offset = 0;
    memset(client->buffer, 0, sizeof(client->buffer));
    memset(client->recv_buffer, 0, sizeof(client->recv_buffer));
    
    return client;
}

int client_connect(BattleshipClient* client, const char* host, int port) {
    if (!client) return -1;
    
    // Tạo socket
    client->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client->socket_fd < 0) {
        perror("socket");
        return -1;
    }

    // Setup server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // Try direct IP first
    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        // Not a valid IP, try DNS resolution
        struct addrinfo hints, *result;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        char port_str[16];
        snprintf(port_str, sizeof(port_str), "%d", port);
        
        int ret = getaddrinfo(host, port_str, &hints, &result);
        if (ret != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
            close(client->socket_fd);
            client->socket_fd = -1;
            return -1;
        }
        
        // Use first result
        memcpy(&server_addr, result->ai_addr, sizeof(server_addr));
        freeaddrinfo(result);
    }

    // Kết nối đến server
    if (connect(client->socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(client->socket_fd);
        client->socket_fd = -1;
        return -1;
    }

    client->connected = 1;
    printf("Connected to server %s:%d\n", host, port);
    
    return 0;
}

int client_send(BattleshipClient* client, const char* message) {
    if (!client || !client->connected || !message) return -1;
    
    int len = strlen(message);
    int total_sent = 0;
    
    // Gửi message
    while (total_sent < len) {
        int sent = send(client->socket_fd, message + total_sent, len - total_sent, 0);
        if (sent < 0) {
            perror("send");
            client->connected = 0;
            return -1;
        }
        total_sent += sent;
    }
    
    // Gửi newline để server phân biệt messages (giống như Node.js server làm)
    if (send(client->socket_fd, "\n", 1, 0) < 0) {
        perror("send newline");
        client->connected = 0;
        return -1;
    }
    
    return total_sent;
}

int client_receive(BattleshipClient* client, char* out_buffer, int max_len) {
    if (!client || !client->connected || !out_buffer) return -1;
    
    // Đọc thêm data từ socket vào recv_buffer
    char temp_buffer[4096];
    int received = recv(client->socket_fd, temp_buffer, sizeof(temp_buffer) - 1, MSG_DONTWAIT);
    
    if (received < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            // No data available (non-blocking mode)
            received = 0;
        } else {
            perror("recv");
            client->connected = 0;
            return -1;
        }
    } else if (received == 0) {
        // Connection closed by server
        printf("Server closed connection\n");
        client->connected = 0;
        return -1;
    }
    
    if (received > 0) {
        temp_buffer[received] = '\0';
        
        // Append vào recv_buffer
        if (client->recv_offset + received < sizeof(client->recv_buffer)) {
            memcpy(client->recv_buffer + client->recv_offset, temp_buffer, received);
            client->recv_offset += received;
            client->recv_buffer[client->recv_offset] = '\0';
        } else {
            fprintf(stderr, "Buffer overflow! Clearing buffer.\n");
            client->recv_offset = 0;
            client->recv_buffer[0] = '\0';
            return 0;
        }
    }
    
    // Tìm message hoàn chỉnh (kết thúc bằng \n)
    char* newline = strchr(client->recv_buffer, '\n');
    if (newline != NULL) {
        // Tìm thấy message hoàn chỉnh
        int msg_len = newline - client->recv_buffer;
        
        if (msg_len >= max_len) {
            msg_len = max_len - 1;
        }
        
        // Copy message ra out_buffer
        memcpy(out_buffer, client->recv_buffer, msg_len);
        out_buffer[msg_len] = '\0';
        
        // Xóa message đã xử lý khỏi buffer
        int remaining = client->recv_offset - msg_len - 1;
        if (remaining > 0) {
            memmove(client->recv_buffer, newline + 1, remaining);
            client->recv_offset = remaining;
            client->recv_buffer[client->recv_offset] = '\0';
        } else {
            client->recv_offset = 0;
            client->recv_buffer[0] = '\0';
        }
        
        return msg_len;
    }
    
    // Chưa có message hoàn chỉnh
    return 0;
}

int client_set_nonblocking(BattleshipClient* client) {
    if (!client || client->socket_fd < 0) return -1;
    
    int flags = fcntl(client->socket_fd, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl F_GETFL");
        return -1;
    }
    
    if (fcntl(client->socket_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl F_SETFL");
        return -1;
    }
    
    return 0;
}

void client_disconnect(BattleshipClient* client) {
    if (!client) return;
    
    if (client->socket_fd >= 0) {
        close(client->socket_fd);
        client->socket_fd = -1;
    }
    
    client->connected = 0;
    client->recv_offset = 0;
    memset(client->recv_buffer, 0, sizeof(client->recv_buffer));
}

void client_destroy(BattleshipClient* client) {
    if (!client) return;
    
    client_disconnect(client);
    free(client);
}
