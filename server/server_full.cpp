#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define PORT 8080
#define MAX_CLIENTS 100
#define BUFFER_SIZE 4096
#define USERNAME_SIZE 50
#define PASSWORD_SIZE 100
#define GRID_SIZE 10
#define MAX_SHIPS 5

// Enums for game states
typedef enum {
    PLAYER_OFFLINE = 0,
    PLAYER_ONLINE = 1,
    PLAYER_IN_LOBBY = 2,
    PLAYER_IN_GAME = 3
} PlayerStatus;

typedef enum {
    GAME_WAITING = 0,
    GAME_PLACING_SHIPS = 1,
    GAME_PLAYING = 2,
    GAME_FINISHED = 3
} GameStatus;

// Ship structure
typedef struct {
    char name[30];
    int size;
    int start_row;
    int start_col;
    int is_horizontal;
    int hits;
} Ship;

// Player structure
typedef struct {
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    int score;
    int games_played;
    int games_won;
} PlayerAccount;

// Game Board structure
typedef struct {
    int grid[GRID_SIZE][GRID_SIZE]; // 0: water, 1: ship, 2: hit, 3: miss
    Ship ships[MAX_SHIPS];
    int ship_count;
    int total_ship_cells;
    int hits_received;
} GameBoard;

// Client structure
typedef struct {
    int sock;
    PlayerStatus status;
    char username[USERNAME_SIZE];
    struct sockaddr_in address;
    int in_game_with; // socket của đối thủ
    GameBoard board;
    int ready; // đã đặt xong tàu chưa
    int is_turn; // lượt của mình không
} Client;

// Game session structure
typedef struct {
    int player1_sock;
    int player2_sock;
    GameStatus status;
    int current_turn; // socket của người chơi đang có lượt
    time_t start_time;
    char log_id[50];
} GameSession;

// Global variables
Client *clients[MAX_CLIENTS];
GameSession *game_sessions[MAX_CLIENTS / 2];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t games_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function prototypes
void add_client(Client *client);
void remove_client(int sock);
Client* get_client(int sock);
Client* get_client_by_username(const char *username);
void send_message(int sock, const char *message);
void broadcast_message(const char *message, int sender_sock);
int authenticate_user(const char *username, const char *password);
int register_user(const char *username, const char *password);
void handle_client(Client *client);
void *client_thread(void *arg);
void handle_command(Client *client, const char *cmd, const char *payload);
void send_player_list(int sock);
void handle_challenge(Client *challenger, const char *target_username);
void handle_challenge_reply(Client *client, const char *challenger_username, const char *status);
void start_game(Client *player1, Client *player2);
void handle_place_ships(Client *client, const char *ships_data);
void handle_move(Client *client, const char *coord);
void check_game_end(GameSession *session);
void end_game(GameSession *session, int winner_sock, const char *reason);
void handle_disconnect(Client *client);

// Utility functions
void init_board(GameBoard *board) {
    memset(board->grid, 0, sizeof(board->grid));
    board->ship_count = 0;
    board->total_ship_cells = 0;
    board->hits_received = 0;
}

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

Client* get_client(int sock) {
    pthread_mutex_lock(&clients_mutex);
    Client *result = NULL;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->sock == sock) {
            result = clients[i];
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return result;
}

Client* get_client_by_username(const char *username) {
    pthread_mutex_lock(&clients_mutex);
    Client *result = NULL;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && 
            clients[i]->status != PLAYER_OFFLINE &&
            strcmp(clients[i]->username, username) == 0) {
            result = clients[i];
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return result;
}

void send_message(int sock, const char *message) {
    if (send(sock, message, strlen(message), MSG_NOSIGNAL) < 0) {
        printf("Send failed to socket %d: %s\n", sock, strerror(errno));
    }
}

