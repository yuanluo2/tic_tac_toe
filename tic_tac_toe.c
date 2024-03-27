#include <stdio.h>
#include <string.h>

#define BOARD_WIDTH 3
#define BOARD_HISTORY_LEN 9

#define P_PLAYER  0
#define P_AI      1
#define P_EMPTY   2

#define INPUT_BUF_LEN 10

typedef int Piece;
typedef int Boolean;

typedef struct Pos {
	int row, col;
} Pos;

typedef struct Board {
	Piece data[BOARD_WIDTH][BOARD_WIDTH];
	Pos history[BOARD_HISTORY_LEN];
	int historyLen;
} Board;

void board_init(Board* bd){
	int r, c;
	for (r = 0; r < BOARD_WIDTH; ++r){
		for (c = 0; c < BOARD_WIDTH; ++c){
			bd->data[r][c] = P_EMPTY;
		}
	}

	bd->historyLen = 0;
}

/* board is full of pieces ? */
Boolean board_is_full(Board* bd){
	return bd->historyLen == BOARD_WIDTH * BOARD_WIDTH;
}

/* set p at given position. */
void board_push(Board* bd, int r, int c, Piece p){
	bd->data[r][c] = p;
	bd->history[bd->historyLen].row = r;
	bd->history[bd->historyLen].col = c;
	bd->historyLen += 1;
}

/* undo last move. */
void board_pop(Board* bd){
	int r, c;

	bd->historyLen -= 1;

	r = bd->history[bd->historyLen].row;
	c = bd->history[bd->historyLen].col;
	bd->data[r][c] = P_EMPTY;
}

void board_print(const Board* bd){
	static const char mapping[] = {
		'X', 'O', ' '
	};

	int n = 3;
	Piece p0, p1, p2;

	int r;
	for (r = 0; r < BOARD_WIDTH; ++r){
		p0 = bd->data[r][0];
		p1 = bd->data[r][1];
		p2 = bd->data[r][2];

		printf("    +---+---+---+\n");
		printf(" %d  | %c | %c | %c |\n", n, mapping[p0], mapping[p1], mapping[p2]);

		n -= 1;
	}

	printf("    +---+---+---+\n");
	printf("      a   b   c\n");
}

/* evaluate the board's score. */
int board_evaluate(const Board* bd){
	static const char mapping[] = {
		10, -10, 0
	};

	Piece p;
	int r, c;

	/* any row is full. */
	for (r = 0; r < BOARD_WIDTH; ++r){
		if (bd->data[r][0] == bd->data[r][1] && bd->data[r][1] == bd->data[r][2]){
			p = bd->data[r][0];

			if (p != P_EMPTY){
				return mapping[p];
			}
		}
	}

	/* any column is full. */
	for (c = 0; c < BOARD_WIDTH; ++c){
		if (bd->data[0][c] == bd->data[1][c] && bd->data[1][c] == bd->data[2][c]){
			p = bd->data[0][c];

			if (p != P_EMPTY){
				return mapping[p];
			}
		}
	}

	/* top left to bottom right. */
	if (bd->data[0][0] == bd->data[1][1] && bd->data[1][1] == bd->data[2][2]){
		p = bd->data[0][0];

		if (p != P_EMPTY){
			return mapping[p];
		}
	}

	/* top right to bottom left. */
	if (bd->data[0][2] == bd->data[1][1] && bd->data[1][1] == bd->data[2][0]){
		p = bd->data[0][2];

		if (p != P_EMPTY){
			return mapping[p];
		}
	}

	return 0;
}

/*
 * min, max macros.
 *
 * attention: 
 *   before calling these macros you must calculate the a and b first, 
 *   otherwise a or b would be calculated twice by these macros, because they
 *   are not functions !
*/
#define VALUE_MAX(a, b) ((a) > (b) ? (a) : (b))
#define VALUE_MIN(a, b) ((a) < (b) ? (a) : (b))

