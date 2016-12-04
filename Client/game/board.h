#define BOARD_SIZE 6
#define BATTLESHIPS_NUMBER 7

typedef enum board_cell_status {
	ALIVE,HIT,MISSED,EMPTY,E_INVAL
} board_cell_status;


void boards_initialize();
boolean place_battelship(char row,int column);
board_cell_status try_hit(char row,int column,int* rem);

void print_board(board_cell_status board[BOARD_SIZE][BOARD_SIZE]);
void mark_board(char named_row,int column,board_cell_status sh);