void broadcast_message(const char *message, int sender_sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && 
            clients[i]->sock != sender_sock && 
            clients[i]->status != PLAYER_OFFLINE) {
            send_message(clients[i]->sock, message);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Authentication functions (simple file-based storage)
int register_user(const char *username, const char *password) {
    FILE *fp = fopen("users.dat", "a+");
    if (!fp) return 0;
    
    // Check if username exists
    char line[256];
    rewind(fp);
    while (fgets(line, sizeof(line), fp)) {
        char stored_user[USERNAME_SIZE];
        sscanf(line, "%[^:]:", stored_user);
        if (strcmp(stored_user, username) == 0) {
            fclose(fp);
            return 0; // Username already exists
        }
    }
    
    // Add new user
    fprintf(fp, "%s:%s:0:0:0\n", username, password);
    fclose(fp);
    return 1;
}

int authenticate_user(const char *username, const char *password) {
    FILE *fp = fopen("users.dat", "r");
    if (!fp) return 0;
    
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        char stored_user[USERNAME_SIZE];
        char stored_pass[PASSWORD_SIZE];
        sscanf(line, "%[^:]:%[^:]:", stored_user, stored_pass);
        if (strcmp(stored_user, username) == 0 && strcmp(stored_pass, password) == 0) {
            fclose(fp);
            return 1;
        }
    }
    
    fclose(fp);
    return 0;
}

void send_player_list(int sock) {
    char response[BUFFER_SIZE];
    int offset = sprintf(response, "{\"cmd\":\"PLAYER_LIST\",\"payload\":{\"players\":[");
    
    pthread_mutex_lock(&clients_mutex);
    int first = 1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && 
            clients[i]->status != PLAYER_OFFLINE && 
            clients[i]->status != PLAYER_IN_GAME &&
            clients[i]->sock != sock) {
            
            if (!first) offset += sprintf(response + offset, ",");
            offset += sprintf(response + offset, 
                "{\"username\":\"%s\",\"status\":%d}", 
                clients[i]->username, clients[i]->status);
            first = 0;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    offset += sprintf(response + offset, "]}}\n");
    send_message(sock, response);
}

void handle_challenge(Client *challenger, const char *target_username) {
    Client *target = get_client_by_username(target_username);
    
    if (!target) {
        char response[BUFFER_SIZE];
        sprintf(response, "{\"cmd\":\"SYSTEM_MSG\",\"payload\":{\"code\":404,\"message\":\"Player not found or offline\"}}\n");
        send_message(challenger->sock, response);
        return;
    }
    
    if (target->status == PLAYER_IN_GAME) {
        char response[BUFFER_SIZE];
        sprintf(response, "{\"cmd\":\"SYSTEM_MSG\",\"payload\":{\"code\":400,\"message\":\"Player is in game\"}}\n");
        send_message(challenger->sock, response);
        return;
    }
    
    // Send challenge to target
    char message[BUFFER_SIZE];
    sprintf(message, "{\"cmd\":\"CHALLENGE\",\"payload\":{\"challenger\":\"%s\"}}\n", challenger->username);
    send_message(target->sock, message);
    
    // Notify challenger
    sprintf(message, "{\"cmd\":\"SYSTEM_MSG\",\"payload\":{\"code\":200,\"message\":\"Challenge sent to %s\"}}\n", target_username);
    send_message(challenger->sock, message);
}

void handle_challenge_reply(Client *client, const char *challenger_username, const char *status) {
    Client *challenger = get_client_by_username(challenger_username);
    
    if (!challenger) {
        char response[BUFFER_SIZE];
        sprintf(response, "{\"cmd\":\"SYSTEM_MSG\",\"payload\":{\"code\":404,\"message\":\"Challenger not found\"}}\n");
        send_message(client->sock, response);
        return;
    }
    
    char message[BUFFER_SIZE];
    
    if (strcmp(status, "ACCEPT") == 0) {
        // Start game
        start_game(challenger, client);
    } else {
        // Notify challenger of rejection
        sprintf(message, "{\"cmd\":\"CHALLENGE_REPLY\",\"payload\":{\"player\":\"%s\",\"status\":\"REJECT\"}}\n", client->username);
        send_message(challenger->sock, message);
    }
}

