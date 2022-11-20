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
static int first_round_valid_play(Player *player, Card *card);
static int lead_valid_play(Player *player, Card *card);
static int follow_suit_valid_play(Player *player, Card *card);
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
    int end_score;
    Hearts_Error error;
} game;

int main(void) {
    start_hearts();
    return 1;
}

int start_hearts(void) {
    Deck *deck;
    Player *players;
    int i, win = 0;
    
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

    game.end_score = 100; /*note this can be user-defined*/
    game.first_trick = 0;
    game.hearts_broken = 0;
    game.is_lead = 0;
    game.error = ERR_NONE;

    do {
        play_round(players, deck);
        printf("Scores:\n");
        for (i = 0; i < NUM_PLAYERS; i++)
            printf("Player %d: %d\n", i, players[i].score);
    } while (!stop_game(players, game.end_score));

    for (i = 1; i < NUM_PLAYERS; i++)
        if (players[i].score < players[win].score)
            win = i;

    printf("\nPlayer %d won the game!!!!\n", win);
    
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
    
    player_start_num = find_player_with_card(players, game.two_clubs, 4)->num;
    while (rounds_left--) {
        game.led_suit = NONE_SUIT;
        game.is_lead = 1;
        game.round_num++;
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
            
        /*print_hand(&players[table_index]);*/
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

int stop_game(Player *players, const int end) {
    int i;
    int stop = 0;
    
    for (i = 0; !stop && i < NUM_PLAYERS; i++)
        if (players[i].score >= end)
            stop = 1;

    return stop;
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
    int valid = 0;

    if (game.round_num == 1)
        valid = first_round_valid_play(player, card);
    else if (game.is_lead)
        valid = lead_valid_play(player, card);
    else
        valid = follow_suit_valid_play(player, card);

    if (!valid)
        print_err();

    return valid;
}

static int first_round_valid_play(Player *player, Card *card) {
    int valid = 0;
    printf("it is the first round\n");

    if (game.is_lead) {
        if (card != game.two_clubs) {
            printf("player tried to play something other than 2C on lead\n");
            game.error = ERR_2C_FIRST_TRICK;
        } else
            valid = 1;
    } else if (card->suit == HEARTS) {
        printf("player tried to play hearts on first trick\n");
        game.error = ERR_HEARTS_FIRST_TRICK;
    } else if (card == game.queen_spades) {
        printf("player tried to play QS on first trick\n");
        game.error = ERR_QUEEN_FIRST_TRICK;
    } else
        valid = follow_suit_valid_play(player, card);

    return valid;
}

static int lead_valid_play(Player *player, Card *card) {
    int valid = 0;
    printf("it is the lead\n");
    
    if (!game.hearts_broken && card->suit == HEARTS
        && player->num_cards != player->num_hearts) {
        printf("player tried to play a heart when they were not broken\n");
        game.error = ERR_HEARTS_BRKN;
    }
    else
        valid = 1;

    return valid;
}

static int follow_suit_valid_play(Player *player, Card *card) {
    int valid = 0;
    printf("checking out a regular play\n");

    /* if player has zero cards in led suit then they can play
       whatever they want */
    if (card->suit != game.led_suit && num_of_suit(player, game.led_suit)) {
        printf("player has cards they could have played\n");
        game.error = ERR_FOLLOW_SUIT;
    } else
        valid = 1;

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

