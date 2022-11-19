#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include "cards.h"

#define IN_RANGE(check, max) ((check) >= 1 && (check) <= (max))

static int insert_sorted(Player *player, Card *to_insert);
static Card *remove_from_hand(Player *player, int cardno);
static unsigned int deal_card(Player *player, Deck *deck, int decksize);
static char *card_string_abbrev(char *name, Value value, Suit suit);
static int extract_from_card_name(const char *name, Value *value, Suit *suit);
static Value string_to_suit(const char *s);
static Suit string_to_value(const char *s);
static int prompt_for_card_name(char *name, Player *player);
static int valid_card_name_format(const char *value, const char *suit);
static int player_card_index(Player *player, const Card *card);
static void str_toupper(char *s);

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
        for (i = SPADES; i <= DIAMONDS; i++)
            for (j = start; j <= end; j++) {
                p->value = j;
               p->suit = i;
                p->state = IN_DECK;
                p->name = malloc(4);
                card_string_abbrev(p->name, p->value, p->suit);
                p++;
            }

    return k;
}

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

/* passing in the separate decksize is not needed I don't think, it comes with deck */
int dealdeck(Player *players, Deck *deck, int n, int numplayers, int decksize) {
    int i;

    srand(time(NULL));
    n *= 4;

    for (i = 0; n-- > 0 && i < decksize; i++)
        deal_card(&players[i % numplayers], deck, decksize);

    return i;
}

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
    char card_name[CARD_NAME_LENGTH];

    do {
        print_hand(player);
        printf("Choose a card to play: ");
        valid = prompt_for_card_name(card_name, player);
        if (valid) {
            played = card_with_name(card_name, player->hand, player->num_cards);
            if (played == NULL) {
                printf("Invalid entry: play a card in your hand\n");
                valid = 0;
            } else {
                if (is_valid != NULL)
                    valid = is_valid(player, played);
            }
        }
    } while (!valid);

    num = player_card_index(player, played);
    player->hand[num]->state = IN_PLAY;
    dec_card_suit(player, player->hand[num]->suit);
    played = remove_from_hand(player, num);

    return played;
}

Card *card_with_name(const char *name, Card **cards, int num_cards) {
    int i;

    for (i = 0; i < num_cards; i++)
        if (strcmp(cards[i]->name, name) == 0)
            return cards[i];

    return NULL;
}

static int prompt_for_card_name(char *name, Player *player) {
    char buf[CARD_NAME_LENGTH + 1], svalue[3], ssuit[2];
    Value value;
    Suit suit;
    int done = 0;
    int n;
    
    while (!done && fgets(buf, CARD_NAME_LENGTH + 1, stdin) != NULL) {
        n = sscanf(buf, " %2[0-9AJKQajkq]%1[CDHScdhs] ", svalue, ssuit);

        if (n == 2 && valid_card_name_format(svalue, ssuit)) {
            str_toupper(svalue);
            str_toupper(ssuit);
            if (ssuit[0] == '0')
                ssuit[0] = ssuit[1];
            done = 1;
        } else {
            printf("Invalid card format!\n");
        }
    }

    strncpy(name, svalue, 3);
    strncat(name, ssuit, 2);

    return done;
}

static int valid_card_name_format(const char *value, const char *suit) {
    int ret = 0;

    
    if (value[0] == '1' && value[1] == '0')
        ret = 1;
    else if (value[0] != '1' && isdigit(value[0]) && value[1] == '\0')
        ret = 1;
    else {
        char temp = toupper(value[0]);
        if (temp == 'A' || temp == 'J' || temp == 'K' || temp == 'Q')
            ret = 1;
    }
    
    if (ret) {
        char temp = toupper(suit[0]);
        if (temp != 'C' && temp != 'D' && temp != 'H' && temp != 'S')
            ret = 0; /* turn off valid flag here */
    }

    return ret;
}

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

Player *find_player_with_card(Player *players, const Card *card, int numplayers) {
    int i, j, found;

    /* iterate over all the players */
    found = 0;
    for (i = 0; i < numplayers && !found; i++)
        found = player_card_index(&players[i], card) != -1 ? 1 : 0;

    return found ? players + i - 1 : NULL;
}

static int player_card_index(Player *player, const Card *card) {
    int i;

    /* stop once we hit the target suit */
    for (i = 0; i < player->num_cards && player->hand[i]->suit < card->suit; i++)
        ;
    
    /*stop once we hit the target value */
    for ( ; i < player->num_cards && player->hand[i]->value < card->value; i++)
        ;

    /* if the card we stopped at matches the target suit and value return the index */
    return player->hand[i]->suit == card->suit && player->hand[i]->value == card->value
        ? i : -1;
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

/********** START PRIVATE FUNCTIONS *************/

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

static void str_toupper(char *s) {
    for ( ; *s; s++)
        *s = toupper(*s);
}

 
 
/**************** START DEBUG FUNCTIONS ****************/

/**
 * Used for debugging, currently prints the contents of card to stdout.
 */
void printcard(const Card *card, unsigned int option) {
    char name[4];
    unsigned int print_type;

    
    print_type = option & PRINT_DEBUG;

    if (!print_type) {
        card_string_abbrev(name, card->value, card->suit);
        printf("%s", name);
    }

    #if 0
    
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
    
    #endif
}

static char *card_string_abbrev(char *name, Value value, Suit suit) {
    char sval[3], *ssuit;

    switch (value) {
    case TWO:
    case THREE:
    case FOUR:
    case FIVE:
    case SIX:
    case SEVEN:
    case EIGHT:
    case NINE:
    case TEN:
        snprintf(sval, 3, "%d", value);
        break;
    case ACE_LOW:
    case ACE_HIGH:
        snprintf(sval, 2, "%s", "A");
        break;
    case JACK:
        snprintf(sval, 2, "%s", "J");
        break;
    case QUEEN:
        snprintf(sval, 2, "%s", "Q");
        break;
    case KING:
        snprintf(sval, 2, "%s", "K");
        break;
    default:
        snprintf(sval, 2, "%s", "?");
        break;
    }

    switch (suit) {
    case SPADES:
        ssuit = "S";
        break;
    case HEARTS:
        ssuit = "H";
        break;
    case CLUBS:
        ssuit = "C";
        break;
    case DIAMONDS:
        ssuit = "D";
        break;
    default:
        ssuit = "?";
        break;
    }

    snprintf(name, 4, "%s%s", sval, ssuit);
    return name;
}
