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
    int elo;
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
    char session_token[64]; // Token để xác thực session
    time_t last_active; // Thời gian hoạt động cuối
    struct sockaddr_in address;
    int in_game_with; // socket của đối thủ
    GameBoard board;
    int ready; // đã đặt xong tàu chưa
    int is_turn; // lượt của mình không
    int is_matching; // đang tìm trận không
    int match_ready; // đã sẵn sàng sau khi matching
    int ping; // ping của client (ms)
    time_t last_ping_time; // thời điểm gửi ping gần nhất
} Client;

// Game session structure
typedef struct {
    int player1_sock;
    int player2_sock;
    char player1_username[USERNAME_SIZE]; // Lưu username để reconnect
    char player2_username[USERNAME_SIZE];
    GameStatus status;
    int current_turn; // socket của người chơi đang có lượt
    time_t start_time;
    time_t player1_disconnect_time; // Thời gian disconnect
    time_t player2_disconnect_time;
    int player1_disconnected; // 0 = connected, 1 = disconnected
    int player2_disconnected;
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
void handle_surrender(Client *client);
void handle_draw_offer(Client *client);
void handle_draw_reply(Client *client, const char *status);
void cleanup_game_on_exit(Client *client, int notify_opponent);
void handle_disconnect(Client *client);
void handle_logout(Client *client);
void generate_session_token(char *token);
void handle_start_matching(Client *client);
void handle_cancel_matching(Client *client);
void handle_match_ready(Client *client);
void handle_match_decline(Client *client);
void handle_leaderboard(Client *client);
void try_match_players();

// Utility functions
void init_board(GameBoard *board) {
    memset(board->grid, 0, sizeof(board->grid));
    board->ship_count = 0;
    board->total_ship_cells = 0;
    board->hits_received = 0;
}

// Generate session token
void generate_session_token(char *token) {
    sprintf(token, "%ld_%d", time(NULL), rand());
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
    
    // Add new user with default ELO 800
    fprintf(fp, "%s:%s:800:0:0\n", username, password);
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

// Get player ELO
int get_player_elo(const char *username) {
    FILE *fp = fopen("users.dat", "r");
    if (!fp) return 800; // Default ELO
    
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        char stored_user[USERNAME_SIZE];
        char stored_pass[PASSWORD_SIZE];
        int elo, games_played, games_won;
        sscanf(line, "%[^:]:%[^:]:%d:%d:%d", stored_user, stored_pass, &elo, &games_played, &games_won);
        if (strcmp(stored_user, username) == 0) {
            fclose(fp);
            return elo;
        }
    }
    
    fclose(fp);
    return 800;
}

// Update player ELO and stats
void update_player_stats(const char *username, int elo_change, int is_winner) {
    FILE *fp = fopen("users.dat", "r");
    if (!fp) return;
    
    char lines[MAX_CLIENTS][256];
    int line_count = 0;
    
    // Read all lines
    while (fgets(lines[line_count], 256, fp) && line_count < MAX_CLIENTS) {
        line_count++;
    }
    fclose(fp);
    
    // Update the user's line
    fp = fopen("users.dat", "w");
    if (!fp) return;
    
    for (int i = 0; i < line_count; i++) {
        char stored_user[USERNAME_SIZE];
        char stored_pass[PASSWORD_SIZE];
        int elo, games_played, games_won;
        sscanf(lines[i], "%[^:]:%[^:]:%d:%d:%d", stored_user, stored_pass, &elo, &games_played, &games_won);
        
        if (strcmp(stored_user, username) == 0) {
            elo += elo_change;
            if (elo < 0) elo = 0; // Minimum ELO is 0
            games_played++;
            if (is_winner) games_won++;
            fprintf(fp, "%s:%s:%d:%d:%d\n", stored_user, stored_pass, elo, games_played, games_won);
        } else {
            fprintf(fp, "%s", lines[i]);
        }
    }
    
    fclose(fp);
}

// Save match history to user's history file
// Format: timestamp:opponent:result (WIN/LOSE/DRAW)
void save_match_history(const char *username, const char *opponent, const char *result) {
    // Create history directory if it doesn't exist
    system("mkdir -p history");
    
    char filename[128];
    sprintf(filename, "history/match_history_%s.dat", username);
    
    FILE *fp = fopen(filename, "a");
    if (!fp) return;
    
    time_t now = time(NULL);
    fprintf(fp, "%ld:%s:%s\n", now, opponent, result);
    fclose(fp);
    
    printf("[MATCH_HISTORY] Saved for %s vs %s: %s\n", username, opponent, result);
}

// Get match history for a user
void send_match_history(int sock, const char *username) {
    char filename[128];
    sprintf(filename, "history/match_history_%s.dat", username);
    
    FILE *fp = fopen(filename, "r");
    
    char response[BUFFER_SIZE * 4]; // Larger buffer for history
    int offset = sprintf(response, "{\"cmd\":\"MATCH_HISTORY\",\"payload\":{\"matches\":[");
    
    if (fp) {
        char line[256];
        int first = 1;
        int count = 0;
        
        // Read last 50 matches (or all if less)
        char matches[50][256];
        int match_count = 0;
        while (fgets(line, sizeof(line), fp) && match_count < 50) {
            strcpy(matches[match_count++], line);
        }
        fclose(fp);
        
        // Send in reverse order (newest first)
        for (int i = match_count - 1; i >= 0; i--) {
            long timestamp;
            char opponent[USERNAME_SIZE];
            char result[10];
            
            if (sscanf(matches[i], "%ld:%[^:]:%s", &timestamp, opponent, result) == 3) {
                if (!first) offset += sprintf(response + offset, ",");
                offset += sprintf(response + offset, 
                    "{\"timestamp\":%ld,\"opponent\":\"%s\",\"result\":\"%s\"}", 
                    timestamp, opponent, result);
                first = 0;
                count++;
            }
        }
    }
    
    offset += sprintf(response + offset, "]}}\n");
    send_message(sock, response);
}

void send_player_list(int sock) {
    char response[BUFFER_SIZE];
    int offset = sprintf(response, "{\"cmd\":\"PLAYER_LIST\",\"payload\":{\"players\":[");
    
    pthread_mutex_lock(&clients_mutex);
    int first = 1;
    int count = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && 
            clients[i]->status != PLAYER_OFFLINE && 
            clients[i]->status != PLAYER_IN_GAME &&
            clients[i]->sock != sock) {
            
            if (!first) offset += sprintf(response + offset, ",");
            int elo = get_player_elo(clients[i]->username);
            offset += sprintf(response + offset, 
                "{\"username\":\"%s\",\"status\":%d,\"elo\":%d}", 
                clients[i]->username, clients[i]->status, elo);
            first = 0;
            count++;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    offset += sprintf(response + offset, "]}}\n");
    printf("[PLAYER_LIST] Sending %d players to socket %d\n", count, sock);
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
    strncpy(session->player1_username, player1->username, USERNAME_SIZE - 1);
    strncpy(session->player2_username, player2->username, USERNAME_SIZE - 1);
    session->status = GAME_PLACING_SHIPS;
    session->start_time = time(NULL);
    session->player1_disconnected = 0;
    session->player2_disconnected = 0;
    session->player1_disconnect_time = 0;
    session->player2_disconnect_time = 0;
    sprintf(session->log_id, "game_%ld", session->start_time);
    
    // Update players
    player1->status = PLAYER_IN_GAME;
    player1->in_game_with = player2->sock;
    player1->ready = 0;
    player1->is_turn = 1;
    player1->last_active = time(NULL);
    init_board(&player1->board);
    
    player2->status = PLAYER_IN_GAME;
    player2->in_game_with = player1->sock;
    player2->ready = 0;
    player2->is_turn = 0;
    player2->last_active = time(NULL);
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
            char horizontal_str[10];
            int size, row, col, horizontal;
            
            // Extract ship data - parse horizontal as string first
            if (sscanf(ptr, "{\"name\":\"%[^\"]\",\"size\":%d,\"row\":%d,\"col\":%d,\"horizontal\":%[^,}]", 
                      name, &size, &row, &col, horizontal_str) == 5) {
                
                // Convert "true"/"false" string to int
                horizontal = (strstr(horizontal_str, "true") != NULL) ? 1 : 0;
                
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
    
    
    client->ready = 1;
    
    // Check if opponent is ready
    Client *opponent = get_client(client->in_game_with);
    if (opponent && opponent->ready) {
        // Both ready, start game
        char message[BUFFER_SIZE];
        
        // Find game session and update status
        pthread_mutex_lock(&games_mutex);
        for (int i = 0; i < MAX_CLIENTS / 2; i++) {
            if (game_sessions[i] && 
                ((game_sessions[i]->player1_sock == client->sock && game_sessions[i]->player2_sock == opponent->sock) ||
                 (game_sessions[i]->player2_sock == client->sock && game_sessions[i]->player1_sock == opponent->sock))) {
                game_sessions[i]->status = GAME_PLAYING;
                game_sessions[i]->current_turn = game_sessions[i]->player1_sock;
                
                // Set turn flags correctly
                Client *p1 = get_client(game_sessions[i]->player1_sock);
                Client *p2 = get_client(game_sessions[i]->player2_sock);
                if (p1 && p2) {
                    p1->is_turn = 1;
                    p2->is_turn = 0;
                    
                    // Notify both players
                    sprintf(message, "{\"cmd\":\"GAME_READY\",\"payload\":{\"message\":\"Game starting!\",\"your_turn\":true}}\n");
                    send_message(p1->sock, message);
                    
                    sprintf(message, "{\"cmd\":\"GAME_READY\",\"payload\":{\"message\":\"Game starting!\",\"your_turn\":false}}\n");
                    send_message(p2->sock, message);
                }
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
    // Check if client has placed ships
    if (!client->ready || client->board.total_ship_cells == 0) {
        char response[BUFFER_SIZE];
        sprintf(response, "{\"cmd\":\"SYSTEM_MSG\",\"payload\":{\"code\":400,\"message\":\"You haven't placed your ships yet\"}}\n");
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
    if (!opponent->ready || opponent->board.total_ship_cells == 0) {
        char response[BUFFER_SIZE];
        sprintf(response, "{\"cmd\":\"SYSTEM_MSG\",\"payload\":{\"code\":400,\"message\":\"Opponent hasn't placed ships yet\"}}\n");
        send_message(client->sock, response);
        return;
    }
    
    // Check if it's the player's turn
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
    
    // Check hit or miss
    int cell = opponent->board.grid[row][col];
    char result[20];
    char ship_sunk[30] = "";
    
    if (cell == 1) {
        strcpy(result, "HIT");
        opponent->board.grid[row][col] = 2; // mark as hit
        opponent->board.hits_received++;
        
        // Find which ship was hit and increment its hit counter
        for (int i = 0; i < opponent->board.ship_count; i++) {
            Ship *ship = &opponent->board.ships[i];
            // Check if this coordinate belongs to this ship
            for (int j = 0; j < ship->size; j++) {
                int r = ship->start_row + (ship->is_horizontal ? 0 : j);
                int c = ship->start_col + (ship->is_horizontal ? j : 0);
                if (r == row && c == col) {
                    // This is the ship that was hit
                    ship->hits++;
                    // Only report sunk if this hit completes the ship (prevents duplicate sunk messages)
                    if (ship->hits == ship->size) {
                        strcpy(ship_sunk, ship->name);
                    }
                    break;
                }
            }
            if (ship_sunk[0] != '\0') break; // Found the ship, no need to check others
        }
    } else if (cell == 0) {
        strcpy(result, "MISS");
        opponent->board.grid[row][col] = 3; // mark as miss
    } else {
        strcpy(result, "ALREADY_HIT");
    }
    
    // Check game end FIRST - opponent must have ships and all ships sunk
    if (opponent->board.total_ship_cells > 0 && 
        opponent->board.hits_received >= opponent->board.total_ship_cells) {
        // Game is over - include final ship sunk info in GAME_END message
        pthread_mutex_lock(&games_mutex);
        GameSession *session = NULL;
        for (int i = 0; i < MAX_CLIENTS / 2; i++) {
            if (game_sessions[i] && 
                ((game_sessions[i]->player1_sock == client->sock && game_sessions[i]->player2_sock == opponent->sock) ||
                 (game_sessions[i]->player2_sock == client->sock && game_sessions[i]->player1_sock == opponent->sock))) {
                session = game_sessions[i];
                break;
            }
        }
        pthread_mutex_unlock(&games_mutex);
        
        if (session) {
            // Send final MOVE_RESULT with game_over flag to suppress popup
            char message[BUFFER_SIZE];
            sprintf(message, "{\"cmd\":\"MOVE_RESULT\",\"payload\":{\"coord\":\"%s\",\"result\":\"%s\",\"ship_sunk\":\"%s\",\"is_your_shot\":true,\"game_over\":true}}\n", 
                    coord, result, ship_sunk);
            send_message(client->sock, message);
            
            sprintf(message, "{\"cmd\":\"MOVE_RESULT\",\"payload\":{\"coord\":\"%s\",\"result\":\"%s\",\"ship_sunk\":\"%s\",\"is_your_shot\":false,\"game_over\":true}}\n", 
                    coord, result, ship_sunk);
            send_message(opponent->sock, message);
            
            // Now end the game - end_game() will handle free() and cleanup
            end_game(session, client->sock, "ALL_SHIPS_SUNK");
        }
        return;
    }
    
    // Game continues - send normal MOVE_RESULT
    char message[BUFFER_SIZE];
    sprintf(message, "{\"cmd\":\"MOVE_RESULT\",\"payload\":{\"coord\":\"%s\",\"result\":\"%s\",\"ship_sunk\":\"%s\",\"is_your_shot\":true}}\n", 
            coord, result, ship_sunk);
    send_message(client->sock, message);
    
    sprintf(message, "{\"cmd\":\"MOVE_RESULT\",\"payload\":{\"coord\":\"%s\",\"result\":\"%s\",\"ship_sunk\":\"%s\",\"is_your_shot\":false}}\n", 
            coord, result, ship_sunk);
    send_message(opponent->sock, message);
    
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
    
    // Update ELO ratings and save match history
    if (winner && loser) {
        update_player_stats(winner->username, 10, 1);  // +10 ELO, win
        update_player_stats(loser->username, -10, 0);  // -10 ELO, lose
        
        // Save match history for both players
        save_match_history(winner->username, loser->username, "WIN");
        save_match_history(loser->username, winner->username, "LOSE");
        
        printf("[ELO] %s +10, %s -10\n", winner->username, loser->username);
    }
    
    if (winner) {
        int new_elo = get_player_elo(winner->username);
        char message[BUFFER_SIZE];
        sprintf(message, "{\"cmd\":\"GAME_END\",\"payload\":{\"result\":\"WIN\",\"reason\":\"%s\",\"log_id\":\"%s\",\"elo\":%d}}\n", 
                reason, session->log_id, new_elo);
        send_message(winner->sock, message);
        
        // Reset winner state completely
        winner->status = PLAYER_ONLINE;
        winner->in_game_with = 0;
        winner->ready = 0;
        winner->is_turn = 0;
        winner->is_matching = 0;
        winner->match_ready = 0;
        init_board(&winner->board);
        
        printf("[END_GAME] %s wins, status set to ONLINE, ELO: %d\n", winner->username, new_elo);
    }
    
    if (loser) {
        int new_elo = get_player_elo(loser->username);
        char message[BUFFER_SIZE];
        sprintf(message, "{\"cmd\":\"GAME_END\",\"payload\":{\"result\":\"LOSE\",\"reason\":\"%s\",\"log_id\":\"%s\",\"elo\":%d}}\n", 
                reason, session->log_id, new_elo);
        send_message(loser->sock, message);
        
        // Reset loser state completely
        loser->status = PLAYER_ONLINE;
        loser->in_game_with = 0;
        loser->ready = 0;
        loser->is_turn = 0;
        loser->is_matching = 0;
        loser->match_ready = 0;
        init_board(&loser->board);
        
        printf("[END_GAME] %s loses, status set to ONLINE, ELO: %d\n", loser->username, new_elo);
    }
    
    // Remove game session
    pthread_mutex_lock(&games_mutex);
    for (int i = 0; i < MAX_CLIENTS / 2; i++) {
        if (game_sessions[i] == session) {
            free(game_sessions[i]);
            game_sessions[i] = NULL;
            printf("[END_GAME] Game session removed\n");
            break;
        }
    }
    pthread_mutex_unlock(&games_mutex);
    
    printf("Game ended: %s\n", reason);
}

// Helper function to handle game cleanup (used by both disconnect and logout)
void cleanup_game_on_exit(Client *client, int notify_opponent) {
    if ((client->status == PLAYER_IN_GAME || client->status == PLAYER_IN_LOBBY) && client->in_game_with > 0) {
        Client *opponent = get_client(client->in_game_with);
        
        if (opponent && notify_opponent) {
            // Determine phase for better messaging
            const char *phase = (client->status == PLAYER_IN_LOBBY) ? "đặt thuyền" : "chơi game";
            
            printf("[GAME_CLEANUP] %s left during %s - %s wins\n", 
                   client->username, phase, opponent->username);
            
            // Save match history
            save_match_history(opponent->username, client->username, "WIN");
            save_match_history(client->username, opponent->username, "LOSE");
            
            // Update ELO - opponent wins
            update_player_stats(opponent->username, 10, 1);  // Winner +10
            update_player_stats(client->username, -10, 0);   // Loser -10
            
            int new_elo = get_player_elo(opponent->username);
            
            // Send WIN notification to opponent
            char message[BUFFER_SIZE];
            sprintf(message, "{\"cmd\":\"GAME_END\",\"payload\":{\"result\":\"WIN\",\"reason\":\"OPPONENT_DISCONNECTED\",\"opponent\":\"%s\",\"message\":\"Đối thủ đã ngắt kết nối. Bạn thắng!\",\"elo\":%d}}\n", 
                    client->username, new_elo);
            send_message(opponent->sock, message);
            
            // Reset opponent state to online
            opponent->status = PLAYER_ONLINE;
            opponent->in_game_with = 0;
            opponent->ready = 0;
            opponent->is_turn = 0;
            opponent->is_matching = 0;
            opponent->match_ready = 0;
            init_board(&opponent->board);
        }
        
        // Remove game session
        pthread_mutex_lock(&games_mutex);
        for (int i = 0; i < MAX_CLIENTS / 2; i++) {
            if (game_sessions[i] && 
                (game_sessions[i]->player1_sock == client->sock || 
                 game_sessions[i]->player2_sock == client->sock)) {
                free(game_sessions[i]);
                game_sessions[i] = NULL;
                printf("[GAME_CLEANUP] Game session removed\n");
                break;
            }
        }
        pthread_mutex_unlock(&games_mutex);
    }
}

void handle_disconnect(Client *client) {
    printf("[DISCONNECT] %s disconnected (sock %d, status %d)\n", 
           client->username, client->sock, client->status);
    
    // Cleanup game with opponent notification
    cleanup_game_on_exit(client, 1);
    
    // Mark client as offline (DON'T invalidate socket here - thread will close it)
    client->status = PLAYER_OFFLINE;
}

void handle_surrender(Client *client) {
    printf("[SURRENDER] %s wants to surrender\n", client->username);
    if (client->status != PLAYER_IN_GAME || client->in_game_with == 0) {
        char message[BUFFER_SIZE];
        sprintf(message, "{\"cmd\":\"ERROR\",\"payload\":{\"message\":\"Not in a game\"}}\n");
        send_message(client->sock, message);
        return;
    }

    Client *opponent = get_client(client->in_game_with);
    if (!opponent) {
        printf("[SURRENDER] No opponent found\n");
        return;
    }
    printf("[SURRENDER] Ending game, opponent %s wins\n", opponent->username);

    // Find game session
    GameSession *session = NULL;
    pthread_mutex_lock(&games_mutex);
    for (int i = 0; i < MAX_CLIENTS / 2; i++) {
        if (game_sessions[i] && 
            (game_sessions[i]->player1_sock == client->sock || game_sessions[i]->player2_sock == client->sock)) {
            session = game_sessions[i];
            break;
        }
    }
    pthread_mutex_unlock(&games_mutex);

    if (session) {
        end_game(session, opponent->sock, "SURRENDER");
    }
}

void handle_draw_offer(Client *client) {
    printf("[DRAW_OFFER] %s offers draw\n", client->username);
    if (client->status != PLAYER_IN_GAME || client->in_game_with == 0) {
        char message[BUFFER_SIZE];
        sprintf(message, "{\"cmd\":\"ERROR\",\"payload\":{\"message\":\"Not in a game\"}}\n");
        send_message(client->sock, message);
        return;
    }

    Client *opponent = get_client(client->in_game_with);
    if (!opponent) {
        printf("[DRAW_OFFER] No opponent found\n");
        return;
    }

    // Send draw offer to opponent
    char message[BUFFER_SIZE];
    sprintf(message, "{\"cmd\":\"DRAW_OFFER\",\"payload\":{\"from\":\"%s\"}}\n", client->username);
    printf("[DRAW_OFFER] Sending to %s (sock %d): %s", opponent->username, opponent->sock, message);
    send_message(opponent->sock, message);
}

void handle_draw_reply(Client *client, const char *status) {
    printf("[DRAW_REPLY] %s replies: %s\n", client->username, status);
    if (client->status != PLAYER_IN_GAME || client->in_game_with == 0) {
        return;
    }

    Client *opponent = get_client(client->in_game_with);
    if (!opponent) {
        printf("[DRAW_REPLY] No opponent found\n");
        return;
    }

    if (strcmp(status, "accept") == 0) {
        printf("[DRAW_REPLY] Draw accepted, ending game\n");
        // Find game session and end as draw
        GameSession *session = NULL;
        pthread_mutex_lock(&games_mutex);
        for (int i = 0; i < MAX_CLIENTS / 2; i++) {
            if (game_sessions[i] && 
                (game_sessions[i]->player1_sock == client->sock || game_sessions[i]->player2_sock == client->sock)) {
                session = game_sessions[i];
                break;
            }
        }
        pthread_mutex_unlock(&games_mutex);

        if (session) {
            // Save match history for both players as DRAW
            save_match_history(client->username, opponent->username, "DRAW");
            save_match_history(opponent->username, client->username, "DRAW");
            
            // No ELO change for draw
            int client_elo = get_player_elo(client->username);
            int opponent_elo = get_player_elo(opponent->username);
            
            // Send DRAW to both players with current ELO
            char message[BUFFER_SIZE];
            sprintf(message, "{\"cmd\":\"GAME_END\",\"payload\":{\"result\":\"DRAW\",\"reason\":\"DRAW_ACCEPTED\",\"log_id\":\"\",\"elo\":%d}}\n", client_elo);
            send_message(client->sock, message);
            sprintf(message, "{\"cmd\":\"GAME_END\",\"payload\":{\"result\":\"DRAW\",\"reason\":\"DRAW_ACCEPTED\",\"log_id\":\"\",\"elo\":%d}}\n", opponent_elo);
            send_message(opponent->sock, message);

            client->status = PLAYER_ONLINE;
            client->in_game_with = 0;
            opponent->status = PLAYER_ONLINE;
            opponent->in_game_with = 0;

            pthread_mutex_lock(&games_mutex);
            for (int i = 0; i < MAX_CLIENTS / 2; i++) {
                if (game_sessions[i] == session) {
                    free(game_sessions[i]);
                    game_sessions[i] = NULL;
                    break;
                }
            }
            pthread_mutex_unlock(&games_mutex);
        }
    } else {
        // Reject - notify opponent
        char message[BUFFER_SIZE];
        sprintf(message, "{\"cmd\":\"DRAW_REJECTED\",\"payload\":{}}\n");
        send_message(opponent->sock, message);
    }
}

// Matching functions
void handle_start_matching(Client *client) {
    if (client->status != PLAYER_ONLINE) {
        char response[BUFFER_SIZE];
        sprintf(response, "{\"cmd\":\"SYSTEM_MSG\",\"payload\":{\"code\":400,\"message\":\"Cannot start matching\"}}\n");
        send_message(client->sock, response);
        return;
    }
    
    client->is_matching = 1;
    client->status = PLAYER_IN_LOBBY;
    
    char response[BUFFER_SIZE];
    sprintf(response, "{\"cmd\":\"MATCHING_STARTED\",\"payload\":{\"message\":\"Đang tìm đối thủ...\"}}\n");
    send_message(client->sock, response);
    
    printf("[MATCHING] %s started matching (ELO: %d)\n", client->username, get_player_elo(client->username));
    
    // Try to match immediately
    try_match_players();
}

void handle_cancel_matching(Client *client) {
    if (!client->is_matching) {
        return;
    }
    
    client->is_matching = 0;
    client->status = PLAYER_ONLINE;
    
    char response[BUFFER_SIZE];
    sprintf(response, "{\"cmd\":\"MATCHING_CANCELLED\",\"payload\":{\"message\":\"Đã hủy tìm trận\"}}\n");
    send_message(client->sock, response);
    
    printf("[MATCHING] %s cancelled matching\n", client->username);
}

void try_match_players() {
    pthread_mutex_lock(&clients_mutex);
    
    // Find all players in matching queue
    Client *matching_players[MAX_CLIENTS];
    int matching_count = 0;
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->is_matching) {
            matching_players[matching_count++] = clients[i];
        }
    }
    
    // Try to match players with similar ELO (±100)
    for (int i = 0; i < matching_count - 1; i++) {
        if (!matching_players[i]->is_matching) continue; // Already matched
        
        int player1_elo = get_player_elo(matching_players[i]->username);
        
        for (int j = i + 1; j < matching_count; j++) {
            if (!matching_players[j]->is_matching) continue;
            
            int player2_elo = get_player_elo(matching_players[j]->username);
            int elo_diff = abs(player1_elo - player2_elo);
            
            if (elo_diff <= 100) {
                // Match found!
                Client *p1 = matching_players[i];
                Client *p2 = matching_players[j];
                
                p1->is_matching = 0;
                p2->is_matching = 0;
                p1->match_ready = 0;
                p2->match_ready = 0;
                p1->in_game_with = p2->sock;
                p2->in_game_with = p1->sock;
                p1->status = PLAYER_IN_LOBBY;
                p2->status = PLAYER_IN_LOBBY;
                
                printf("[MATCHING] Matched %s (ELO: %d) with %s (ELO: %d)\n",
                       p1->username, player1_elo, p2->username, player2_elo);
                
                // Send match found notification
                char message[BUFFER_SIZE];
                sprintf(message, "{\"cmd\":\"MATCH_FOUND\",\"payload\":{\"opponent\":\"%s\",\"elo\":%d}}\n", 
                        p2->username, player2_elo);
                send_message(p1->sock, message);
                
                sprintf(message, "{\"cmd\":\"MATCH_FOUND\",\"payload\":{\"opponent\":\"%s\",\"elo\":%d}}\n", 
                        p1->username, player1_elo);
                send_message(p2->sock, message);
                
                break;
            }
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

void handle_match_ready(Client *client) {
    if (client->in_game_with == 0) {
        char response[BUFFER_SIZE];
        sprintf(response, "{\"cmd\":\"SYSTEM_MSG\",\"payload\":{\"code\":400,\"message\":\"No match found\"}}\n");
        send_message(client->sock, response);
        return;
    }
    
    client->match_ready = 1;
    
    Client *opponent = get_client(client->in_game_with);
    if (!opponent) {
        return;
    }
    
    // Notify opponent that this player is ready
    char message[BUFFER_SIZE];
    sprintf(message, "{\"cmd\":\"OPPONENT_READY\",\"payload\":{\"username\":\"%s\"}}\n", client->username);
    send_message(opponent->sock, message);
    
    // Check if both players are ready
    if (opponent->match_ready) {
        printf("[MATCH_READY] Both players ready, starting game: %s vs %s\n", 
               client->username, opponent->username);
        
        // Reset match_ready flags
        client->match_ready = 0;
        opponent->match_ready = 0;
        
        // Start the game
        start_game(client, opponent);
    } else {
        sprintf(message, "{\"cmd\":\"WAITING_OPPONENT\",\"payload\":{\"message\":\"Đang chờ đối thủ sẵn sàng...\"}}\n");
        send_message(client->sock, message);
    }
}

void handle_match_decline(Client *client) {
    if (client->in_game_with == 0) {
        return;
    }
    
    printf("[MATCH_DECLINE] %s declined match\n", client->username);
    
    Client *opponent = get_client(client->in_game_with);
    if (opponent) {
        // Notify opponent
        char message[BUFFER_SIZE];
        sprintf(message, "{\"cmd\":\"MATCH_DECLINED\",\"payload\":{\"message\":\"Đối thủ đã từ chối trận đấu\"}}\n");
        send_message(opponent->sock, message);
        
        // Reset opponent state
        opponent->in_game_with = 0;
        opponent->match_ready = 0;
        opponent->is_matching = 0;
        opponent->status = PLAYER_ONLINE;
    }
    
    // Send confirmation to the person who declined
    char response[BUFFER_SIZE];
    sprintf(response, "{\"cmd\":\"MATCH_DECLINED\",\"payload\":{\"message\":\"Bạn đã từ chối trận đấu\"}}\n");
    send_message(client->sock, response);
    
    // Reset this player's state
    client->in_game_with = 0;
    client->match_ready = 0;
    client->is_matching = 0;
    client->status = PLAYER_ONLINE;
}

void handle_leaderboard(Client *client) {
    FILE *fp = fopen("users.dat", "r");
    if (!fp) {
        char response[BUFFER_SIZE];
        sprintf(response, "{\"cmd\":\"LEADERBOARD\",\"payload\":{\"players\":[]}}\n");
        send_message(client->sock, response);
        return;
    }
    
    // Read all players
    PlayerAccount players[MAX_CLIENTS];
    int player_count = 0;
    
    char line[256];
    while (fgets(line, sizeof(line), fp) && player_count < MAX_CLIENTS) {
        sscanf(line, "%[^:]:%[^:]:%d:%d:%d", 
               players[player_count].username, 
               players[player_count].password,
               &players[player_count].elo,
               &players[player_count].games_played,
               &players[player_count].games_won);
        player_count++;
    }
    fclose(fp);
    
    // Sort by ELO (descending)
    for (int i = 0; i < player_count - 1; i++) {
        for (int j = i + 1; j < player_count; j++) {
            if (players[j].elo > players[i].elo) {
                PlayerAccount temp = players[i];
                players[i] = players[j];
                players[j] = temp;
            }
        }
    }
    
    // Build JSON response
    char response[BUFFER_SIZE * 2];
    char players_json[BUFFER_SIZE * 2] = "[";
    
    for (int i = 0; i < player_count && i < 50; i++) { // Top 50
        char player_entry[256];
        double winrate = players[i].games_played > 0 
            ? (players[i].games_won * 100.0 / players[i].games_played) 
            : 0.0;
        
        sprintf(player_entry, "%s{\"rank\":%d,\"username\":\"%s\",\"elo\":%d,\"games\":%d,\"wins\":%d,\"winrate\":%.1f}",
                i > 0 ? "," : "",
                i + 1,
                players[i].username,
                players[i].elo,
                players[i].games_played,
                players[i].games_won,
                winrate);
        strcat(players_json, player_entry);
    }
    strcat(players_json, "]");
    
    sprintf(response, "{\"cmd\":\"LEADERBOARD\",\"payload\":{\"players\":%s}}\n", players_json);
    send_message(client->sock, response);
    
    printf("[LEADERBOARD] Sent top %d players to %s\n", player_count < 50 ? player_count : 50, client->username);
}

void handle_logout(Client *client) {
    time_t now = time(NULL);
    printf("[LOGOUT] User: %s\n", client->username);
    printf("[LOGOUT] Socket: %d\n", client->sock);
    printf("[LOGOUT] Status: %d\n", client->status);
    printf("[LOGOUT] Time: %s", ctime(&now));
    
    // If in game or lobby, cleanup game (notify opponent)
    if ((client->status == PLAYER_IN_GAME || client->status == PLAYER_IN_LOBBY) && client->in_game_with > 0) {
        printf("[LOGOUT] %s is in game - cleaning up game\n", client->username);
        cleanup_game_on_exit(client, 1);
    }
    
    // Clear session and reset state
    memset(client->session_token, 0, sizeof(client->session_token));
    client->status = PLAYER_OFFLINE;
    client->in_game_with = 0;
    client->ready = 0;
    client->is_turn = 0;
    client->is_matching = 0;
    client->match_ready = 0;
    init_board(&client->board);
    
    char response[BUFFER_SIZE];
    sprintf(response, "{\"cmd\":\"LOGOUT_SUCCESS\",\"payload\":{\"message\":\"Logged out successfully\"}}\n");
    send_message(client->sock, response);
    
    printf("[LOGOUT] %s logout completed\n", client->username);
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
            client->last_active = time(NULL);
            generate_session_token(client->session_token);
            
            int elo = get_player_elo(username);
            char response[BUFFER_SIZE];
            sprintf(response, "{\"cmd\":\"LOGIN_SUCCESS\",\"payload\":{\"username\":\"%s\",\"message\":\"Welcome!\",\"elo\":%d,\"sessionToken\":\"%s\"}}\n", 
                    username, elo, client->session_token);
            send_message(client->sock, response);
            
            printf("User logged in: %s (socket %d, ELO: %d, token: %s)\n", username, client->sock, elo, client->session_token);
        } else {
            char response[BUFFER_SIZE];
            sprintf(response, "{\"cmd\":\"SYSTEM_MSG\",\"payload\":{\"code\":401,\"message\":\"Invalid credentials\"}}\n");
            send_message(client->sock, response);
        }
    }
    else if (strcmp(cmd, "PLAYER_LIST") == 0) {
        send_player_list(client->sock);
    }
    else if (strcmp(cmd, "MATCH_HISTORY") == 0) {
        send_match_history(client->sock, client->username);
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
        
        printf("[CHAT] From: %s, Message: %s\n", client->username, message);
        
        Client *opponent = get_client(client->in_game_with);
        if (opponent) {
            printf("[CHAT] Sending to opponent: %s (sock %d)\n", opponent->username, opponent->sock);
            char response[BUFFER_SIZE];
            sprintf(response, "{\"cmd\":\"CHAT\",\"payload\":{\"from\":\"%s\",\"message\":\"%s\"}}\n", 
                    client->username, message);
            send_message(opponent->sock, response);
        } else {
            printf("[CHAT] No opponent found for %s\n", client->username);
        }
    }
    else if (strcmp(cmd, "SURRENDER") == 0) {
        handle_surrender(client);
    }
    else if (strcmp(cmd, "DRAW_OFFER") == 0) {
        handle_draw_offer(client);
    }
    else if (strcmp(cmd, "DRAW_REPLY") == 0) {
        char status[20];
        sscanf(payload, "{\"status\":\"%[^\"]\"}", status);
        handle_draw_reply(client, status);
    }
    else if (strcmp(cmd, "START_MATCHING") == 0) {
        handle_start_matching(client);
    }
    else if (strcmp(cmd, "CANCEL_MATCHING") == 0) {
        handle_cancel_matching(client);
    }
    else if (strcmp(cmd, "MATCH_READY") == 0) {
        handle_match_ready(client);
    }
    else if (strcmp(cmd, "MATCH_DECLINE") == 0) {
        handle_match_decline(client);
    }
    else if (strcmp(cmd, "LEADERBOARD") == 0) {
        handle_leaderboard(client);
    }
    else if (strcmp(cmd, "LOGOUT") == 0) {
        handle_logout(client);
    }
    else if (strcmp(cmd, "PING") == 0) {
        // Respond with PONG immediately
        char response[BUFFER_SIZE];
        sprintf(response, "{\"cmd\":\"PONG\",\"payload\":{\"timestamp\":%ld}}\n", time(NULL));
        send_message(client->sock, response);
    }
    else if (strcmp(cmd, "UPDATE_PING") == 0) {
        // Client sends its calculated ping
        int ping_value = 0;
        sscanf(payload, "{\"ping\":%d}", &ping_value);
        client->ping = ping_value;
        
        printf("[UPDATE_PING] %s ping: %d ms, status: %d, in_game_with: %d\n", 
               client->username, ping_value, client->status, client->in_game_with);
        
        // If in game, broadcast ping update to opponent
        if (client->status == PLAYER_IN_GAME && client->in_game_with > 0) {
            pthread_mutex_lock(&clients_mutex);
            Client *opponent = NULL;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] != NULL && clients[i]->sock == client->in_game_with) {
                    opponent = clients[i];
                    break;
                }
            }
            
            if (opponent != NULL) {
                char ping_update[BUFFER_SIZE];
                sprintf(ping_update, "{\"cmd\":\"PING_UPDATE\",\"payload\":{\"opponent_ping\":%d}}\n", client->ping);
                send_message(opponent->sock, ping_update);
                printf("[UPDATE_PING] Sent to opponent %s: ping=%d\n", opponent->username, client->ping);
            } else {
                printf("[UPDATE_PING] Opponent not found for %s\n", client->username);
            }
            pthread_mutex_unlock(&clients_mutex);
        } else {
            printf("[UPDATE_PING] %s not in game, skipping broadcast\n", client->username);
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
    printf("║   Disconnect = Instant Loss          ║\n");
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
        memset(client->session_token, 0, sizeof(client->session_token));
        client->last_active = time(NULL);
        client->address = address;
        client->in_game_with = 0;
        client->ready = 0;
        client->is_turn = 0;
        client->is_matching = 0;
        client->match_ready = 0;
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