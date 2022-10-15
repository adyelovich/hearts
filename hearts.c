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
static int compare_values(Card *first, Card *second);

static struct {
    short hearts_broken;
    short first_trick;
    short is_lead;
    short round_num;
    Suit led_suit;
    const Card *two_clubs;
    const Card *queen_spades;
    Hearts_Error error;
} game;

int main(void) {
    start_hearts();
    return 1;
}

int start_hearts(void) {
    Deck *deck;
    Player *players;
    
    deck = malloc(NUM_CARDS_IN_DECK * sizeof(Card));
    players = malloc(NUM_PLAYERS * sizeof(Player));

    if (deck == NULL || players == NULL) {
        printf("<start_hearts> error: failure allocating starting memory\n");
        return -1;
    }

    gensd(deck, SMALLEST_VAL, LARGEST_VAL, 1);

    /* -2 because one for the general offset back to 0-index, and
       another due to the fact that we start with TWO instead of ACE_LOW */
    game.two_clubs = &deck->cards[13 * CLUBS + TWO - 2];
    game.queen_spades = &deck->cards[13 * SPADES + QUEEN - 2];

    gentable(players, NUM_PLAYERS, NUM_START_IN_HAND);

    game.first_trick = 0;
    game.hearts_broken = 0;
    game.is_lead = 0;
    game.error = ERR_NONE;

    play_round(players, deck);
        
    return 1;
}

/* think of this like playing one deal */
int play_round(Player *players, Deck *deck) {
    int rounds_left = NUM_START_IN_HAND;
    int player_start_num;
    Player *player_start;

    /*set the game state*/
    /*at the start of each round we have the following:
      - it is the first trick
      - the first play is that leading play for that trick
      - hearts are not broken
      - (for now) there is no error
    */
    game.first_trick = 1;
    game.hearts_broken = 0;
    
    /*Deal deck of cards*/
    dealdeck(players, deck, NUM_START_IN_HAND, NUM_PLAYERS, NUM_CARDS_IN_DECK);
    
    /*Play a trick*/
    game.round_num++;
    
    player_start_num = find_player_with_card(players, game.two_clubs, 4)->num;
    #if 0
    printf("Can I lead? %s\n", game.is_lead ? "yes" : "no");
    printf("%d\n", play_trick(players, player_start_num));
    #endif
    while (rounds_left--) {
        game.led_suit = NONE_SUIT;
        game.is_lead = 1;
        player_start_num = play_trick(players, player_start_num);
    }
    return 0;
}

int play_trick(Player *players, int startno) {
    int i, winning_index = startno;
    Card *winning_card, *trick[NUM_PLAYERS];

    puts("starting up play trick");
    printf("it is player %d's lead!\n", winning_index);
    for (i = 0; i < NUM_PLAYERS; i++) {
        int table_index = (i + startno) % NUM_PLAYERS;
            
        print_hand(&players[table_index]);
        trick[table_index] = play_card(&players[table_index], hearts_valid_play);
        
        if (!i) {
            game.is_lead = 0;
            game.led_suit = trick[table_index]->suit;
        }

        if (!game.hearts_broken && trick[table_index]->suit == HEARTS)
            game.hearts_broken = 1;

        winning_index = compare_cards(trick[table_index], trick[winning_index]) > 0
            ? table_index : winning_index;
    }

    /* collect the trick */
    for (i = 0; i < NUM_PLAYERS; i++) {
        if (trick[i]->suit == HEARTS)
            players[winning_index].score++;
        else if (trick[i]->suit == SPADES && trick[i]->value == QUEEN)
            players[winning_index].score += 13;

        trick[i]->state = IN_DISCARD;
    }

    if (game.first_trick)
        game.first_trick = 0;

    printf("player %d won the last trick!\n", winning_index);
    return winning_index;
}

int compare_cards(Card *first, Card *second) {
    int ret;

    if (first->suit == second->suit)
        ret = compare_values(first, second);
    else
        ret = first->suit == game.led_suit ? 1 : -1;

    return ret;
}

static int compare_values(Card *first, Card *second) {
    if (first->value > second->value)
        return 1;
    else if (first->value == second->value)
        return 0;
    else
        return -1;
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