/* min-max algorithm for tic-tac-toe. */
int min_max(Board* bd, int depth, int isMax){
	int value = board_evaluate(bd);
	int r, c, bestValue, tempValue;
	Piece p;

	if (depth == 0 || value != 0){   /* depth = 0 or some one wins. */
		return value;
	}

	if (board_is_full(bd)){   /* draw. */
		return 0;
	}

	if (isMax){   /* player side, the bigger the value, the better. */
		bestValue = -10000;

		for (r = 0; r < BOARD_WIDTH; ++r){
			for (c = 0; c < BOARD_WIDTH; ++c){
				p = bd->data[r][c];

				if (p == P_EMPTY){
					board_push(bd, r, c, P_PLAYER);

					tempValue = min_max(bd, depth - 1, !isMax);
					bestValue = VALUE_MAX(bestValue, tempValue);

					board_pop(bd);
				}
			}
		}

		return bestValue;
	}
	else {   /* ai side, the smaller the value, the better. */
		bestValue = 10000;

		for (r = 0; r < BOARD_WIDTH; ++r){
			for (c = 0; c < BOARD_WIDTH; ++c){
				p = bd->data[r][c];

				if (p == P_EMPTY){
					board_push(bd, r, c, P_AI);

					tempValue = min_max(bd, depth - 1, !isMax);
					bestValue = VALUE_MIN(bestValue, tempValue);

					board_pop(bd);
				}
			}
		}

		return bestValue;
	}
}

Pos gen_best_pos_for_ai(Board* bd, int depth){
	int bestValue = 10000;
	int r, c, tempValue;
	Piece p;
	Pos bestPos;

	for (r = 0; r < BOARD_WIDTH; ++r){
		for (c = 0; c < BOARD_WIDTH; ++c){
			p = bd->data[r][c];

			if (p == P_EMPTY){
				board_push(bd, r, c, P_AI);
				tempValue = min_max(bd, depth, 1);
				board_pop(bd);

				if (tempValue < bestValue){   /* ai side, the smaller the value, the better. */
					bestPos.row = r;
					bestPos.col = c;
					bestValue = tempValue;
				}
			}
		}
	}

	return bestPos;
}

/* 
 * fetch the user input line.
 * return the input string's length, exclude '\n'.
*/
int get_input(char* input, int limit){
	int count = 0;
	char c;

	while (1){
		c = getchar();

		if (c == '\n' || c == EOF){
			break;
		}

		if (count < limit - 1){
			input[count] = c;
			++count;
		}
	}

	input[count] = '\0';
	return count;
}

/* given input can be converted to a pos struct ? */
Boolean is_input_a_pos(const char* input){
	char row, col;

	if (strlen(input) < 2){
		return 0;
	}

	row = input[1];
	col = input[0];
	return col >= 'a' && col <= 'c' && row >= '1' && row <= '3';
}

Pos convert_input_to_pos(const char* input){
	Pos pos;
	pos.row = '3' - input[1];
	pos.col = input[0] - 'a';

	return pos;
}

/* when some wins or draw, return true. */
Boolean game_over(Board* bd){
	int value = board_evaluate(bd);

	if (value > 0){
		printf("You win!\n");
		return 1;
	}
	else if (value < 0){
		printf("You lose!\n");
		return 1;
	}
	else {
		if (board_is_full(bd)){
			printf("draw.\n");
			return 1;
		}
		else {
			return 0;
		}
	}
}

int main(){
	Board bd;
	char input[INPUT_BUF_LEN];
	Pos pos, aiPos;

	board_init(&bd);
	board_print(&bd);

	while (1){
		get_input(input, INPUT_BUF_LEN);

		if (is_input_a_pos(input)){
			pos = convert_input_to_pos(input);

			board_push(&bd, pos.row, pos.col, P_PLAYER);
			board_print(&bd);
			
			if (game_over(&bd)){
				break;
			}

			aiPos = gen_best_pos_for_ai(&bd, 5);
			board_push(&bd, aiPos.row, aiPos.col, P_AI);
			board_print(&bd);
			
			if (game_over(&bd)){
				break;
			}
		}

	}

	return 0;
}