void start_game(Client *player1, Client *player2) {
    // Create game session
    pthread_mutex_lock(&games_mutex);
    GameSession *session = NULL;
    for (int i = 0; i < MAX_CLIENTS / 2; i++) {
        if (game_sessions[i] == NULL) {
            session = (GameSession *)malloc(sizeof(GameSession));
            game_sessions[i] = session;
            break;
        }
    }
    pthread_mutex_unlock(&games_mutex);
    
    if (!session) {
        char msg[BUFFER_SIZE];
        sprintf(msg, "{\"cmd\":\"SYSTEM_MSG\",\"payload\":{\"code\":500,\"message\":\"Server full\"}}\n");
        send_message(player1->sock, msg);
        send_message(player2->sock, msg);
        return;
    }
    
    // Initialize session
    session->player1_sock = player1->sock;
    session->player2_sock = player2->sock;
    session->status = GAME_PLACING_SHIPS;
    session->start_time = time(NULL);
    sprintf(session->log_id, "game_%ld", session->start_time);
    
    // Update players
    player1->status = PLAYER_IN_GAME;
    player1->in_game_with = player2->sock;
    player1->ready = 0;
    player1->is_turn = 1;
    init_board(&player1->board);
    
    player2->status = PLAYER_IN_GAME;
    player2->in_game_with = player1->sock;
    player2->ready = 0;
    player2->is_turn = 0;
    init_board(&player2->board);
    
    // Notify both players
    char message[BUFFER_SIZE];
    sprintf(message, "{\"cmd\":\"GAME_START\",\"payload\":{\"opponent\":\"%s\",\"your_turn\":%d}}\n", 
            player2->username, player1->is_turn);
    send_message(player1->sock, message);
    
    sprintf(message, "{\"cmd\":\"GAME_START\",\"payload\":{\"opponent\":\"%s\",\"your_turn\":%d}}\n", 
            player1->username, player2->is_turn);
    send_message(player2->sock, message);
    
    printf("Game started: %s vs %s\n", player1->username, player2->username);
}

void handle_place_ships(Client *client, const char *ships_data) {
    // Parse ships data from JSON
    // Format: [{"name":"Carrier","size":5,"row":0,"col":0,"horizontal":true}, ...]
    
    printf("[DEBUG] handle_place_ships called for user: %s\n", client->username);
    printf("[DEBUG] ships_data: %s\n", ships_data);
    
    init_board(&client->board);
    
    // Simple parsing (in production, use a JSON library)
    const char *ptr = ships_data;
    while (*ptr) {
        if (*ptr == '{') {
            char name[30];
            int size, row, col, horizontal;
            
            // Extract ship data (simplified)
            if (sscanf(ptr, "{\"name\":\"%[^\"]\",\"size\":%d,\"row\":%d,\"col\":%d,\"horizontal\":%d", 
                      name, &size, &row, &col, &horizontal) == 5) {
                
                if (client->board.ship_count < MAX_SHIPS && 
                    row >= 0 && row < GRID_SIZE && 
                    col >= 0 && col < GRID_SIZE) {
                    
                    Ship *ship = &client->board.ships[client->board.ship_count];
                    strncpy(ship->name, name, sizeof(ship->name) - 1);
                    ship->size = size;
                    ship->start_row = row;
                    ship->start_col = col;
                    ship->is_horizontal = horizontal;
                    ship->hits = 0;
                    
                    // Place ship on grid
                    for (int i = 0; i < size; i++) {
                        int r = row + (horizontal ? 0 : i);
                        int c = col + (horizontal ? i : 0);
                        if (r < GRID_SIZE && c < GRID_SIZE) {
                            client->board.grid[r][c] = 1;
                            client->board.total_ship_cells++;
                        }
                    }
                    
                    client->board.ship_count++;
                }
            }
        }
        ptr++;
    }
    
    printf("[DEBUG] Ships placed: %d, Total cells: %d\n", client->board.ship_count, client->board.total_ship_cells);
    
    client->ready = 1;
    
    // Check if opponent is ready
    Client *opponent = get_client(client->in_game_with);
    if (opponent && opponent->ready) {
        // Both ready, start game
        char message[BUFFER_SIZE];
        sprintf(message, "{\"cmd\":\"GAME_READY\",\"payload\":{\"message\":\"Game starting!\"}}\n");
        send_message(client->sock, message);
        send_message(opponent->sock, message);
        
        // Find game session and update status
        pthread_mutex_lock(&games_mutex);
        for (int i = 0; i < MAX_CLIENTS / 2; i++) {
            if (game_sessions[i] && 
                ((game_sessions[i]->player1_sock == client->sock && game_sessions[i]->player2_sock == opponent->sock) ||
                 (game_sessions[i]->player2_sock == client->sock && game_sessions[i]->player1_sock == opponent->sock))) {
                game_sessions[i]->status = GAME_PLAYING;
                game_sessions[i]->current_turn = game_sessions[i]->player1_sock;
                break;
            }
        }
        pthread_mutex_unlock(&games_mutex);
    } else {
        char response[BUFFER_SIZE];
        sprintf(response, "{\"cmd\":\"PLACE_SHIP_ACK\",\"payload\":{\"message\":\"Waiting for opponent\"}}\n");
        send_message(client->sock, response);
    }
}

