#pragma once

#ifdef __cplusplus
extern "C" {
#endif
  /*  number of cards in deck */
#define CARDS  40

#define PREFIX    10

  /* cut card = deck[CUT_ZTH] */
#define CUT_ZTH 0
  /*  0-based index of cipher mark card */
#define MARK_ZTH 2
  /* mark card = (deck[MARK_ZTH] + MARK_ADD) % CARDS */
#define MARK_ADD 39

  /* 0..39 values only */
  typedef signed char Card;
  typedef Card Deck[CARDS];

  /* 0,1,2,3 ... 8,9 for Q,A,23 ... 8,9 of any suite */
  int cardFaceNo(Card card);

  /* 0,1,2,3 for club, diamond, heart, spade of any face */
  int cardSuiteNo(Card card);

  /* 'Q','A','1','2','3', ... '8','9' for 0,1,2,3,...,8,9 */
  wchar_t cardFaceFromNo(int cardFaceNo);

  /* unicode club, diamond, heart, spade from 0,1,2,3 */
  wchar_t cardSuiteFromNo(int cardSuiteNo);

  /* returns -1 if face or suite value are not in range */
  int cardFromFaceSuiteNo(int cardFaceNo, int cardSuiteNo);

  /* modular 40 add */
  Card cardAdd(Card x, Card y);

  /* modular 40 subtract */
  Card cardSubtract(Card x, Card y);

  /* set deck to values 0 .. 39 */
  void deckInit(Deck deck);

  /* fixed time deck cut at location, so output[0]=input[cutLoc]  */
  void deckCut(Deck input, int cutLoc,Deck output);

  /* back-front shuffle of card deck */
  void deckBackFrontShuffle(Deck input, Deck output);

  /* fixed time find card in deck (assumes card exists uniquely in deck) */
  int deckFindCard(Deck deck,Card card);

  /* in-place fixed-time pseudo-shuffle at given cut location */
  void deckPseudoShuffle(Deck deck, int cutLoc);

  /* determine cut pad card fron deck */
  Card deckCutPad(Deck deck);

  /* determine cipher pad card fron deck */
  Card deckCipherPad(Deck deck);

  /* encrypt (mode 1) or decrypt (mode -1) input and pseudo-shuffle deck,
     OutputCard can be null; this saves time if the result is not
     needed (for example, to key a deck) */
  void deckAdvance(Deck deck, Card inputCard, Card *outputCard, int mode);


  /* return encrypted card and advance deck */
  Card deckEncryptCard(Deck deck, Card plainCard);

  /* return decrypted card and advance deck */
  Card deckDecryptCard(Deck deck, Card plainCard);

  struct CardIOStruct;
  typedef struct CardIOStruct CardIO;
  struct CardIOStruct {
    int (*read)(CardIO *me);
    int (*write)(CardIO *me, int card);
    int (*peek)(CardIO *me, int offset);
    void (*close)(CardIO *me);
  };

  struct CardArrayIOStruct;
  typedef struct CardArrayIOStruct CardArrayIO;
  
  struct CardArrayIOStruct {
    CardIO base;
    Card *cards;
    int position;
    int step;
    int size;
    int capacity;
    int reads;

    int writes;
  };
  
  void CardArrayIOInit(CardArrayIO *me, Card *cards, int step, int size, int capacity);

  struct CardRandIOStruct;
  typedef struct CardRandIOStruct CardRandIO;
  struct CardRandIOStruct {
    CardIO base;
    FILE *urand;
  };

  void CardRandIOInit(CardRandIO *me);

  struct CardTranslateIOStruct;

  typedef struct CardTranslateIOStruct CardTranslateIO;
  struct CardTranslateIOStruct {
    CardIO base;
    CardIO *io;
    Card *deck;
    int mode;
  };

  void CardTranslateIOInit(CardTranslateIO *me, CardIO *io, Deck deck, int mode);
  struct WideCharIOStruct;
  typedef struct WideCharIOStruct WideCharIO;
  
  struct WideCharIOStruct {
    int (*read)(WideCharIO *me);
    int (*write)(WideCharIO *me, int ch);
    int (*peek)(WideCharIO *me, int offset);
    void (*close)(WideCharIO *me);
  };

  
  struct WideCharArrayIOStruct;
  typedef struct WideCharArrayIOStruct WideCharArrayIO;
  

  struct WideCharArrayIOStruct {
    WideCharIO base;
    wchar_t *chars;
    int position;
    int step;
    int size;
    int capacity;
    int reads;
    int writes;
  };
  
  void WideCharArrayIOInit(WideCharArrayIO *me, wchar_t *chars, int step, int size, int capacity);
  
  int encodeIO(WideCharIO *in, CardIO *out);
  int encodeArray(wchar_t *str, int strLen,
		  Card *cards, int cardCapacity);
  int encodeLen(wchar_t *str, int strLen);
  
  int decodeIO(CardIO *in, WideCharIO *out);
  int decodeArray(Card *cards, int cardsLen,
		  wchar_t *str, int capacity);
  int decodeLen(Card *cards, int cardsLen);
  int envelopeLen(int encodeLen);

  int encryptEnvelopeIO(Deck deck, WideCharIO *in, CardIO *rng, CardIO *out);
  int encryptEnvelopeArray(Deck deck, wchar_t *str, int strLen, CardIO *rng, Card *cards, int capacity);

  int decryptEnvelopeIO(Deck deck, CardIO *in, CardIO *deniableOut, WideCharIO *out);
  int decryptEnvelopeArray(Deck deck, Card *cards, int cardLen, wchar_t *str, int strCapacity);  
  
#ifdef __cplusplus
}
#endif
