#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "bitops.h" 

typedef struct {
    unsigned long long red_pieces;
    unsigned long long black_pieces;
    unsigned long long red_kings;
    unsigned long long black_kings;
    int current_turn; // 0 = Red, 1 = Black
} Game;

// board controller
int coord_to_pos(const char *coord) {
    if (strlen(coord) < 2) return -1;
    char file = tolower(coord[0]);
    char rank = coord[1];
    if (file < 'a' || file > 'h' || rank < '1' || rank > '8') return -1;
    int col = file - 'a';
    int row = rank - '1';
    int pos = row * 8 + col;
    if ((row + col) % 2 == 0) return -1;
    return pos;
}

void pos_to_coord(int pos, char *out) {
    int row = pos / 8;
    int col = pos % 8;
    out[0] = 'a' + col;
    out[1] = '1' + row;
    out[2] = '\0';
}

int pos_row(int pos) { return pos / 8; }
int pos_col(int pos) { return pos % 8; }

// create and print board
unsigned long long generate_start_positions(int isRed) {
    unsigned long long board = 0ULL;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int pos = row * 8 + col;
            if ((row + col) % 2 == 1) {
                if (isRed && row < 3)
                    board = ModifyBit(board, pos, 1); 
                else if (!isRed && row > 4)
                    board = ModifyBit(board, pos, 1); 
            }
        }
    }
    return board;
}

void PrintBoard(unsigned long long red, unsigned long long black, unsigned long long red_kings, unsigned long long black_kings) {
    printf("\n");
    for (int row = 7; row >= 0; row--) {
        printf("%d ", row + 1);
        for (int col = 0; col < 8; col++) {
            int pos = row * 8 + col;
            if ((row + col) % 2 == 0) {
                printf("   ");
            } else if (GetBit(red, pos)) { 
                printf(GetBit(red_kings, pos) ? " Rk" : " R "); 
            } else if (GetBit(black, pos)) { 
                printf(GetBit(black_kings, pos) ? " Bk" : " B "); 
            } else {
                printf(" . ");
            }
        }
        printf("\n");
    }
    printf("    a  b  c  d  e  f  g  h\n");
}


void saveGame(Game *game, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Could not save game.\n");
        return;
    }
    fwrite(game, sizeof(Game), 1, file);
    fclose(file);
    printf("Game saved to %s\n", filename);
}

void loadGame(Game *game, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not load game.\n");
        return;
    }
    fread(game, sizeof(Game), 1, file);
    fclose(file);
    printf("Game loaded from %s\n", filename);
}


bool is_occupied(Game *g, int pos) {
    unsigned long long all_pieces = g->red_pieces | g->black_pieces;
    return GetBit(all_pieces, pos); 
}

bool is_opponent_piece(Game *g, int pos, int player) {
    if (player == 0) return GetBit(g->black_pieces, pos); 
    else return GetBit(g->red_pieces, pos); 
}

bool is_player_piece(Game *g, int pos, int player) {
    if (player == 0) return GetBit(g->red_pieces, pos); 
    else return GetBit(g->black_pieces, pos); 
}

bool is_king(Game *g, int pos, int player) {
    if (player == 0) return GetBit(g->red_kings, pos); 
    else return GetBit(g->black_kings, pos); 
}

bool valid_shift(int pos, int delta) {
    int r = pos_row(pos), c = pos_col(pos);
    int newpos = pos + delta;
    if (newpos < 0 || newpos > 63) return false;
    int nr = pos_row(newpos), nc = pos_col(newpos);
    int rowdiff = abs(nr - r), coldiff = abs(nc - c);
    if (delta == 7 || delta == 9 || delta == -7 || delta == -9) {
        return rowdiff == 1 && coldiff == 1;
    } else if (delta == 14 || delta == 18 || delta == -14 || delta == -18) {
        return rowdiff == 2 && coldiff == 2;
    }
    return false;
}

bool can_simple_move_from(Game *g, int pos, int player, int delta) {
    if (!valid_shift(pos, delta)) return false;
    return !is_occupied(g, pos + delta);
}

