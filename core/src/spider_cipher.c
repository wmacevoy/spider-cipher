
#include "spider_cipher.h"

void spider_cipher_init(spider_cipher_deck_t deck)
{
    for (uint8_t i = 0; i < SPIDER_CIPHER_CARDS; ++i)
    {
        deck[i] = i;
    }
}

void spider_cipher_fini(spider_cipher_deck_t deck)
{
    spider_cipher_init(deck);
}

void spider_cipher_cut_at(spider_cipher_deck_t dst, const spider_cipher_deck_t src, uint8_t at)
{
    for (uint8_t i = 0; i < SPIDER_CIPHER_CARDS; ++i)
    {
        dst[i] = src[(i + at) % SPIDER_CIPHER_CARDS];
    }
}

void spider_cipher_back_front_shuffle(spider_cipher_deck_t dst, const spider_cipher_deck_t src)
{
    for (uint8_t i = 0; i < SPIDER_CIPHER_CARDS / 2; ++i)
    {
        dst[(SPIDER_CIPHER_CARDS/2)+i] = src[2 * i];
        dst[(SPIDER_CIPHER_CARDS/2)-1-i] = src[2 * i+1];
    }
    if (SPIDER_CIPHER_CARDS % 2 != 0)
    {
        dst[SPIDER_CIPHER_CARDS - 1] = src[SPIDER_CIPHER_CARDS - 1];
    }
    if (SPIDER_CIPHER_PERFECT) {
        /* swap positions 0 and CARDS/2-1 (0 and 19 for 40 cards), which is
           the same as swapping the 2nd and bottom cards of the cut deck
           before the shuffle (the human form).  This keeps the minimum
           first return to a rotation at 30; swapping first and last cards
           instead drops it to 6, below plain spider's 9. */
        uint8_t temp = dst[SPIDER_CIPHER_CARDS / 2 - 1];
        dst[SPIDER_CIPHER_CARDS / 2 - 1] = dst[0];
        dst[0] = temp;
        temp = 0;
    }
}

uint8_t spider_cipher_find(const spider_cipher_deck_t deck, uint8_t card)
{
    uint8_t at = 0;
    for (uint8_t i = 0; i < SPIDER_CIPHER_CARDS; ++i)
    {
        at = at | (i & -(deck[i] == card));
    }
    return at;
}

uint8_t spider_cipher_noise(const spider_cipher_deck_t deck)
{
    uint8_t tag = (deck[2] + (SPIDER_CIPHER_CARDS - 1)) % SPIDER_CIPHER_CARDS;
    uint8_t at = spider_cipher_find(deck, tag);
    uint8_t noise = deck[(at + 1) % SPIDER_CIPHER_CARDS];
    tag = 0;
    at = 0;
    return noise;
}

uint8_t spider_cipher_scramble(spider_cipher_deck_t deck, uint8_t clear) {
    uint8_t noise = spider_cipher_noise(deck);
    uint8_t scrambled = (clear + noise) % SPIDER_CIPHER_CARDS;
    uint8_t cut = (deck[0] + clear) % SPIDER_CIPHER_CARDS;
    uint8_t cut_at = spider_cipher_find(deck, cut);

    spider_cipher_deck_t temp;
    spider_cipher_cut_at(temp, deck, cut_at);
    spider_cipher_back_front_shuffle(deck, temp);

    noise=0;
    cut=0;
    cut_at=0;
    spider_cipher_fini(temp);

    return scrambled;
}

void spider_cipher_update(spider_cipher_deck_t deck, uint8_t clear) {
    uint8_t cut = (deck[0] + clear) % SPIDER_CIPHER_CARDS;
    uint8_t cut_at = spider_cipher_find(deck, cut);

    spider_cipher_deck_t temp;
    spider_cipher_cut_at(temp, deck, cut_at);
    spider_cipher_back_front_shuffle(deck, temp);

    cut=0;
    cut_at=0;
    spider_cipher_fini(temp);
}

uint8_t spider_cipher_unscramble(spider_cipher_deck_t deck, uint8_t scrambled) {
    uint8_t denoise = SPIDER_CIPHER_CARDS-spider_cipher_noise(deck);
    uint8_t clear = (scrambled+denoise) % SPIDER_CIPHER_CARDS;
    uint8_t cut = (deck[0] + clear) % SPIDER_CIPHER_CARDS;
    uint8_t cut_at = spider_cipher_find(deck, cut);

    spider_cipher_deck_t temp;
    spider_cipher_cut_at(temp, deck, cut_at);
    spider_cipher_back_front_shuffle(deck, temp);

    denoise=0;
    cut=0;
    cut_at=0;
    spider_cipher_fini(temp);
    return clear;
}