void handle_move(Client *client, const char *coord) {
    if (!client->is_turn) {
        char response[BUFFER_SIZE];
        sprintf(response, "{\"cmd\":\"SYSTEM_MSG\",\"payload\":{\"code\":400,\"message\":\"Not your turn\"}}\n");
        send_message(client->sock, response);
        return;
    }
    
    // Parse coordinate (e.g., "A5" -> row=0, col=4)
    int row = coord[0] - 'A';
    int col = atoi(coord + 1);
    
    if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) {
        char response[BUFFER_SIZE];
        sprintf(response, "{\"cmd\":\"SYSTEM_MSG\",\"payload\":{\"code\":400,\"message\":\"Invalid coordinate\"}}\n");
        send_message(client->sock, response);
        return;
    }
    
    Client *opponent = get_client(client->in_game_with);
    if (!opponent) {
        char response[BUFFER_SIZE];
        sprintf(response, "{\"cmd\":\"SYSTEM_MSG\",\"payload\":{\"code\":404,\"message\":\"Opponent not found\"}}\n");
        send_message(client->sock, response);
        return;
    }
    
    // Check if opponent has placed ships
    if (opponent->board.total_ship_cells == 0) {
        char response[BUFFER_SIZE];
        sprintf(response, "{\"cmd\":\"SYSTEM_MSG\",\"payload\":{\"code\":400,\"message\":\"Opponent hasn't placed ships yet\"}}\n");
        send_message(client->sock, response);
        return;
    }
    
    // Check hit or miss
    int cell = opponent->board.grid[row][col];
    char result[20];
    char ship_sunk[30] = "";
    
    if (cell == 1) {
        strcpy(result, "HIT");
        opponent->board.grid[row][col] = 2; // mark as hit
        opponent->board.hits_received++;
        
        // Check if ship is sunk
        for (int i = 0; i < opponent->board.ship_count; i++) {
            Ship *ship = &opponent->board.ships[i];
            int ship_hits = 0;
            for (int j = 0; j < ship->size; j++) {
                int r = ship->start_row + (ship->is_horizontal ? 0 : j);
                int c = ship->start_col + (ship->is_horizontal ? j : 0);
                if (opponent->board.grid[r][c] == 2) ship_hits++;
            }
            if (ship_hits == ship->size) {
                strcpy(ship_sunk, ship->name);
                break;
            }
        }
    } else if (cell == 0) {
        strcpy(result, "MISS");
        opponent->board.grid[row][col] = 3; // mark as miss
    } else {
        strcpy(result, "ALREADY_HIT");
    }
    
    // Send result to both players
    char message[BUFFER_SIZE];
    sprintf(message, "{\"cmd\":\"MOVE_RESULT\",\"payload\":{\"coord\":\"%s\",\"result\":\"%s\",\"ship_sunk\":\"%s\"}}\n", 
            coord, result, ship_sunk);
    send_message(client->sock, message);
    send_message(opponent->sock, message);
    
    // Check game end - opponent must have ships and all ships sunk
    if (opponent->board.total_ship_cells > 0 && 
        opponent->board.hits_received >= opponent->board.total_ship_cells) {
        // Client wins
        pthread_mutex_lock(&games_mutex);
        for (int i = 0; i < MAX_CLIENTS / 2; i++) {
            if (game_sessions[i] && 
                ((game_sessions[i]->player1_sock == client->sock && game_sessions[i]->player2_sock == opponent->sock) ||
                 (game_sessions[i]->player2_sock == client->sock && game_sessions[i]->player1_sock == opponent->sock))) {
                end_game(game_sessions[i], client->sock, "ALL_SHIPS_SUNK");
                free(game_sessions[i]);
                game_sessions[i] = NULL;
                break;
            }
        }
        pthread_mutex_unlock(&games_mutex);
        return;
    }
    
    // Switch turns
    client->is_turn = 0;
    opponent->is_turn = 1;
    
    sprintf(message, "{\"cmd\":\"TURN_CHANGE\",\"payload\":{\"your_turn\":false}}\n");
    send_message(client->sock, message);
    
    sprintf(message, "{\"cmd\":\"TURN_CHANGE\",\"payload\":{\"your_turn\":true}}\n");
    send_message(opponent->sock, message);
}