bool can_capture_from(Game *g, int pos, int player, int over_delta, int land_delta) {
    if (!valid_shift(pos, over_delta) || !valid_shift(pos, land_delta)) return false;
    int over = pos + over_delta;
    int land = pos + land_delta;
    if (!is_opponent_piece(g, over, player)) return false;
    if (is_occupied(g, land)) return false;
    return true;
}

bool any_capture_available(Game *g) {
    int player = g->current_turn;
    unsigned long long pieces = (player == 0) ? g->red_pieces : g->black_pieces;
    for (int pos = 0; pos < 64; pos++) {
        if (!GetBit(pieces, pos)) continue; 
        bool king = is_king(g, pos, player);
        int deltas_simple[4] = {7, 9, -7, -9};
        int deltas_over[4]   = {14, 18, -14, -18};
        for (int i = 0; i < 4; i++) {
            int over = deltas_simple[i];
            int land = deltas_over[i];
            if (!king) {
                if (player == 0 && over < 0) continue;
                if (player == 1 && over > 0) continue;
            }
            if (can_capture_from(g, pos, player, over, land)) return true;
        }
    }
    return false;
}

bool any_legal_simple_move(Game *g, int player) {
    unsigned long long pieces = (player == 0) ? g->red_pieces : g->black_pieces;
    for (int pos = 0; pos < 64; pos++) {
        if (!GetBit(pieces, pos)) continue; 
        bool king = is_king(g, pos, player);
        int deltas_simple[4] = {7, 9, -7, -9};
        for (int i = 0; i < 4; i++) {
            int d = deltas_simple[i];
            if (!king && ((player == 0 && d < 0) || (player == 1 && d > 0))) continue;
            if (can_simple_move_from(g, pos, player, d)) return true;
        }
    }
    return false;
}

// Piece movement
void remove_piece_at(Game *g, int pos, int player) {
    if (player == 0) { 
        g->black_pieces = ModifyBit(g->black_pieces, pos, 0); 
        g->black_kings = ModifyBit(g->black_kings, pos, 0);   
    } else { 
        g->red_pieces = ModifyBit(g->red_pieces, pos, 0); 
        g->red_kings = ModifyBit(g->red_kings, pos, 0);   
    }
}

void move_piece(Game *g, int from, int to, int player) {
    bool king = is_king(g, from, player);
    if (player == 0) { // Red
        g->red_pieces = ModifyBit(g->red_pieces, from, 0);
        g->red_pieces = ModifyBit(g->red_pieces, to, 1);
        if (king) {
            g->red_kings = ModifyBit(g->red_kings, from, 0);
            g->red_kings = ModifyBit(g->red_kings, to, 1);
        }
    } else { // Black
        g->black_pieces = ModifyBit(g->black_pieces, from, 0);
        g->black_pieces = ModifyBit(g->black_pieces, to, 1);
        if (king) {
            g->black_kings = ModifyBit(g->black_kings, from, 0);
            g->black_kings = ModifyBit(g->black_kings, to, 1);
        }
    }
}

void promote_if_needed(Game *g, int pos, int player) {
    int row = pos_row(pos);
    if (player == 0 && row == 7) {
        g->red_kings = ModifyBit(g->red_kings, pos, 1); 
    } else if (player == 1 && row == 0) {
        g->black_kings = ModifyBit(g->black_kings, pos, 1); 
    }
}

