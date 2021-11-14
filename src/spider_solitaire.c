#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "spider_solitaire.h"

static int CIORandRead(CIORand *me) {
  uint8_t x;
  
  do {
    int status = fread((char*) &x,sizeof(x),1,me->urand);
    if (status <= 0) return -1;
  } while (x >= (256-256%CARDS));
  return x % CARDS;
}

static void CIORandClose(CIORand *me) {
  fclose(me->urand);
  me->urand=NULL;
}

void CIORandInit(CIORand *me) {
  CIOInit(&(me->base));
  me->base.read=(CIOReadPtr) &CIORandRead;
  me->base.write=NULL;
  me->base.peek=NULL;
  me->base.close=(CIOClosePtr) &CIORandClose;
  me->urand=fopen("/dev/urandom","rb");
}


#define ADD(x,y) (((x)+(y))%CARDS)
#define SUB(x,y) (((x)+(CARDS-(y)))%CARDS)

static const unsigned MASK[] = {0U,~0U};

#define CUT_PAD(deck) deck[CUT_ZTH]
#define CUT_CARD(deck,plain) ADD(CUT_PAD(deck),plain)

#define MARK_CARD(deck) ADD(deck[MARK_ZTH],MARK_ADD)

#if (1 == 1) == 1
#define FIND1(deck,offset,card)			\
  ((offset) & MASK[(deck)[offset]==(card)])
#elif (1 == 1) == ~0
#define FIND1(deck,offset,card)			\
  ((offset) & (deck)[offset]==(card))
#else
#error C compiler defines true as neither 1 nor ~0
#endif

#define FIND5(deck,offset,card)			\
  FIND1(deck,offset+0,card) |			\
    FIND1(deck,offset+1,card) |			\
    FIND1(deck,offset+2,card) |			\
    FIND1(deck,offset+3,card) |			\
    FIND1(deck,offset+4,card)
  
#define FIND10(deck,offset,card)			\
  FIND5(deck,offset,card)|FIND5((deck),offset+5,card)

#define FIND20(deck,offset,card)			\
  FIND10(deck,offset,card)|FIND10(deck,offset+10,card)

#define FIND(deck,card)				\
  (FIND20(deck,0,card)|FIND20(deck,20,card))

int cardFaceNo(Card card)
{
  return card%10;
}

int cardSuiteNo(Card card)
{
  return card/10;
}

wchar_t cardFaceFromNo(int cardFaceNo)
{
  return L"QA23456789"[cardFaceNo];
}

wchar_t cardSuiteFromNo(int cardSuiteNo)
{
  return L"\u2663\u2666\u2665\u2660"[cardSuiteNo];
}

int cardFromFaceSuiteNo(int cardFaceNo, int cardSuiteNo) {
  if (0 <= cardFaceNo && cardFaceNo < 10 &&
      0 <= cardSuiteNo && cardSuiteNo < 4) {
    return 10*cardSuiteNo+cardFaceNo;
  } else {
    return -1;
  }
}

Card cardAdd(Card x, Card y)
{
  return ADD(x,y);
}

Card cardSubtract(Card x, Card y)
{
  return SUB(x,y);
}

void deckInit(Deck deck) {
  for (int i=0; i<CARDS; ++i) {
    deck[i]=i;
  }
}

void deckCut(Deck input, int cutLoc,Deck output)
{
  for (int i=0; i<CARDS; ++i) {
    output[i]=input[(i+cutLoc) % CARDS];
  }
}

void deckBackFrontShuffle(Deck input, Deck output)
{
  int back = CARDS/2;
  int front = CARDS/2-1;
  for (int i=0; i<CARDS; i += 2) {
    output[back]=input[i];
    output[front]=input[i+1];
    ++back;
    --front;
  }
}

int deckFindCard(Deck deck,Card card)
{
  return FIND(deck,card);
}

