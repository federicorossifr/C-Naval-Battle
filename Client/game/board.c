#include "../../Utilities/utility.h"
#include "board.h"

int remaining_battleships;
board_cell_status allies[BOARD_SIZE][BOARD_SIZE];
board_cell_status enemies[BOARD_SIZE][BOARD_SIZE];

void boards_initialize() {
    int i,j;
    for(i = 0; i < BOARD_SIZE;++i)
        for(j = 0; j < BOARD_SIZE;++j) {
            allies[i][j] = enemies[i][j] = EMPTY;
        }
        remaining_battleships = BATTLESHIPS_NUMBER;
}

int get_int_row(char named_row) {
    named_row = tolower(named_row);
    int row = named_row-97;
    return row;
}

boolean is_valid_coord(int row,int column) {
    if(row < 0 || row > BOARD_SIZE-1) return false;
    if(column < 0 || column > BOARD_SIZE-1) return false;	
    return true;
}


void print_cell(board_cell_status cell) {
    switch(cell) {
        case ALIVE: printf("O"); break;
        case HIT: printf("X"); break;
        case EMPTY: printf("-"); break;
        case MISSED: printf("^"); break;
        default: break;
    }
}

boolean place_battelship(char named_row,int column) {
    column--;
    int row = get_int_row(named_row);
    if(!is_valid_coord(row,column)) return false;
    if(allies[row][column] != EMPTY) return false;
    allies[row][column] = ALIVE;
    return true;
}

board_cell_status try_hit(char named_row,int column,int* rem) {
    column--;
    *rem = remaining_battleships;
    int row = get_int_row(named_row);
    if(!is_valid_coord(row,column)) return E_INVAL;
    board_cell_status* cell = &allies[row][column];
    switch(*cell) {
        case EMPTY:
        case MISSED:
            *cell = MISSED;
        case HIT:
            return MISSED; 
            break;
        case ALIVE: *cell = HIT; 
        *rem = --remaining_battleships; 
        return *cell; 
        break;
        default: return E_INVAL;
    }
}

void print_board(board_cell_status board[BOARD_SIZE][BOARD_SIZE]) {
    int i,j,k;
    char l = 'A';
    printf("  ");
    for(k = 1; k <= BOARD_SIZE;++k)
        printf("%d ",k);
    printf("\n");
    for(i = 0; i < BOARD_SIZE;++i) {
        printf("%c ",l+i);
        for(j = 0; j < BOARD_SIZE;++j) {
            print_cell(board[i][j]);
            printf(" ");
        }
        printf("\n");
    }
}


void mark_board(char named_row,int column,board_cell_status sh) {
    column--;
    int row = get_int_row(named_row);
    if(!is_valid_coord(row,column)) return;
    enemies[row][column] = sh;
}