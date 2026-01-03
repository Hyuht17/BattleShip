#ifndef BATTLESHIP_CLIENT_H
#define BATTLESHIP_CLIENT_H

typedef struct {
    int socket_fd;
    char buffer[8192];
    int connected;
    char recv_buffer[8192];  // Buffer for incomplete messages
    int recv_offset;         // Current offset in recv_buffer
} BattleshipClient;

// Khởi tạo client
BattleshipClient* client_create();

// Kết nối đến server
int client_connect(BattleshipClient* client, const char* host, int port);

// Gửi message (JSON string)
int client_send(BattleshipClient* client, const char* message);

// Nhận message (non-blocking, trả về số ký tự nhận được)
// Returns: >0 nếu có message hoàn chỉnh, 0 nếu chưa có, -1 nếu lỗi
int client_receive(BattleshipClient* client, char* out_buffer, int max_len);

// Ngắt kết nối
void client_disconnect(BattleshipClient* client);

// Giải phóng
void client_destroy(BattleshipClient* client);

// Set non-blocking mode
int client_set_nonblocking(BattleshipClient* client);

#endif
