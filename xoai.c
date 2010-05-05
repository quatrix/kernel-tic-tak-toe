#include "xoai.h"
#include <linux/random.h>
#include <linux/kernel.h>
#include <linux/string.h>





static char board[3][3];
static int _last_move = -1;
static int corners[4] = { 1,3,7,9 };
static int edges[4] = { 2,4,6,8  };
static int first_move[5] = {1,3,5,7,9};
static int opposite_move[10] = { -1,9,-1,7,-1,-1,-1,3,-1,1 };

#define OTHER_PLAYER(player) (player == 'x' ? 'o' : 'x')

int make_move(int player, unsigned square);
static int _random(int N);

void init_board(void) 
{
	_last_move = -1;
	for (int i = 0; i<9; i++)
		for (int j = 0; j<3; j++)
			board[i][j] = ' ';

}

void print_board(char *str)
{
	char t[100];
	char buf[4];
	memset(t,0,sizeof(t));

	for (int i = 0; i<3; i++) {
		for (int j = 0; j<3; j++) {
			sprintf(buf,"%c %c",board[i][j],(j<2?'|':'\n'));
			strcat(t,buf);
		}
		if (i < 2)
			strcat(t,"--------\n");
	}

	sprintf(str,"%s",t);
}


int ai_two_in_a_row(char player)
{
	int in_a_row = 0;
	int not_player = -1;
	char other_player = OTHER_PLAYER(player);

	// in a row
	for (int i = 0; i<3; i++) {
		in_a_row = 0;
		not_player = -1;

		for (int j = 0; j<3; j++)
			if (board[i][j] == player)
				in_a_row++;
			else if (board[i][j] != other_player)  
				not_player = i*3+j;

		if (in_a_row == 2 && not_player != -1)
			return not_player + 1;
		else if (in_a_row == 3)
			return -1;
	}

	// in a column
	for (int i = 0; i<3; i++) {
		in_a_row = 0;
		not_player = -1;

		for (int j = 0; j<3; j++)
			if (board[j][i] == player)
				in_a_row++;
			else if (board[j][i] != other_player)  
				not_player = j*3+i;

		if (in_a_row == 2 && not_player != -1)  
			return not_player + 1;
		else if (in_a_row == 3)
			return -1;
	}

	in_a_row = 0;
	not_player = -1;

	// from left corner 
	for (int i = 0; i<3; i++) {
		if (board[i][i] == player)
			in_a_row++;
		else if (board[i][i] != other_player) 
			not_player = i*3+i;
	}

	if (in_a_row == 2 && not_player != -1) 
		return not_player + 1;
	else if (in_a_row == 3)
		return -1;

	in_a_row = 0;
	not_player = -1;

	// from right corner
	for (int i = 0, j=2; i<3; i++,j--) {
		if (board[i][j] == player)
			in_a_row++;
		else if (board[i][j] != other_player)  
			not_player = i*3+j;
	}

	if (in_a_row == 2 && not_player != -1) 
		return not_player + 1;
	else if (in_a_row == 3)
		return -1;

	return 0;
}


static int ai_win(char player)
{
	int s;
	if ((s = ai_two_in_a_row(player)) > 0)  {
		make_move(player,s);
		return 1;
	}
	else if (s == -1)
		return 1;
	
	return 0;

}


static int ai_block(char player)
{
	int s;

	if ((s = ai_two_in_a_row(OTHER_PLAYER(player))))  {
		make_move(player,s);
		return 1;
	}
	
	return 0;

}

int make_move(int player, unsigned square)
{
	int x,y;

	if (square > 9 || square < 1) 
		return 0;

	_last_move = square;
	square -= 1;
	x = square / 3;
	y = square % 3;

	if (board[x][y] == ' ')
		board[x][y] = player;
	else 
		return 0;

	return 1;
}

static char get_square(unsigned square)
{
	int x,y;

	if (square > 9 || square < 1) 
		return 0;

	square -= 1;
	x = square / 3;
	y = square % 3;
	return board[x][y];
}


static int rand_move(int moves[]) 
{ 
	for (int i = 0; i < 99; i++) { 
		int __move = moves[_random(sizeof(moves))];
		if (get_square(__move) == ' ') 
			return __move;
	}
	return -1;
}	


