#ifndef _CARDS_H
#define _CARDS_H

#define PRINT_ABBREV 0u
#define PRINT_DEBUG 1u

typedef enum _Value {
    ACE_LOW = 1,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK,
    QUEEN,
    KING,
    ACE_HIGH
} Value;

typedef enum _Suit {
    SPADES,
    HEARTS,
    CLUBS,
    DIAMONDS
} Suit;

typedef enum _Card_State {
    IN_HAND,
    IN_DECK,
    IN_PLAY,
    IN_DISCARD
} Card_State;

typedef struct _Card {
    Value value;
    Suit suit;
    Card_State state;
} Card;

typedef struct _Player {
    int num;
    int num_cards;
    Card **hand; /* array of pointers to cards */

    int score;
    int num_spades;
    int num_hearts;
    int num_clubs;
    int num_diamonds;
} Player;

typedef struct _Deck {
    Card *cards; /* array of cards */
    Value smallest_val;
    Value biggest_val;
    int num_cards;
} Deck;

typedef int (*valid_play_function)(Player *player, Card *card);

/*int gensd(Card *deck, Value start, Value end, int n);*/
int gensd(Deck *deck, Value start, Value end, int handsize);
int gentable(Player *table, int n, int handsize);
/*int dealdeck(Player *table, Card *deck, int numtodeal, int numplayers, int decksize);*/
int dealdeck(Player *table, Deck *deck, int numtodeal, int numplayers, int decksize);
int print_hand(Player *player);
Card *play_card(Player *player, valid_play_function is_valid);
int inc_card_suit(Player *player, Suit suit);
int dec_card_suit(Player *player, Suit suit);
void printcard(Card *card, unsigned int option);
int num_of_suit(Player *player, Suit suit);
Player *find_player_with_card(Player *players, Card *card, int numplayers);
#endif
