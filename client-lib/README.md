# C Client Library for Battleship

Pure C library để kết nối TCP đến Battleship C++ server.

## API

```c
// Khởi tạo
BattleshipClient* client = client_create();

// Kết nối
client_connect(client, "127.0.0.1", 8080);
client_set_nonblocking(client);

// Gửi JSON message
client_send(client, "{\"type\":\"LOGIN\",\"username\":\"test\"}");

// Nhận JSON message (non-blocking)
char buffer[4096];
int len = client_receive(client, buffer, sizeof(buffer));
if (len > 0) {
    printf("Received: %s\n", buffer);
}

// Cleanup
client_disconnect(client);
client_destroy(client);
```

## Build

```bash
make              # Build library và test
./test_client     # Test kết nối (cần server đang chạy)
make clean
```

## Features

- TCP socket thuần C (không phụ thuộc external libs)
- Non-blocking I/O với `MSG_DONTWAIT`
- Message buffering để xử lý incomplete JSON
- Messages phân tách bằng newline `\n`
- Compatible với server C++ protocol

## Integration

### Với Qt (C++)
```cpp
extern "C" {
#include "battleship_client.h"
}

BattleshipClient* client = client_create();
client_connect(client, "127.0.0.1", 8080);
```

### Với C program
```c
#include "battleship_client.h"

int main() {
    BattleshipClient* client = client_create();
    // ... use client
    client_destroy(client);
}
```

## Protocol

Server expects JSON messages kết thúc bằng `\n`:
```
{"type":"LOGIN","username":"test"}\n
```

Server sends JSON responses kết thúc bằng `\n`:
```
{"type":"LOGIN_SUCCESS","user_id":1}\n
```

Library tự động append/parse newlines.
