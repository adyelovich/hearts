/* hearts.c
   Alexander Yelovich, June 12, 2022
*/

#include <stdio.h>
#include <stdlib.h>

#include "cards.h"
#include "hearts.h"

#define ERR_MSG_FOLLOW_SUIT "Invalid: Must follow suit if able."
#define ERR_MSG_HEARTS_FIRST_TRICK "Invalid: Cannot play hearts " \
                                 "on the first trick."
#define ERR_MSG_QUEEN_FIRST_TRICK "Invalid: Cannot play QS on the " \
                                "first trick."
#define ERR_MSG_2C_FIRST_TRICK "Invalid: Leading player must play " \
                             "2C on the first trick."
#define ERR_MSG_HEARTS_BRKN "Invalid: Cannot lead hearts until they " \
                            "have been broken."

static int hearts_valid_play(Player *player, Card *card);
static void print_err(void);

static struct {
    short hearts_broken;
    short first_trick;
    short is_lead;
    Suit led_suit;
    Hearts_Error error;
} game;

int main(void) {
    start_hearts();
    return 1;
}

int start_hearts(void) {
    Card *deck;
    Player *players;

    deck = malloc(NUM_CARDS_IN_DECK * sizeof(Card));
    players = malloc(NUM_PLAYERS * sizeof(Player));

    if (deck == NULL || players == NULL) {
        printf("<start_hearts> error: failure allocating starting memory\n");
        return -1;
    }

    gensd(deck, SMALLEST_VAL, LARGEST_VAL, 1);
    gentable(players, NUM_PLAYERS, NUM_START_IN_HAND);

    dealdeck(players, deck, NUM_START_IN_HAND, NUM_PLAYERS, NUM_CARDS_IN_DECK);

    game.first_trick = 0;
    game.hearts_broken = 0;
    game.is_lead = 0;
    game.error = ERR_NONE;
    
    

    return 1;
}

int start_round(Player *players, Card *deck) {
    dealdeck(players, deck, NUM_START_IN_HAND, NUM_PLAYERS, NUM_CARDS_IN_DECK);

    game.first_trick = 0;
    game.hearts_broken = 0;
    game.is_lead = 1;
    game.error = ERR_NONE;

    play_trick(players, find_player_with_card
}

int play_trick(Player *players, int startno) {
    int i;
    Card *trick[NUM_PLAYERS];
    
    for (i = 0; i < NUM_PLAYERS; i++) {
        trick[i] = play_card(&players[(i + startno) % NUM_PLAYERS], hearts_valid_play);

        if (!i) {
            game.is_lead = 0;
            game.led_suit = trick[i]->suit;
        }

        if (!game.hearts_broken && trick[i]->suit == HEARTS)
            game.hearts_broken = 1;
    }

    return 1;
}

static int hearts_valid_play(Player *player, Card *card) {
    int valid;

    valid = 1;

    if (game.is_lead) {
        printf("it is the player's lead\n");
        if (game.first_trick) {
            printf("it is the first trick\n");
            if (card->value != TWO || card->suit != CLUBS) {
                printf("player tried to play a non-2C on first trick\n");
                game.error = ERR_2C_FIRST_TRICK;
                valid = 0;
            } else
                printf("successfully chose 2C\n");
        } else if (card->suit == HEARTS && !game.hearts_broken && player->num_hearts) {
            printf("tried to play hearts when they weren\'t broken\n");
            game.error = ERR_HEARTS_BRKN;
            valid = 0;
        } else
            printf("successfully played a legal card\n");
    } else if (card->suit != game.led_suit) {
        printf("it is not the player's lead\n");
        printf("player tried to play a suit that does not match led suit\n");
        printf("%d\n", num_of_suit(player, game.led_suit));
        if (num_of_suit(player, game.led_suit)) {
            printf("player has cards they could play\n");
            game.error = ERR_FOLLOW_SUIT;
            valid = 0;
        } else if (game.first_trick) {
            printf("player has no cards in led suit\n");
            printf("it is the first trick\n");
            if (card->suit == HEARTS) {
                printf("player tried to play a heart on first trick\n");
                game.error = ERR_HEARTS_FIRST_TRICK;
                valid = 0;
            } else if (card->value == QUEEN && card->suit == SPADES) {
                printf("player tried to play a QS on first trick");
                game.error = ERR_QUEEN_FIRST_TRICK;
                valid = 0;
            }
        }
        
    }

    if (!valid)
        print_err();
    
    return valid;
}

static void print_err(void) {
    switch (game.error) {
    case ERR_FOLLOW_SUIT:
        printf(ERR_MSG_FOLLOW_SUIT"\n");
        break;
    case ERR_HEARTS_FIRST_TRICK:
        printf(ERR_MSG_HEARTS_FIRST_TRICK"\n");
        break;
    case ERR_QUEEN_FIRST_TRICK:
        printf(ERR_MSG_QUEEN_FIRST_TRICK"\n");
        break;
    case ERR_2C_FIRST_TRICK:
        printf(ERR_MSG_2C_FIRST_TRICK"\n");
        break;
    case ERR_HEARTS_BRKN:;
        printf(ERR_MSG_HEARTS_BRKN"\n");
        break;
    default:
        printf("Encountered unknown error\n");
        break;
    }
}

