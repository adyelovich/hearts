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

/* function used to determine if a play is valid or not,
   which is dependent on the rules of the game */
typedef int (*valid_play_function)(Player *player, Card *card);

/*int gensd(Card *deck, Value start, Value end, int n);*/

/**
   Generate a "standard" deck of cards, where standard means n cards with values
   from start to end, inclusive.
   deck must already have space allocated for it.
   Returns the number of cards created.
 */
int gensd(Deck *deck, Value start, Value end, int n);

/**
   Generates an array of players which simulates the players at the table.
   It will create n players that can hold handsize cards each.
   Player must be allocated prior, a field is allocated by gentable for
   each player at the table.
 */
int gentable(Player *table, int n, int handsize);

/**
   Deal numtodeal cards to each player from deck. If the deck runs out of
   cards before the needed number of cards is dealt, then the deal ends there,
   just like in real life.
 */
/*int dealdeck(Player *table, Card *deck, int numtodeal, int numplayers, int decksize);*/
int dealdeck(Player *table, Deck *deck, int numtodeal, int numplayers, int decksize);

/**
   Print the contents of the player's hand in a non-debug format.
 */
int print_hand(Player *player);

/**
   Simulates playing a card from a player's hand. The player
   is prompted with entering a number, and then the corresponding
   card in their hand is removed from their hand and the
   card's state is changed to IN_PLAY. The memory address of
   the removed card is returned.
   The basic conditions for deeming a card valid is if the
   selected number is within the number of cards in their hand.
   The caller may also specify additional conditions on a
   card by passing a function that checks further. If NULL is
   passed for the function, then no additional checks are made.
 */
Card *play_card(Player *player, valid_play_function is_valid);

/**
   Increments the number of cards in suit in player's hand.
 */
int inc_card_suit(Player *player, Suit suit);

/**
   Decrements the number of cards in suit in player's hand.
 */
int dec_card_suit(Player *player, Suit suit);

/**
   Print a card, either in debug or abbreviated format.
 */
void printcard(const Card *card, unsigned int option);

/**
   Print the number of cards in suit player has.
 */
int num_of_suit(Player *player, Suit suit);

/**
   Finds the player in players that has card in their hand.
   If the card is not found then NULL is returned.
 */
Player *find_player_with_card(Player *players, const Card *card, int numplayers);

#endif /* _CARDS_H */