void deckPseudoShuffle(Deck deck, int cutLoc) {
  Deck temp;
  deckCut(deck,cutLoc,temp);
  deckBackFrontShuffle(temp,deck);
}

Card deckCutPad(Deck deck) {
  Card cutPad=deck[CUT_ZTH];
  return cutPad;
}

Card deckCipherPad(Deck deck) {
  Card markCard = MARK_CARD(deck);
  int markLoc = deckFindCard(deck,markCard);
  Card cipherPad = deck[ADD(markLoc,1)];
  return cipherPad;
}

void deckAdvance(Deck deck, Card inputCard, Card *outputCard, int mode) {
  Card plainCard,cipherCard;

  if (mode == 1) {
    plainCard=inputCard;
    if (outputCard != NULL) {
      Card cipherPad = deckCipherPad(deck);
      cipherCard=cardAdd(plainCard,cipherPad);
      *outputCard=cipherCard;
    }
  } else if (mode == -1) {
    cipherCard=inputCard;
    Card cipherPad = deckCipherPad(deck);    
    plainCard=cardSubtract(cipherCard,cipherPad);
    if (outputCard != NULL) {
      *outputCard=plainCard;
    }
  } else {
    assert(0);
  }

  Card cutPad = deckCutPad(deck);
  Card cutCard = cardAdd(plainCard,cutPad);
  int cutLoc = deckFindCard(deck,cutCard);
  deckPseudoShuffle(deck,cutLoc);
}

Card deckEncryptCard(Deck deck, Card plainCard) {
  Card cipherCard=~0;
  deckAdvance(deck,plainCard,&cipherCard,1);
  return cipherCard;
}

Card deckDecryptCard(Deck deck, Card cipherCard) {
  Card plainCard=~0;
  deckAdvance(deck,cipherCard,&plainCard,-1);
  return plainCard;
}

#define CODE_LEN        36
#define SHIFT_DOWN      36
#define SHIFT_UP        37
#define SHIFT_LOCK_DOWN 38
#define SHIFT_LOCK_UP   39

