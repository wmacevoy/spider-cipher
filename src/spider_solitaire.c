#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "spider_solitaire.h"

const char *const BASE40 = "BCDEFGHLMNPRSTUVWXYZbcdefghimnprstuvwxyz";  

static int CIORandRead(CIORand *me) {
  uint8_t x;
  
  do {
    int status = fread((char*) &x,sizeof(x),1,me->urand);
    if (status <= 0) return -1;
  } while (x >= (256-256%CARDS));

  Card cutCard = cardAdd(deckCutPad(me->pool),x%CARDS);
  int cutLoc = deckFindCard(me->pool,cutCard);
  deckCut(me->pool,cutLoc,me->tmp);
  deckBackFrontShuffle(me->tmp,me->pool);
  return deckCipherPad(me->pool);
}

static void CIORandClose(CIORand *me) {
  fclose(me->urand);
  me->urand=NULL;
  deckInit(me->pool);
  deckInit(me->tmp);
}

void CIORandInit(CIORand *me) {
  CIOInit(&(me->base));
  me->base.read=(CIOReadPtr) &CIORandRead;
  me->base.write=NULL;
  me->base.peek=NULL;
  me->base.close=(CIOClosePtr) &CIORandClose;
  me->urand=fopen("/dev/urandom","rb");
  deckInit(me->pool);
  deckInit(me->tmp);
  for (int i=0; i<PREFIX; ++i) {
    CIORandRead(me);
  }
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

static const uint32_t FACES[] = {'Q','A','2','3','4','5','6','7','8','9'};

uint32_t cardFaceFromNo(int cardFaceNo)
{
  return FACES[cardFaceNo];
}

int cardFaceNoFromFace(uint32_t face) {
  for (int i=0; FACES[i] != 0; ++i) {
    if (FACES[i] == face) return i;
  }
  return -1;
}

static const uint32_t SUITES[] = {0x2663,0x2666,0x2665,0x2660};

uint32_t cardSuiteFromNo(int cardSuiteNo)
{
  return SUITES[cardSuiteNo];
}

int cardSuiteNoFromSuite(uint32_t suite) {
  for (int i=0; SUITES[i] != 0; ++i) {
    if (SUITES[i] == suite) return i;
  }
  return -1;
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
  for (int i=0; i<CARDS; ++i) {
    temp[i]=0;
  }
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

const uint32_t DOWN_CODES[] =
  {//Q    A    2    3    4    5    6    7    8    9
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',  // + 0 club 
    'A', 'B', 'C', 'D', 'E', 'F', '@', '=','\\', '~',  // +10 diamond
    '#', '$', '%', '^', '&', '|', '-', '+', '/', '*',  // +20 heart
   '\n', ';', '?','\'', 0x1F622, 0x1F604               // +30 spade  
  };

const uint32_t CODES[] =
  {//Q    A    2    3    4    5    6    7    8    9
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',  // + 0 club   
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',  // +10 diamond
    'u', 'v', 'w', 'x', 'y', 'z', '<', '>', '(', ')',  // +20 heart  
    ' ', ',', '.','\"',0x1F44E, 0x1F44D		       // +30 spade  
  };

const uint32_t UP_CODES[] =
  {//Q    A    2    3    4    5    6    7    8    9
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',  // + 0 club   
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',  // +10 diamond
    'U', 'V', 'W', 'X', 'Y', 'Z', '{', '}', '[', ']',  // +20 heart  
    '_', ':', '!', 0x1F494, 0x2764		       // +30 spade
  };

const uint32_t *const ALL_CODES[] = { DOWN_CODES, CODES, UP_CODES };


static int find(const uint32_t *str, int len, int code)
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
    // characters are looked at before it is a tie.

    int en[4]={0,0,0,0};
    for (int el=0; el < PREFIX; ++el) {
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

static int CIOCardsFmtWrite(CIOCardsFmt *me, int card) {
  if (card < 0 || card >= CARDS) {
    return -1;
  }
  
  int ws[4];
  int p=0;
  if (me->mode != 'b') {
    int c = CIOGetWrites(me);
    if (c > 0) {
      if (c % 20 == 0) {
	ws[p++]='\n';
      } else if (c % 5 == 0) {
	ws[p++]='|';
      } else {
      ws[p++]=' ';
      }
    }
  }

  if (me->mode == '#') {
    ws[p++]='0'+(card/10);
    ws[p++]='0'+(card%10);
    ws[p++]=0;
  } else if (me->mode == 'b') {
    ws[p++]=BASE40[card];
    ws[p++]=0;
  } else if (me->mode == 'c') {
    ws[p++]=cardFaceFromNo(cardFaceNo(card));
    ws[p++]=cardSuiteFromNo(cardSuiteNo(card));
    ws[p++]=0;
  } else {
    return -1;
  }

  for (int i=0; ws[i] != 0; ++i) {
    if (CIOWrite(me->io,ws[i]) == -1) return -1;
  }

  return 0;
}

static const char *WS=" \t\n\r.,-_()[]|";

static int CIOCardsFmtRead(CIOCardsFmt *me) {
  for (;;) {
    int c = CIOPeek(me->io,0);
    if (strchr(WS,c) == NULL) break;
    CIORead(me->io);
  }

  int c=CIOPeek(me->io,0);
  if (c == -1) return -1;
  
  const char *p=strchr(BASE40,c);
  if (p != NULL) {
    CIORead(me->io);      
    return p-BASE40;
  }
  
  int face = c;
  if (face == '0' || face == 'q') face = 'Q';
  if (face == '1' || face == 'a') face = 'A';
  int faceNo=cardFaceNoFromFace(face);
  if (faceNo != -1) {
    int suite = CIOPeek(me->io,1);
    if (suite == 'C' || suite == 'c') suite=cardSuiteFromNo(0);
    if (suite == 'D' || suite == 'd') suite=cardSuiteFromNo(1);
    if (suite == 'H' || suite == 'h') suite=cardSuiteFromNo(2);
    if (suite == 'S' || suite == 's') suite=cardSuiteFromNo(3);    

    int suiteNo=cardSuiteNoFromSuite(suite);
    if (suiteNo != -1) {
      CIORead(me->io);
      CIORead(me->io);	
      return cardFromFaceSuiteNo(faceNo,suiteNo);
    }
  }
  
  if ('0' <= c && c <= '9') {
    CIORead(me->io);
    int card = c-'0';
    c=CIOPeek(me->io,0);
    if ('0' <= c && c <= '9' && (card*10+(c-'0') < CARDS)) {
      CIORead(me->io);      
      card = card*10 + (c-'0');
    }
    return card;
  }

  return -1;
}

void CIOCardsFmtInit(CIOCardsFmt *me,CIO *io, int mode) {
  CIOInit(&me->base);
  me->io=io;
  me->base.read = (CIOReadPtr) &CIOCardsFmtRead;
  me->base.peek = NULL;
  me->base.write = (CIOWritePtr) &CIOCardsFmtWrite;
  me->mode=mode;
}

