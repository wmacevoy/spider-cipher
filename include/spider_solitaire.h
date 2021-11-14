#pragma once

#include <stdio.h>
#include "cio.h"

#ifdef __cplusplus
extern "C" {
#endif

  struct CIORandStruct;
  typedef struct CIORandStruct CIORand;
  struct CIORandStruct {
    CIO base;
    FILE *urand;
  };
  
  void CIORandInit(CIORand *me);
  
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

  int cardFaceNoFromFace(wchar_t face);

  /* unicode club, diamond, heart, spade from 0,1,2,3 */
  wchar_t cardSuiteFromNo(int cardSuiteNo);
  
  int cardSuiteNoFromSuite(wchar_t suite);

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

  struct CIOTranslateStruct;

  typedef struct CIOTranslateStruct CIOTranslate;
  struct CIOTranslateStruct {
    CIO base;
    CIO *io;
    Card *deck;
    int mode;
  };

  void CIOTranslateInit(CIOTranslate *me, CIO *io, Deck deck, int mode);

  int encodeIO(CIO *in, CIO *out);
  
  int decodeIO(CIO *in, CIO *out);
  int decodeLen(Card *cards, int cardsLen);
  int envelopeLen(int encodeLen);

  int encryptEnvelopeIO(Deck deck, CIO *in, CIO *rng, CIO *out);
  int encryptEnvelopeArray(Deck deck, wchar_t *str, int strLen,
			   CIO *rng, Card *cards, int capacity);

  int decryptEnvelopeIO(Deck deck, CIO *in, CIO *deniableOut, CIO *out);
  int decryptEnvelopeArray(Deck deck, Card *cards, int cardLen,
			   wchar_t *str, int strCapacity);  
  
#ifdef __cplusplus
}
#endif
