#ifndef SPIDER_CIPHER_H
#define SPIDER_CIPHER_H

#include <stdint.h>
#if !defined(SPIDER_CIPHER_CARDS)
#define SPIDER_CIPHER_CARDS 40
#endif

#if !defined(SPIDER_CIPHER_PERFECT)
#define SPIDER_CIPHER_PERFECT 0
#endif

#ifdef __cplusplus
extern "C" {
#endif


typedef uint8_t spider_cipher_deck_t[SPIDER_CIPHER_CARDS];

/* Initialize deck to standard order 0..CARDS-1 */
void spider_cipher_init(spider_cipher_deck_t deck);

/* Reset deck to standard order 0..CARDS-1 */
void spider_cipher_fini(spider_cipher_deck_t deck);

/* Cut deck at position at */
void spider_cipher_cut_at(spider_cipher_deck_t dst, const spider_cipher_deck_t src, uint8_t at);

/* Perform a back-front shuffle from src to dst
   while not done:
     card 0,2,4,... move to back of new (shuffled) deck
     card 1,3,5,... move to front of new (shuffled) deck
   if perfect shuffle, swap shuffled positions 0 and CARDS/2-1
   (equivalently: swap the 2nd and bottom cards before the shuffle)
*/
void spider_cipher_back_front_shuffle(spider_cipher_deck_t dst, const spider_cipher_deck_t src);

/* Find position of card in deck */
uint8_t spider_cipher_find(const spider_cipher_deck_t deck, uint8_t card);

/* Get noise value from deck (used to scramble/encrypt). */
uint8_t spider_cipher_noise(const spider_cipher_deck_t deck);

/* Update deck: cut on (clear + first card), then back-front shuffle */
void spider_cipher_update(spider_cipher_deck_t deck, uint8_t clear);

/* Like update, but add noise to clear card to scramble / encrypt it */
uint8_t spider_cipher_scramble(spider_cipher_deck_t deck, uint8_t clear);

/* Like update, but subtract noise from scrambled card to unscramble / decrypt it */
uint8_t spider_cipher_unscramble(spider_cipher_deck_t deck, uint8_t scrambled);

/* Use OS RNG to shuffle deck */
void spider_cipher_shuffle(spider_cipher_deck_t deck);
  
#ifdef __cplusplus
}
#endif

#endif