void end_game(GameSession *session, int winner_sock, const char *reason) {
    Client *winner = get_client(winner_sock);
    Client *loser = get_client(winner_sock == session->player1_sock ? session->player2_sock : session->player1_sock);
    
    if (winner) {
        char message[BUFFER_SIZE];
        sprintf(message, "{\"cmd\":\"GAME_END\",\"payload\":{\"result\":\"WIN\",\"reason\":\"%s\",\"log_id\":\"%s\"}}\n", 
                reason, session->log_id);
        send_message(winner->sock, message);
        winner->status = PLAYER_ONLINE;
        winner->in_game_with = 0;
    }
    
    if (loser) {
        char message[BUFFER_SIZE];
        sprintf(message, "{\"cmd\":\"GAME_END\",\"payload\":{\"result\":\"LOSE\",\"reason\":\"%s\",\"log_id\":\"%s\"}}\n", 
                reason, session->log_id);
        send_message(loser->sock, message);
        loser->status = PLAYER_ONLINE;
        loser->in_game_with = 0;
    }
    
    printf("Game ended: %s\n", reason);
}

void handle_disconnect(Client *client) {
    if (client->status == PLAYER_IN_GAME && client->in_game_with > 0) {
        Client *opponent = get_client(client->in_game_with);
        if (opponent) {
            char message[BUFFER_SIZE];
            sprintf(message, "{\"cmd\":\"GAME_END\",\"payload\":{\"result\":\"WIN\",\"reason\":\"OPPONENT_DISCONNECT\",\"log_id\":\"\"}}\n");
            send_message(opponent->sock, message);
            opponent->status = PLAYER_ONLINE;
            opponent->in_game_with = 0;
        }
        
        // Remove game session
        pthread_mutex_lock(&games_mutex);
        for (int i = 0; i < MAX_CLIENTS / 2; i++) {
            if (game_sessions[i] && 
                (game_sessions[i]->player1_sock == client->sock || game_sessions[i]->player2_sock == client->sock)) {
                free(game_sessions[i]);
                game_sessions[i] = NULL;
                break;
            }
        }
        pthread_mutex_unlock(&games_mutex);
    }
}

void handle_command(Client *client, const char *cmd, const char *payload) {
    if (strcmp(cmd, "REGISTER") == 0) {
        char username[USERNAME_SIZE], password[PASSWORD_SIZE];
        sscanf(payload, "{\"username\":\"%[^\"]\",\"password\":\"%[^\"]\"}", username, password);
        
        if (register_user(username, password)) {
            char response[BUFFER_SIZE];
            sprintf(response, "{\"cmd\":\"REGISTER_SUCCESS\",\"payload\":{\"message\":\"Registration successful\"}}\n");
            send_message(client->sock, response);
        } else {
            char response[BUFFER_SIZE];
            sprintf(response, "{\"cmd\":\"SYSTEM_MSG\",\"payload\":{\"code\":400,\"message\":\"Username already exists\"}}\n");
            send_message(client->sock, response);
        }
    }
    else if (strcmp(cmd, "LOGIN") == 0) {
        char username[USERNAME_SIZE], password[PASSWORD_SIZE];
        sscanf(payload, "{\"username\":\"%[^\"]\",\"password\":\"%[^\"]\"}", username, password);
        
        if (authenticate_user(username, password)) {
            strncpy(client->username, username, USERNAME_SIZE - 1);
            client->status = PLAYER_ONLINE;
            
            char response[BUFFER_SIZE];
            sprintf(response, "{\"cmd\":\"LOGIN_SUCCESS\",\"payload\":{\"username\":\"%s\",\"message\":\"Welcome!\"}}\n", username);
            send_message(client->sock, response);
            
            printf("User logged in: %s (socket %d)\n", username, client->sock);
        } else {
            char response[BUFFER_SIZE];
            sprintf(response, "{\"cmd\":\"SYSTEM_MSG\",\"payload\":{\"code\":401,\"message\":\"Invalid credentials\"}}\n");
            send_message(client->sock, response);
        }
    }
    else if (strcmp(cmd, "PLAYER_LIST") == 0) {
        send_player_list(client->sock);
    }
    else if (strcmp(cmd, "CHALLENGE") == 0) {
        char target[USERNAME_SIZE];
        sscanf(payload, "{\"target_username\":\"%[^\"]\"}", target);
        handle_challenge(client, target);
    }
    else if (strcmp(cmd, "CHALLENGE_REPLY") == 0) {
        char challenger[USERNAME_SIZE], status[20];
        sscanf(payload, "{\"challenger_username\":\"%[^\"]\",\"status\":\"%[^\"]\"}", challenger, status);
        handle_challenge_reply(client, challenger, status);
    }
    else if (strcmp(cmd, "PLACE_SHIPS") == 0) {
        // Extract ships array from payload: {"ships":[...]}
        const char *ships_start = strstr(payload, "\"ships\":");
        if (ships_start) {
            ships_start += 8; // skip "ships":
            while (*ships_start == ' ') ships_start++; // skip whitespace
            handle_place_ships(client, ships_start);
        }
    }
    else if (strcmp(cmd, "MOVE") == 0) {
        char coord[10];
        sscanf(payload, "{\"coord\":\"%[^\"]\"}", coord);
        handle_move(client, coord);
    }
    else if (strcmp(cmd, "CHAT") == 0) {
        char message[BUFFER_SIZE];
        sscanf(payload, "{\"message\":\"%[^\"]\"}", message);
        
        Client *opponent = get_client(client->in_game_with);
        if (opponent) {
            char response[BUFFER_SIZE];
            sprintf(response, "{\"cmd\":\"CHAT\",\"payload\":{\"from\":\"%s\",\"message\":\"%s\"}}\n", 
                    client->username, message);
            send_message(opponent->sock, response);
        }
    }
}