bool attempt_move(Game *g, int from, int to) {
    if (from < 0 || from > 63 || to < 0 || to > 63) return false;
    int player = g->current_turn;
    if (!is_player_piece(g, from, player)) { printf("Invalid move: Not your piece.\n"); return false; }
    if (is_occupied(g, to)) { printf("Invalid move: Destination occupied.\n"); return false; }
    bool king = is_king(g, from, player);
    int diff = to - from;
    bool is_simple = (abs(diff) == 7 || abs(diff) == 9);
    bool is_jump = (abs(diff) == 14 || abs(diff) == 18);
    if (is_simple && any_capture_available(g)) { printf("You must capture when available.\n"); return false; }
    if (!king && ((player == 0 && diff < 0) || (player == 1 && diff > 0))) { printf("Invalid move: Men only move forward.\n"); return false; }

    if (is_simple) {
        if (!valid_shift(from, diff)) { printf("Invalid simple move.\n"); return false; }
        move_piece(g, from, to, player);
        promote_if_needed(g, to, player);
        return true;
    } else if (is_jump) {
        int over = (from + to) / 2;
        if (!valid_shift(from, over - from)) { printf("Invalid jump path.\n"); return false; }
        if (!is_opponent_piece(g, over, player)) { printf("Invalid jump: No opponent piece to capture.\n"); return false; }
        move_piece(g, from, to, player);
        remove_piece_at(g, over, player);
        promote_if_needed(g, to, player);
        return true;
    }
    printf("Invalid move distance.\n");
    return false;
}

// win detection
bool check_win(Game *g, int *winner_out) {
    int red_count = CountBits(g->red_pieces); 
    int black_count = CountBits(g->black_pieces); 
    if (red_count == 0) { *winner_out = 1; return true; }
    if (black_count == 0) { *winner_out = 0; return true; }
    if (!any_legal_simple_move(g, g->current_turn) && !any_capture_available(g)) {
        *winner_out = 1 - g->current_turn;
        return true;
    }
    return false;
}

// menu 
void prompt_move_and_execute(Game *g) {
    char from_s[8], to_s[8];
    printf("Enter move (e.g. b6 c5): ");
    if (scanf("%7s %7s", from_s, to_s) != 2) {
        int c; while ((c = getchar()) != '\n' && c != EOF);
        printf("Invalid input format.\n");
        return;
    }
    int from = coord_to_pos(from_s);
    int to = coord_to_pos(to_s);

    if (from == -1 || to == -1) {
        printf("Invalid coordinates.\n");
        return;
    }
    
    // if successful, switch turn. 
    if (attempt_move(g, from, to)) {
        g->current_turn = 1 - g->current_turn;
    }
}

void menu(Game *game) {
    int choice = 0;
    char filename[128];
    while (choice != 6) {
        int winner;
        if (check_win(game, &winner)) {
            PrintBoard(game->red_pieces, game->black_pieces, game->red_kings, game->black_kings);
            printf("GAME OVER: %s wins!\n", winner == 0 ? "Red" : "Black");
        }
        printf("\n===== CHECKERS MENU =====\n1. New Game\n2. Load Game\n3. Save Game\n4. Display Board\n5. Make Move\n6. Quit\nChoose: ");
        if (scanf("%d", &choice) != 1) {
            int c; while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input.\n"); continue;
        }
        switch (choice) {
            case 1:
                game->red_pieces = generate_start_positions(1);
                game->black_pieces = generate_start_positions(0);
                game->red_kings = 0ULL; game->black_kings = 0ULL;
                game->current_turn = 0;
                printf("New game started.\n");
                break;
            case 2:
                printf("Enter filename to load: "); scanf("%127s", filename);
                loadGame(game, filename);
                break;
            case 3:
                printf("Enter filename to save: "); scanf("%127s", filename);
                saveGame(game, filename);
                break;
            case 4:
                PrintBoard(game->red_pieces, game->black_pieces, game->red_kings, game->black_kings);
                printf("Current turn: %s\n", game->current_turn == 0 ? "Red" : "Black");
                break;
            case 5:
                PrintBoard(game->red_pieces, game->black_pieces, game->red_kings, game->black_kings);
                printf("Current turn: %s\n", game->current_turn == 0 ? "Red" : "Black");
                prompt_move_and_execute(game);
                break;
            case 6: printf("Exiting.\n"); break;
            default: printf("Invalid choice.\n");
        }
    }
}

int main() {
    Game game;
    game.red_pieces = generate_start_positions(1);
    game.black_pieces = generate_start_positions(0);
    game.red_kings = 0ULL;
    game.black_kings = 0ULL;
    game.current_turn = 0;
    menu(&game);
    return 0;
}
