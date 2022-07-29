/* hearts.h
   Alexander Yelovich, June 12, 2022
*/

#ifndef _HEARTS_H
#define _HEARTS_H

#include "cards.h"

#define NUM_CARDS_IN_DECK 52
#define NUM_START_IN_HAND 13
#define NUM_PLAYERS 4

#define SMALLEST_VAL 2
#define LARGEST_VAL ACE_HIGH

typedef enum {
    ERR_NONE = 0,
    ERR_FOLLOW_SUIT = 10,
    ERR_HEARTS_FIRST_TRICK,
    ERR_QUEEN_FIRST_TRICK,
    ERR_2C_FIRST_TRICK,
    ERR_HEARTS_BRKN
} Hearts_Error;

int start_hearts(void);
int play_trick(Player *players, int startno);
int compare_cards(Card *first, Card *second, Suit lead);

#endif /* _HEARTS_H */