static int ai_center_corners_edges(char player)
{
	int _move;
	if ((_move =  rand_move(first_move)) != -1) {
		make_move(player, _move);
		return 0;
	}
	else if ((_move = rand_move(edges)) != -1 ) {
		make_move(player, _move);
		return 0;
	}

	return -1;
}

static int find_opposite_move(char player)
{
	for (int i = 0; i < 4; i++)
		if (corners[i] == player && opposite_move[corners[i]] == ' ')
			return opposite_move[corners[i]];

	return -1;
}

static int ai_block_fork(char player)
{
	int om;
	char other_player = OTHER_PLAYER(player);

	// edge forks
	if ((get_square(2) == other_player && get_square(4) == other_player) || 
		((get_square(8) == other_player && get_square(6) == other_player))) {
		if (get_square(5) == ' ') {
			make_move(player,5);
			return 0;
		}
	}

	// corner forks
	if ((get_square(1) == other_player && get_square(9) == other_player) ||
		(get_square(3) == other_player && get_square(7) == other_player)) {
		if ((om = rand_move(edges)) != -1) {
			make_move(player,om);
			return 0;
		}
	}

	return -1;
}


static int ai_fork(char player)
{
	int other_player_takes_corner;

	//printf("in ai_fork move (last move: %d)\n",_last_move);

	// if opponet took center, take an opposite corner of the corner already taken
	if (_last_move == 5) {  
		int om;
		if ((om = find_opposite_move(player)) != -1) {
			make_move(player, om);
			return 0;
		} // if no corners taken yet, take a corner
		else if ((om = rand_move(corners)) != -1) {
			make_move(player,om);
			return 0;
		}
	}

	// if opponet took corner, take center
	for (int i = 0; i < 4; i++)
		if (_last_move == corners[i] && get_square(5) == ' ') {
			//printf("taking center\n");
			make_move(player,5);
			return 0;
		}
		


	other_player_takes_corner = 0;
	for (int i = 0; i < 4; i++)
		if (_last_move == corners[i]) 
			other_player_takes_corner = i;

	if (other_player_takes_corner) {
		int corner;
		if (get_square(opposite_move[other_player_takes_corner]) == ' ') {
			make_move(player,opposite_move[other_player_takes_corner]);
			return 0;
		}
		else if ((corner = rand_move(corners)) != -1) { 
			make_move(player,corner);
			return 0;
		}
	}

	return -1;
}

static int ai_win_or_block(char player)
{
	if (ai_win(player)) 
		return 1;
	
	if (ai_block(player))
		return 0;

	return -1;
}

static int _random(int N) 
{
	int r;
	get_random_bytes(&r,sizeof(int));

	return r % (N+1);
}

static int readuser(void) 
{
	/*
	int s;
	scanf("%d",&s);
	return s;
	*/
	return 0;
}

int make_ai_move(char player) 
{ 
	int rc;

	if ((rc = ai_win_or_block(player)) != -1)
		return rc;
	else if ((rc = ai_block_fork(player)) != -1)
		return rc;
	else if ((rc = ai_fork(player)) != -1)
		return rc;
	else if ((rc =ai_center_corners_edges(player)) != -1)
		return rc;
	else 
		return 0;
}


#if 0
int main(void) { 
	srand(time(NULL));
	init_board();
	int _move;

	/*
	make_move('x', 3);
	make_ai_move('o');
	make_move('x', 7);
	make_ai_move('o');

	print_board();
	return 0;
	*/

	print_board();
	for (int i = 0; i < 5; i++) {
		if (i < 5) {
			/*
			do {
				printf("enter move: ");
				_move = readuser();
			} while(!make_move('x',_move));
			*/
			make_ai_move('x');
			print_board();
		}
		if (ai_two_in_a_row('x') == -1) {
			printf("x wins\n");
			return 0;
		}

		int x_wins = make_ai_move('o');
		print_board();

		if (x_wins) {
			printf("o wins\n");
			return 0;
		}	
	}

	printf("draw\n");



	return 0;
}
#endif
