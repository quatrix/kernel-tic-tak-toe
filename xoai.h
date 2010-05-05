#ifndef XOAI_H
#define XOAI_H

#define OTHER_PLAYER(player) (player == PLAYER_X ? PLAYER_O : PLAYER_X )

void init_board(void);
void print_board(char *);
int make_move(int, unsigned);
int make_ai_move(char);
int ai_two_in_a_row(char);

#endif