void handle_client(Client *client) {
    char buffer[BUFFER_SIZE];
    int read_size;
    
    send_message(client->sock, "{\"cmd\":\"WELCOME\",\"payload\":{\"message\":\"Welcome to BattleShip Server\"}}\n");
    
    while ((read_size = recv(client->sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[read_size] = '\0';
        
        // Remove newline
        char *newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';
        
        printf("Received from %s (sock %d): %s\n", client->username[0] ? client->username : "unknown", client->sock, buffer);
        
        // Parse JSON command
        char cmd[50] = "", payload[BUFFER_SIZE] = "";
        char *cmd_start = strstr(buffer, "\"cmd\":\"");
        char *payload_start = strstr(buffer, "\"payload\":");
        
        if (cmd_start) {
            cmd_start += 7;
            char *cmd_end = strchr(cmd_start, '"');
            if (cmd_end) {
                int len = cmd_end - cmd_start;
                strncpy(cmd, cmd_start, len < 50 ? len : 49);
                cmd[len < 50 ? len : 49] = '\0';
            }
        }
        
        if (payload_start) {
            payload_start += 10;
            char *payload_end = strrchr(payload_start, '}');
            if (payload_end) {
                payload_end++;
                int len = payload_end - payload_start;
                strncpy(payload, payload_start, len < BUFFER_SIZE ? len : BUFFER_SIZE - 1);
                payload[len < BUFFER_SIZE ? len : BUFFER_SIZE - 1] = '\0';
            }
        }
        
        if (cmd[0]) {
            handle_command(client, cmd, payload);
        }
    }
    
    if (read_size == 0) {
        printf("Client disconnected: %s (sock %d)\n", client->username, client->sock);
    } else if (read_size == -1) {
        printf("Recv error from client %s (sock %d): %s\n", client->username, client->sock, strerror(errno));
    }
    
    handle_disconnect(client);
}

void *client_thread(void *arg) {
    Client *client = (Client *)arg;
    handle_client(client);
    close(client->sock);
    remove_client(client->sock);
    free(client);
    pthread_exit(NULL);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    // Initialize arrays
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = NULL;
    }
    for (int i = 0; i < MAX_CLIENTS / 2; i++) {
        game_sessions[i] = NULL;
    }
    
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }
    
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
    
    printf("╔═══════════════════════════════════════╗\n");
    printf("║   BattleShip TCP Server Started!     ║\n");
    printf("║   Port: %d                         ║\n", PORT);
    printf("║   Max Clients: %d                   ║\n", MAX_CLIENTS);
    printf("╚═══════════════════════════════════════╝\n");
    
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
        
        Client *client = (Client *)malloc(sizeof(Client));
        client->sock = new_socket;
        client->status = PLAYER_OFFLINE;
        strcpy(client->username, "");
        client->address = address;
        client->in_game_with = 0;
        client->ready = 0;
        client->is_turn = 0;
        init_board(&client->board);
        
        add_client(client);
        
        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread, (void *)client) != 0) {
            perror("pthread_create failed");
            close(new_socket);
            remove_client(new_socket);
            free(client);
            continue;
        }
        
        pthread_detach(tid);
    }
    
    close(server_fd);
    return 0;
}