const wchar_t *const DOWN_CODES = L"0123456789ABCDEF@=\\~#$%^&|-+/*\n;?\'\U0001F622\U0001F604";
const wchar_t *const     CODES = L"abcdefghijklmnopqrstuvwxyz<>() ,.\"\U0001F44E\U0001F44D";
const wchar_t *const  UP_CODES = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ{}[]_:!`\U0001F494\u2764";
const wchar_t *const ALL_CODES[] = { DOWN_CODES, CODES, UP_CODES };


static int find(const wchar_t *str, int len, int code)
{
  if (code >= 0) {
    for (int pos=0; pos<len; ++pos) {
      if (str[pos] == code) {
	return pos;
      }
    }
  }
  return -1;
}

static int ord(int shift, int code)
{
  if (code < 0) return -1;
  
  if (shift >= 0) {
    return find(ALL_CODES[shift],CODE_LEN,code);
  } else {
    for (int i=0; i<3; ++i) {
      if (find(ALL_CODES[i],CODE_LEN,code) >= 0) return -1;
    }
    return code;
  }
}

#define ENCODE_WRITE(card) { if (CIOWrite(out,card) != 0) return -1; else ++count; }

int encodeIO(CIO *in, CIO *out)
{
  int shift = 1;
  int next = 1;
  int count = 0;
  
  for (;;) {
    int chr = CIOPeek(in,0);
    if (chr < 0) { CIORead(in); break; }
    int pos = ord(shift,chr);
    
    if (pos >= 0) {
      CIORead(in);
      if (shift == -1) {
	int s=4;
	while ((((uint64_t) 1)<<s) < ((uint64_t)chr)) { s += 4; }
	while (s >= 4) {
	  s -= 4;
	  uint8_t hex = (chr >> s) & 0xF;
	  ENCODE_WRITE(hex);
	}
	if (ord(-1,CIOPeek(in,1)) >= 0) {
	  ENCODE_WRITE(SHIFT_LOCK_DOWN);
	  next=-1;
	} else {
	  ENCODE_WRITE(SHIFT_LOCK_UP);
	  next=0;
	}
      } else {
	ENCODE_WRITE(pos);
      }
      shift = next;
      continue;
    }

    // find best next shift state en[q] for shift state q-1
    // preferring nearby shift states for ties.  Ties are
    // only possible for capital letter A-F sequences, which are
    // encoded UP and DOWN as CAPS and HEX.  At most 10 (PREFIX)
    // characters are looked at for

    // look forward only until ties are broken or end of string
    int en[4]={0,0,0,0};
    for (int el=0; ; ++el) {
      int ties = 0;
      for (int q=-1; q<3; ++q) {
	if (en[q+1] == el && ord(q,in->peek(in,el)) >= 0) { ++en[q+1]; ++ties; }
      }
      if (ties < 2) break;
    }

    // choose longer encoding or nearest (larger) shift state for ties
    int m=-1;
    for (int q=0; q<3; ++q) {
      if (en[m+1] < en[q+1] || (en[m+1] == en[q+1] && abs(q-next) <= abs(m-next))) {
	m = q;
      }
    }

    // lock for unicode, multiple matches or next shift state makes no progress
    int lock = (m == -1) || (en[m+1] > 1) || (ord(next,in->peek(in,1)) < 0);

    while (shift != m) {
      if (shift < m) {
	++shift;
	Card up = lock ? SHIFT_LOCK_UP : SHIFT_UP;
	ENCODE_WRITE(up);
      }
      if (m < shift) {
	--shift;
	Card down = lock ? SHIFT_LOCK_DOWN : SHIFT_DOWN;
	ENCODE_WRITE(down);
      }      
    }
    if (lock) { next = shift; }
  }
  for (int i=0; (i<PREFIX/2) || ((count % PREFIX) != 0); ++i) {
    ENCODE_WRITE(CARDS-1);
  }

  return count;
}


#define DECODE_WRITE(chr) { if (out != NULL) { if (out->write(out,(chr)) != 0) { return -1; } } ++count; shift = next; }

int decodeIO(CIO *in, CIO *out) {
  int count = 0;
  int shift = 1;
  int next = 1;
  int unicode = -1;
  int pad = 0;
  for (;;) {
    int card = in->read(in);
    if (card == CARDS-1) {
      ++pad;
      if (pad >= PREFIX/2) {
	out = NULL;
      }
    } else {
      if (pad < PREFIX/2) {
	pad = 0;
      }
    }
    if (shift == -1) {
      if (0 <= card && card < 16) { // base 16 encoding
	unicode = (unicode > 0 ? 16*unicode : 0) + card;
      } else if (20 <= card && card < 30) { // base 10 encoding
	unicode = (unicode > 0 ? 10*unicode : 0) + (card-20);
      } else {
	if (unicode >= 0) {
	  DECODE_WRITE(unicode);
	}
	unicode=-1;
      }
    }
    if (shift >= 0 && 0 <= card && card < 36) { 
      int chr = ALL_CODES[shift][card];
      DECODE_WRITE(chr);
    }
    if (card < 0) return (card == -1 && pad >= PREFIX/2) ? count : -1;

    if ((card == SHIFT_UP || card == SHIFT_LOCK_UP) && shift <= 1) {
      ++shift;
    }
    if ((card == SHIFT_DOWN || card == SHIFT_LOCK_DOWN) && shift >= 0) {
      --shift;
    }
    if (card == SHIFT_LOCK_DOWN || card == SHIFT_LOCK_UP) {
      next = shift;
    }
  }
}

static int CIOTranslateRead(CIOTranslate *me) {
  int card=CIORead(me->io);
  if (card < 0) {
    return card;
  }
  Card translate;
  deckAdvance(me->deck,card,&translate,me->mode);
  return translate;
}

static int CIOTranslateWrite(CIOTranslate *me,int card) {
  if (card >= 0) {
    Card translate;
    deckAdvance(me->deck,card,&translate,me->mode);
    return CIOWrite(me->io,translate);
  } else {
    return -1;
  }
}

static void CIOTranslateClose(CIOTranslate *me) {
  CIOClose(me->io);
}

void CIOTranslateInit(CIOTranslate *me, CIO *io, Deck deck, int mode) {
  me->base.read = (CIOReadPtr) &CIOTranslateRead;
  me->base.write = (CIOWritePtr) &CIOTranslateWrite;
  me->base.peek = NULL;
  me->base.close = (CIOClosePtr) &CIOTranslateClose;
  me->io=io;
  me->deck=deck;
  me->mode=mode;
}

struct CIOEnvelopeStruct;
typedef struct CIOEnvelopeStruct CIOEnvelope;
struct CIOEnvelopeStruct {
  CIO base;
  CIOTranslate trans;
  CIO *rng;
  int len;
};

static int CIOEnvelopeRead(CIOEnvelope *me) {
  if (me->len == 0) {
    for (int i=0; i<PREFIX; ++i) {
      int pfx = CIORead(&me->trans);
      if (pfx < 0) { 
	return pfx;
      }
      if (me->rng != NULL) { // deniable stream
	int rngStatus=CIOWrite(me->rng,pfx);
	if (rngStatus < 0) {
	  return rngStatus;
	}
      }
    }
  }

  int pfx = CIORead(&me->trans);
  if (pfx < 0) { 
    return pfx;
  }
  if (me->rng != NULL) {
    int rngStatus=CIOWrite(me->rng,pfx);
    if (rngStatus < 0) {
      return rngStatus;
    }
  }
  int card = CIORead(&me->trans);
  if (card >= 0) {
    ++me->len;
    return card;
  } else {
    return -1;
  }
}

static int CIOEnvelopeWrite(CIOEnvelope *me, int card) {
  int status = 0;
  while (me->len < PREFIX) {
    int rand = CIORead(me->rng);
    if (rand < 0) {
      return rand;
    }
    status = CIOWrite(&me->trans,rand);
    if (status < 0) { 
      return status;
    }
    ++me->len;
  }
  int rand = CIORead(me->rng);
  if (rand < 0) {
    return rand;
  }
  status = CIOWrite(&me->trans,rand);
  if (status < 0) { 
    return status;
  }
  ++me->len;

  status = CIOWrite(&me->trans,card);
  if (status < 0) { 
    return status;
  }
  ++me->len;
  return 0;
}

static void CIOEnvelopeClose(CIOEnvelope *me) {
  CIOClose(&me->trans);
}

static void CIOEnvelopeInit(CIOEnvelope *me,Deck deck, int mode, CIO *rng, CIO *io) {
  me->base.read=(CIOReadPtr) &CIOEnvelopeRead;
  me->base.write=(CIOWritePtr) &CIOEnvelopeWrite;
  me->base.peek=NULL;
  me->base.close=(CIOClosePtr) &CIOEnvelopeClose;
  CIOTranslateInit(&me->trans,io,deck,mode);
  me->rng=rng;
  me->len=0;
}

int encryptEnvelopeIO(Deck deck, CIO *in, CIO *rng, CIO *out) {
  CIOEnvelope envOut;
  CIOEnvelopeInit(&envOut,deck,1,rng,out);
  if (encodeIO(in,&envOut.base)>0) {
    return envOut.len;
  } else {
    return -1;
  }
}

int decryptEnvelopeIO(Deck deck, CIO *in, CIO *deniableOut, CIO *out) {
  CIOEnvelope envIn;
  CIOEnvelopeInit(&envIn,deck,-1,deniableOut,in);
  return decodeIO(&envIn.base,out);
}
