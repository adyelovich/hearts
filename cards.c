#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cards.h"

#define IN_RANGE(check, max) ((check) >= 1 && (check) <= (max))

static int insert_sorted(Player *player, Card *to_insert);
static Card *remove_from_hand(Player *player, int cardno);
static unsigned int deal_card(Player *player, Deck *deck, int decksize);

/*
static unsigned int deal_card(Player *player, Card *deck, int decksize);
*/
/**
 * Generates a "standard" deck of cards, where is standard is considered to
 * be a type of deck where one of each suit of a card exists. This function
 * creates a deck containing n of each suit of cards with value start to end
 * (inclusive), and then returns the number of cards created.
 */

int gensd(Deck *deck, Value start, Value end, int n) {
    int k;
    Value i;
    Suit j;
    Card *p;

    deck->smallest_val = start;
    deck->biggest_val = end;
    deck->num_cards = (end - start + 1) * 4 * n;
    deck->cards = malloc(deck->num_cards * sizeof(*deck->cards));

    if (deck->cards == NULL)
        return -1;

    p = deck->cards;
    for (k = 0; k < n; k++)
        for (i = start; i <= end; i++)
            for (j = SPADES; j <= DIAMONDS; j++) {
                p->value = i;
                p->suit = j;
                p->state = IN_DECK;
                p++;
            }

    return k;
}

#if 0
int gensd(Card *deck, Value start, Value end, int n) {
    int k;
    Value i;
    Suit j;
    Card *p;

    p = deck;
    for (k = 0; k < n; k++)
        for (i = start; i <= end; i++)
            for (j = SPADES; j <= DIAMONDS; j++) {
                p->value = i;
                p->suit = j;
                p->state = IN_DECK;
                p++;
            }

    return k;
}
#endif

/**
 * Generates an array of players which simulates the table of players. It will
 * create n players which can hold handsize cards.
 */
int gentable(Player *table, int n, int handsize) {
    int i, ret;

    ret = 1;
    for (i = 0; ret != -1 && i < n; i++) {
        (table + i)->num = i;
        (table + i)->num_cards = 0;
        (table + i)->num_spades = 0;
        (table + i)->num_hearts = 0;
        (table + i)->num_clubs = 0;
        (table + i)->num_diamonds = 0;
        (table + i)->score = 0;
        (table + i)->hand = malloc(handsize * sizeof(Card *));
        if ((table+i)->hand == NULL)
            ret = -1;
    }

    return ret;
}

/**
 * Deals n cards to each player in players from deck. Just like in real life,
 * the deck may run out of cards despite the fact that not everyone has the
 * same amount of cards.
 */

int dealdeck(Player *players, Deck *deck, int n, int numplayers, int decksize) {
    int i;

    srand(time(NULL));
    n *= 4;

    for (i = 0; n-- > 0 && i < decksize; i++)
        deal_card(&players[i % numplayers], deck, decksize);

    return i;
}

#if 0
int dealdeck(Player *players, Card *deck, int n, int numplayers, int decksize) {
    int i;
    unsigned int ri;

    srand(time(NULL));
    ri = rand() % decksize;
    n *= 4;
  
    for (i = 0; n-- > 0 && i < decksize; i++) {
        /* Find a card at random that has not been dealt yet */
        printf("Num before check: %u\n", ri);
        while ((deck + ri)->state != IN_DECK) {
            printf("Random number: %u\n", ri);
            ri = rand() % decksize; /* Get a new random number */
        }

        /* The next player gets that card in the next empty spot in their hand */
        printf("Dealing card at %u\n", ri);
        insert_sorted(players, deck + ri, i % numplayers);
        deck[ri].state = IN_HAND; /* Card is now in hand */
    }
    
    return i;
}
#endif


    

/**
   This function prints a player's hand in the format
   Hand: 4S 10S JD , etc
   and then returns the number of cards printed from their hand
 */
int print_hand(Player *player) {
    int i;

    printf("Hand:");
    for (i = 0; i < player->num_cards; i++) {
        printf(" ");
        printcard(player->hand[i], PRINT_ABBREV);
    }
    printf("\n");

    return i + 1;
}

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
Card *play_card(Player *player, valid_play_function is_valid) {
    int num, valid = 0;
    Card *played;

    while (!valid) {
        printf("Choose a card to play: ");
        scanf("%d", &num);

        if ((valid = IN_RANGE(num, player->num_cards))) {
            if (is_valid != NULL)
                valid = is_valid(player, player->hand[num - 1]);
        } else {
            printf("Invalid entry: %d. Must be in range (1 - %d)\n",
                   num, player->num_cards);
        }
    }

    player->hand[num - 1]->state = IN_PLAY;
    dec_card_suit(player, player->hand[num]->suit);
    played = remove_from_hand(player, num - 1);

    return played;
}

#if 0
/**
   THIS IS THE OLD FUNCTION
   Simulates playing a card from a player's hand. The player whose
   turn it is is prompted with choosing a number, and then the
   corresponding card in their hand is removed from their hand, the
   card enters the IN_PLAY state, and the memory address of that
   selected card is returned so that the caller knows what card was
   selected.
 */
Card *play_card(Player *player) {
    int num;
    Card *played;
    
    do {
        printf("Choose a card to play: ");
        scanf("%d", &num);
        
        if (num < 1 || num > player->num_cards)
            printf("Invalid entry: choose a number 1 - %d\n",
                   player->num_cards);
    } while (num < 1 || num > player->num_cards);

    player->hand[num - 1]->state = IN_PLAY;
    played = remove_from_hand(player, num - 1);

    return played;
}

#endif

