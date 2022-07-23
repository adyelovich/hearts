#include <stdio.h>
#include <stdlib.h>
#include "cards.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: deal LARGEST_CARD NUM_PLAYERS HAND_SIZE\n");
        return 1;
    }



    /*******************TODO****************/
    /*Need to figure out logic on handling dealing cards when the number to deal exceeds the total
      legal amount of cards that can be held by the table.
      For example, I want to deal 30 cards yet each player can only hold 3 cards, meaning a total of
      12 can be dealt.
    */
    Deck *deck;
    Player *table;
    int largecard, numplayers, handsize, dealt;
    
    
    largecard = atoi(argv[1]);
    numplayers = atoi(argv[2]);
    handsize = atoi(argv[3]);
    //int size = gensd(&deck, TWO, FOUR, 1);
    deck = malloc(sizeof(deck));
    table = malloc(numplayers * sizeof(Player));
    
    
    gensd(deck, TWO, largecard, 1);
    gentable(table, numplayers, handsize);
    
    dealt = dealdeck(table, deck, handsize, numplayers, (largecard - 1) * 4);
    
    
    printf("JUST FOR FUN\n");
    /*printcard(table[0].hand[3]);*/

    for (int i = 0; i < numplayers; i++) {
        printf("Player %d hand:\n", i);
        for (int j = 0; j < handsize; j++) {
            printcard(table[i].hand[j], PRINT_DEBUG);
        }
    }

    for (int i = 0; i < numplayers; i++) {
        printf("Player %d ", i);
        print_hand(&table[i]);
        printf("\n");
    }

    for (int i = 0; i < handsize; i++) {
        printcard(play_card(&table[0], NULL), PRINT_DEBUG);
        printf("\n");
        for (int j = 0; j < table[0].num_cards; j++) {
            printcard(table[0].hand[j], PRINT_DEBUG);
        }
    }
    
    return 0;
}