int inc_card_suit(Player *player, Suit suit) {
    int ret;
    
    switch (suit) {
    case SPADES:
        ret = ++player->num_spades;
        break;
    case HEARTS:
        ret = ++player->num_hearts;
        break;
    case CLUBS:
        ret = ++player->num_clubs;
        break;
    case DIAMONDS:
        ret = ++player->num_diamonds;
        break;
    default:
        ret = -1;
        break;
    }

    return ret;
}

int dec_card_suit(Player *player, Suit suit) {
    int ret;
    
    switch (suit) {
    case SPADES:
        ret = --player->num_spades;
        break;
    case HEARTS:
        ret = --player->num_hearts;
        break;
    case CLUBS:
        ret = --player->num_clubs;
        break;
    case DIAMONDS:
        ret = --player->num_diamonds;
        break;
    default:
        ret = -1;
        break;
    }

    return ret;
}

Player *find_player_with_card(Player *players, Card *card, int numplayers) {
    int i, j, found;

    for (i = found = 0; i < numplayers && !found; i++) {
        for (j = 0; j < players[i].num_cards
                    && players[i].hand[j]->suit < card->suit; j++)
            ;

        for ( ; j < players[i].num_cards
                && players[i].hand[j]->value < card->value; j++)
            ;

        found = j != players[i].num_cards;
    }

    return i == numplayers ? NULL : &players[i];
}

int num_of_suit(Player *player, Suit suit) {
    int ret;

    switch (suit) {
    case SPADES:
        ret = player->num_spades;
        break;
    case HEARTS:
        ret = player->num_hearts;
        break;
    case CLUBS:
        ret = player->num_clubs;
        break;
    case DIAMONDS:
        ret = player->num_diamonds;
        break;
    default:
        ret = -1;
        break;
    }

    return ret;
}

#if 0
Card *get_card_addr(Card *cards, Value val, Suit suit, int n)
#endif

/*This is the new one*/
static int insert_sorted(Player *player, Card *to_insert) {
    int i;
    
    /* first find the suit offset */
    for (i = 0; i < player->num_cards
                && to_insert->suit > player->hand[i]->suit; i++)
        ;
    /* next find the value offset */
    for ( ; i < player->num_cards
            && to_insert->suit == player->hand[i]->suit
            && to_insert->value > player->hand[i]->value; i++)
        ;

    if (i != player->num_cards) {
        Card *next = player->hand[i];
        Card *temp = player->hand[i + 1];
        player->hand[i++] = to_insert;
        do {
            temp = next;
            next = player->hand[i];
            player->hand[i] = temp;
        } while (i++ < player->num_cards);
    } else
        player->hand[i] = to_insert;

    player->num_cards++;

    return i;
}

static Card *remove_from_hand(Player *player, int cardno) {
    int i;
    Card *remove = player->hand[cardno];
    
    for (i = cardno; i < player->num_cards - 1; i++)
        player->hand[i] = player->hand[i + 1];

    player->num_cards--;

    return remove;
}

static unsigned int deal_card(Player *player, Deck *deck, int decksize) {
    unsigned int ri;

    do {
        ri = rand() % decksize;
    } while (deck->cards[ri].state != IN_DECK);

    insert_sorted(player, &deck->cards[ri]);
    inc_card_suit(player, deck->cards[ri].suit);
    deck->cards[ri].state = IN_HAND;

    return ri;
}


#if 0
static unsigned int deal_card(Player *player, Card *deck, int decksize) {
    unsigned int ri;

    do {
        ri = rand() % decksize;
    } while (deck[ri].state != IN_DECK);

    insert_sorted(player, deck + ri);
    inc_card_suit(player, deck[ri].suit);
    deck[ri].state = IN_HAND;

    return ri;
}
#endif


/**
 * Used for debugging, currently prints the contents of card to stdout.
 */
void printcard(Card *card, unsigned int option) {
    unsigned char print_type;

    print_type = option & PRINT_DEBUG;

    if (print_type)
        printf("Card: ");
    switch (card->value) {
    case TWO:
    case THREE:
    case FOUR:
    case FIVE:
    case SIX:
    case SEVEN:
    case EIGHT:
    case NINE:
    case TEN:
        printf("%d", card->value);
        break;
    case ACE_LOW:
        printf(print_type ? "ACE_LOW" : "A");
        break;
    case JACK:
        printf(print_type ? "JACK" : "J");
        break;
    case QUEEN:
        printf(print_type ? "QUEEN" : "Q");
        break;
    case KING:
        printf(print_type ? "KING" : "K");
        break;
    case ACE_HIGH:
        printf(print_type ? "ACE_HIGH" : "A");
        break;
    default:
        printf("UNKNOWN_VALUE(%d)", card->value);
        break;
    }

    if (print_type)
        printf(" of ");

    switch (card->suit) {
    case SPADES:
        printf(print_type ? "spades" : "S");
        break;
    case HEARTS:
        printf(print_type ? "hearts" : "H");
        break;
    case CLUBS:
        printf(print_type ? "clubs" : "C");
        break;
    case DIAMONDS:
        printf(print_type ? "diamonds" : "D");
        break;
    default:
        printf("UNKNOWN_SUIT(%d)", card->suit);
        break;
    }

    if (print_type) {
        printf(" state: ");
        
        switch (card->state) {
        case IN_HAND:
            printf("IN_HAND");
            break;
        case IN_DECK:
            printf("IN_DECK");
            break;
        case IN_PLAY:
            printf("IN_PLAY");
            break;
        case IN_DISCARD:
            printf("IN_DISCARD");
            break;
        default:
            printf("UNKNOWN_STATE(%d)", card->state);
            break;
        }

        printf("\n");
    }
}
