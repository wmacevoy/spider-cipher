#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <assert.h>
#include <arpa/inet.h>

#include "facts.h"
#include "utf8.h"
#include "spider_solitaire.h"

const char* const UTF8_TEST_STRINGS [] =
  {
   u8"",
   u8"x",
   u8"xy",
   u8"X",
   u8"XY",
   u8"1",
   u8"12",
   u8"♣",
   u8"♣♠",
   u8"xX1♣xyXY12♣♠",
   u8"Spider Solitare",
   u8"int main(int argc, char** argv) {\n" 
   u8"  ::testing::InitGoogleTest(&argc, argv);\n" 
   u8"  return RUN_ALL_TESTS();\n" 
   u8"}\n",
   u8"Q(0)A(1)23456789♣+0a0AQ♣0/0b1BA♣1/-39c2C2♣2/-38d3D3♣3/-37e4E4♣4/-36f5F5♣5/-35g6G6♣6/-34h7H7♣7/-33i8I8♣8/-32j9J9♣9/-31♦+10kAKQ♦10/-30lBLA♦11/-29mCM2♦12/-28nDN3♦13/-27oEO4♦14/-26pFP5♦15/-25q@Q6♦16/-24r=R7♦17/-23s\\S8♦18/-22t~T9♦19/-21♥+20u#UQ♥20/-20v$VA♥21/-19w%W2♥22/-18x^X3♥23/-17y&Y4♥24/-16z|Z5♥25/-15<-{6♥26/-14>+}7♥27/-13(/[8♥28/-12)*]9♥29/-11♠+30☐↩_Q♠30/-10,;:A♠31/-9.?!2♠32/-8\"\'`3♠33/-7👍😄❤️4♠34/-6👎😢💔5♠35/-5↓6♠36/-4↑7♠37/-3⇊8♠38/-2⇈9♠39/-1",
};

const uint32_t* testString(int i) {
  const char *utf8=UTF8_TEST_STRINGS[i];
  int u32len=utf8decode(utf8, strlen(utf8), NULL, 0);
  uint32_t *u32 = (uint32_t*)malloc(sizeof(uint32_t)*(u32len+1));
  utf8decode(utf8, strlen(utf8), u32, u32len);
  u32[u32len]=0;
  return u32;
}

#define DECK_EQ(a,b) { int i; for (i=0; i<CARDS; ++i) { FACT(a[i],==,b[i]); } }
#define DECK_NE(a,b) { int i, ne=0; for (i=0; i<CARDS; ++i) { ne = ne || (a[i] != b[i]); }; FACT(ne,==,1); }


struct CIONotRandStruct;
typedef struct CIONotRandStruct CIONotRand;

struct CIONotRandStruct {
  CIO base;
  int state;
};

int CIONotRandRead(CIONotRand *me) {
  int ans=0;
  if (me->state < PREFIX) {
    ans = me->state + 1; // 1--10 for prefix */
  } else {
    ans = CARDS-1-((me->state-PREFIX) % CARDS);
  }
  ++me->state;
  return ans;
}

void CIONotRandInit(CIONotRand *me) {
  CIOInit(&me->base);
  me->base.read = (CIOReadPtr) &CIONotRandRead;
  me->state = 0;
}

FACTS_EXCLUDE(RandStats) {
  int n = 10*1000*1000;
  int counts1[CARDS];
  int counts2[CARDS][CARDS];
  CIORand rcg;
  CIORandInit(&rcg);

  for (int i=0; i<CARDS; ++i) {
    counts1[i]=0;
    for (int j=0; j<CARDS; ++j) {
      counts2[i][j]=0;
    }
  }

  int c1=CIORead(&rcg);
  for (int i=0; i<n; ++i) {
    int c0=CIORead(&rcg);
    ++counts1[c0];
    ++counts2[c0][c1];
    c1=c0;
  }
  CIOClose(&rcg);

  double p1 = 1.0/CARDS;
  double q1 = 1-p1;
  double mu1 = n*p1;
  double inv_sigma1 = 1/sqrt(mu1);
  double z1=0.0;

  for (int i=0; i<CARDS; ++i) {
    double zi=inv_sigma1*(counts1[i]-mu1);
    FACT(fabs(zi),<=,6.0);
    z1 += pow(zi,2);
  }

  z1 = sqrt(2.0)*(sqrt(z1)-sqrt(CARDS-1.5));

  double p2 = pow(1.0/CARDS,2);
  double q2 = 1-p2;
  double mu2 = n*p2;
  double inv_sigma2 = 1/sqrt(mu2);
  double z2=0.0;

  for (int i=0; i<CARDS; ++i) {
    for (int j=0; j<CARDS; ++j) {
      double zij=inv_sigma2*(counts2[i][j]-mu2);
      FACT(fabs(zij),<=,6.0);
      z2 += pow(zij,2);
    }
  }

  z2 = sqrt(2.0)*(sqrt(z2)-sqrt(pow(CARDS,2)-1.5));

  FACT(fabs(z1),<=,6.0);
  FACT(fabs(z2),<=,6.0);  
}


void testBackFrontShuffle(const Deck in, Deck out)
{
  int n=0;
  for (int i=0; i<CARDS; ++i) {
    int j=(i%2 == 0) ? n : 0;
    for (int k=n; k>j; --k) {
      out[k]=out[k-1];
    }
    out[j]=in[i];
    ++n;
  }
}

void testShuffle(const Deck in, Deck out) {
  for (int i=0; i<CARDS; ++i) {
    out[i]=in[i];
  }
  for (int i=0; i<CARDS; ++i) {
    int n=(CARDS-i);
    int r = (17*n+3) % n;
    int j = i + r;
    int tmp=out[i];
    out[i]=out[j];
    out[j]=tmp;
  }
}

void testCut(const Deck in, int cutPos, Deck out) {
  for (int i=0; i<CARDS; ++i) {
    out[i]=in[(i+cutPos) % CARDS];
  }
}

int testFindCard(const Deck deck, Card card) {
  for (int i=0; i<CARDS; ++i) {
    if (deck[i] == card) {
      return i;
    }
  }
  return -1;
}

Card testCutPad(const Deck deck) {
  return deck[CUT_ZTH];
}

Card testCipherPad(const Deck deck) {
  int markCard = (deck[MARK_ZTH]+MARK_ADD) % CARDS;
  int markLoc = testFindCard(deck, markCard);
  return deck[(markLoc + 1) % CARDS];
}

FACTS(FaceAndSuitNo) {
  for (int suitNo = 0; suitNo <= 3; ++suitNo) {
    for (int faceNo = 0; faceNo <= 9; ++faceNo) {
      Card card = 10*suitNo+faceNo;
      FACT(cardFaceNo(card),==,faceNo);
      FACT(cardSuitNo(card),==,suitNo);
    }
  }
}

FACTS(FaceFromNo) {
  FACT(cardFaceFromNo(0),==,'Q');
  FACT(cardFaceFromNo(1),==,'A');
  FACT(cardFaceFromNo(2),==,'2');
  FACT(cardFaceFromNo(3),==,'3');
  FACT(cardFaceFromNo(4),==,'4');
  FACT(cardFaceFromNo(5),==,'5');
  FACT(cardFaceFromNo(6),==,'6');
  FACT(cardFaceFromNo(7),==,'7');
  FACT(cardFaceFromNo(8),==,'8');
  FACT(cardFaceFromNo(9),==,'9');
}

FACTS(SuitFromNo) {
  const uint32_t u32[]={0x2663,0x2666,0x2665,0x2660};
  const char *utf8[]={u8"♣",u8"♦",u8"♥",u8"♠"};
  for (int i=0; i<4; ++i) {
    FACT(utf8decval(utf8[i],utf8declen(utf8[i],strlen(utf8[i]))),==,u32[i]);
    FACT(cardSuitFromNo(i),==,u32[i]);
  }
}

FACTS(CardFromFaceSuitNo) {
  for (int suitNo = -1; suitNo <= 4; ++suitNo) {
    for (int faceNo = -1; faceNo <= 10; ++faceNo) {
      int card = 10*suitNo+faceNo;
      if (suitNo < 0 || suitNo >= 4) { card = -1; }
      if (faceNo < 0 || faceNo >= 10) { card = -1; }      
      FACT(cardFromFaceSuitNo(faceNo,suitNo),==,card);
    }
  }
}

FACTS(Add) {
  for (int x = 0; x <= CARDS; ++x) {
    for (int y = 0; y <= CARDS; ++y) {
      int z = x + y;
      while (z < 0) z += CARDS;
      while (z >= CARDS) z -= CARDS;
      int result =cardAdd(x,y);
      FACT(z,==,result);
    }
  }
}

FACTS(Subtract) {
  for (int x = 0; x <= CARDS; ++x) {
    for (int y = 0; y <= CARDS; ++y) {
      int z = x - y;
      while (z < 0) z += CARDS;
      while (z >= CARDS) z -= CARDS;
      int result = cardSubtract(x,y);
      FACT(z,==,result);
    }
  }
}

FACTS(Init) {
  Deck deck;
  deckInit(deck);
  for (int i=0; i<CARDS; ++i) {
    FACT(deck[i],==,i);
  }
}

FACTS(Cut) {
  for (int cutLoc = 0; cutLoc < CARDS; ++cutLoc) {
    Deck input,output,expect;
    deckInit(input);
    deckInit(output);
    deckInit(expect);
    testCut(input,cutLoc,expect);
    deckCut(input,cutLoc,output);
    DECK_EQ(output,expect);
  }
}

FACTS(BackFrontShuffle) {
  Deck input,output,expect;
  deckInit(input);
  deckInit(output);
  deckInit(expect);
  testBackFrontShuffle(input,expect);
  deckBackFrontShuffle(input,output);
  DECK_EQ(output,expect);
}

FACTS(FindCard) {
  Deck deck,shuffled;
  deckInit(deck);
  deckInit(shuffled);
  testShuffle(deck,shuffled);
  for (int i=0; i<CARDS; ++i) {
    FACT(testFindCard(deck,i),==,deckFindCard(deck,i));
    FACT(testFindCard(shuffled,i),==,deckFindCard(shuffled,i));    
  }
}

FACTS(PseudoShuffle) {
  for (int cutLoc = 0; cutLoc < CARDS; ++cutLoc) {
    Deck input,output,tmp,expect;
    deckInit(input);
    deckInit(output);
    deckInit(tmp);
    deckInit(expect);
    testCut(input,cutLoc,tmp);
    testBackFrontShuffle(tmp,expect);
    deckPseudoShuffle(output,cutLoc);

    DECK_EQ(output,expect);
  }
}

FACTS(Pads) {
  const char *testString = "spidersolitare";
  Deck testDeck,tmp,deck;
  deckInit(testDeck);
  deckInit(tmp);
  deckInit(deck);
  
  for (int i=0; testString[i] != 0; ++i) {
    DECK_EQ(testDeck,deck);
    FACT(testCutPad(testDeck),==,deckCutPad(deck));
    FACT(testCipherPad(testDeck),==,deckCipherPad(deck));
    Card plain = testString[i]-'a';
    Card cutCard = cardAdd(plain,deckCutPad(deck));
    int cutLoc = deckFindCard(deck,cutCard);
    
    testCut(testDeck,cutLoc,tmp);
    testBackFrontShuffle(tmp,testDeck);
    deckPseudoShuffle(deck,cutLoc);
  }
}

FACTS(Ciphers) {
  const char *testString = "spidersolitare";
  Deck testDeck,tmp,deck;
  deckInit(testDeck);
  deckInit(tmp);
  deckInit(deck);

  for (int k=0; k<10; ++k) {
    for (int i=0; testString[i] != 0; ++i) {
      DECK_EQ(testDeck,deck);
      for (int j=0; j<CARDS; ++j) {
	tmp[j]=deck[j];
      }
      Card plainCard = testString[i]-'a';
      Card cipherCard = cardAdd(plainCard,testCipherPad(testDeck));
      Card cutCard = cardAdd(plainCard,testCutPad(testDeck));
      int cutLoc = testFindCard(testDeck,cutCard);
      for (int j=0; j<CARDS; ++j) {
	deck[j]=tmp[j];
      }
      FACT(testCipherPad(testDeck),==,deckCipherPad(deck));
      FACT(testCutPad(testDeck),==,deckCutPad(deck));      
      int cipher2 = deckEncryptCard(deck,plainCard);
      FACT(cipherCard,==,cipher2);
      
      for (int j=0; j<CARDS; ++j) {
	deck[j]=tmp[j];
      }
      FACT(testCipherPad(testDeck),==,deckCipherPad(deck));
      FACT(testCutPad(testDeck),==,deckCutPad(deck));      
      int plain2 = deckDecryptCard(deck,cipherCard);
      FACT(plainCard,==,plain2);

      deckPseudoShuffle(testDeck,cutLoc);
      DECK_EQ(testDeck,deck);
    }
  }
}

int wstrlen(const uint32_t *s) {  int n=0; while (*s++ != 0) ++n; return n; }

FACTS(Encode) {
  for (int i=0; i<sizeof(UTF8_TEST_STRINGS)/sizeof(char*); ++i) {
    const uint32_t *str=testString(i);
    int strLen = wstrlen(str);
    CIOArray plain;
    CIOArray encoded;
    CIOArray decoded;
    CIOArrayConstU32Init(&plain,str,0,strLen);
    CIOArrayU8Init(&encoded,NULL,0,0,0,INT_MAX);
    CIOArrayU32Init(&decoded,NULL,0,0,0,INT_MAX);    
    encodeIO(&plain.base,&encoded.base);

    int encLen = CIOGetWrites(&encoded);
    encoded.position=0;
    decodeIO(&encoded.base,&decoded.base);
    int decLen = CIOGetWrites(&decoded);
    decoded.position=0;
    int maxLen = strLen > decLen ? strLen : decLen;
    for (int j=0; j<=maxLen; ++j) {
      FACT(j < strLen ? str[j] : -1,==,CIOPeek(&decoded,j));
    }
    CIOClose(&plain);
    CIOClose(&encoded);
    CIOClose(&decoded);
    free((void*)str);
  }
}

FACTS(Envelope) {
  for (int i=0; i<sizeof(UTF8_TEST_STRINGS)/sizeof(char*); ++i) {
    const uint32_t *str=testString(i);    
    int strLen = wstrlen(str);

    CIOArray plain;
    CIOArrayConstU32Init(&plain,str,0,strLen);
    CIONotRand nrcg;
    CIONotRandInit(&nrcg);
    Deck deck;
    deckInit(deck);

    CIOArray envelope;
    CIOArrayU8Init(&envelope,NULL,0,0,0,INT_MAX);
    encryptEnvelopeIO(deck,&plain.base,&nrcg.base,&envelope.base);
    int envLen = CIOGetWrites(&envelope);

    // reset envelope for decryption
    envelope.position = 0;
    
    CIOArray decrypted;
    CIOArrayU32Init(&decrypted,NULL,0,0,0,INT_MAX);
    deckInit(deck);
    decryptEnvelopeIO(deck,&envelope.base,NULL,&decrypted.base);
    int decLen = CIOGetWrites(&decrypted);
    decrypted.position = 0;

    int maxLen = strLen > decLen ? strLen : decLen;
    for (int j=0; j<=maxLen; ++j) {
      FACT(j < strLen ? str[j] : -1,==,CIOPeek(&decrypted,j));
    }

    CIOClose(&decrypted);
    CIOClose(&envelope);
    CIOClose(&plain);
    CIOClose(&nrcg);

    free((void*)str);
  }
}

FACTS(BackFrontUnshuffle) {
  Deck deck;
  Deck tmp;
  Deck id;
  deckInit(deck);
  deckInit(tmp);
  deckInit(id);
  deckBackFrontShuffle(deck,tmp);
  for (int i=0; i<CARDS; ++i) deck[i]=0;
  deckBackFrontUnshuffle(tmp,deck);
  DECK_EQ(deck,id);
}

// pseudo-shuffle on cut location c
void P(Deck deck,int c) {
  Deck tmp;
  deckCut(deck,c,tmp);
  deckBackFrontShuffle(tmp,deck);
}

FACTS(P) {
  for (int c=0; c<CARDS; ++c) {
    Deck a;
    Deck b;
    deckInit(a);
    deckInit(b);
    deckPseudoShuffle(a,c);
    P(b,c);
    DECK_EQ(a,b);
  }
}

FACTS(PReachable) {
  for (int c=0; c<CARDS; ++c) {
    Deck p;
    deckInit(p);
    P(p,c);
    
    Deck reach;
    deckInit(reach);
    deckPseudoShuffle(reach,c);

    DECK_EQ(reach,p);
  }
}

void I(Deck deck) {
  //id;
}

FACTS(I) {
  Deck i;
  deckInit(i);
  I(i);
  for (int k=0; k<CARDS; ++k) FACT(i[k],==,k);
}

FACTS(IReachable) {
  Deck id;
  deckInit(id);
  I(id);

  Deck reach;
  deckInit(reach);
  for (int k=0; k<9; ++k) {
    P(reach,2);
  }

  DECK_EQ(reach,id);
}

// inverse pseudo-shuffle on cut location c
void Q(Deck deck,int c) {
  Deck tmp;
  deckBackFrontUnshuffle(deck,tmp);
  deckCut(tmp,CARDS-c,deck);
}

FACTS(Q) {
  for (int c=0; c<CARDS; ++c) {
    Deck deck;
    Deck id;
    deckInit(deck);
    deckInit(id);
    P(deck,c);
    Q(deck,c);
    DECK_EQ(deck,id);
  }
}

FACTS(QReachable) {
  for (int c=0; c<CARDS; ++c) {
    Deck q;
    deckInit(q);
    Q(q,c);
	  
    Deck reach;
    deckInit(reach);

    for (int i=0; i<17; ++i) {
      deckPseudoShuffle(reach,(i == 8) ? (4+(CARDS-c))%CARDS : 2);
    }
    DECK_EQ(reach,q);
  }
}

void T(Deck deck, int c) {
  Deck tmp;
  deckCut(deck,c,tmp);
  for (int i=0; i<CARDS; ++i) {
    deck[i]=tmp[i];
  }
}

FACTS(T) {
  for (int c=0; c<CARDS; ++c) {
    Deck t;
    deckInit(t);
    T(t,c);

    for (int i=0; i<CARDS; ++i) {
      t[i]=(i+c) % CARDS;
    }
  }
}

FACTS(TReachable) {
  for (int c=0; c<CARDS; ++c) {
    Deck t;
    deckInit(t);
    T(t,c);

    Deck reach;
    deckInit(reach);
    for (int i=0; i<9; ++i) {
      deckPseudoShuffle(reach,(i == 0) ? (2+c)%CARDS : 2);
    }
    
    DECK_EQ(reach,t);
  }
}

void R(Deck deck) { // reverse deck
  Deck tmp;
  for (int i=0; i<CARDS; ++i) {
    tmp[i]=deck[(CARDS-1)-i];
  }
  for (int i=0; i<CARDS; ++i) {
    deck[i]=tmp[i];
  }
}

FACTS(R) {
  Deck r;
  deckInit(r);
  R(r);
  for (int i=0; i<CARDS; ++i) {
    FACT(r[i],==,CARDS-1 - i);
  }
}


FACTS(RReachable) {
  Deck r;
  deckInit(r);
  R(r);

  Deck reach;
  deckInit(reach);
  P(reach,0);
  T(reach,20);
  Q(reach,0);

  DECK_EQ(reach,r);
}

void X(Deck deck) { // exchange pairs
  Deck tmp;
  for (int i=0; i<CARDS; i += 2) {
    int j=i+1;
    tmp[i]=deck[j];
    tmp[j]=deck[i];
  }
  for (int i=0; i<CARDS; ++i) {
    deck[i]=tmp[i];
  }
}

FACTS(X) {
  Deck x;
  deckInit(x);
  X(x);
  for (int i=0; i<CARDS; ++i) {
    FACT(x[i],==,(i % 2 == 0) ? i+1 : i-1);
  }
}

FACTS(XReachable) {
  Deck x;
  deckInit(x);
  X(x);

  Deck reach;
  deckInit(reach);

  P(reach,0);
  R(reach);
  Q(reach,0);

  DECK_EQ(reach,x);
}

void B(Deck deck, int i) { // exchange i with i+1
  int j = (i+1)%CARDS;
  int tmp = deck[i];
  deck[i]=deck[j];
  deck[j]=tmp;
}

FACTS(B) {
  for (int i=0; i<CARDS; ++i) {
    Deck b;
    deckInit(b);
    B(b,i);
    for (int k=0; k<CARDS; ++k) {
      int j=(i+1) % CARDS;
      if (k == i) FACT(b[k],==,j);
      if (k == j) FACT(b[k],==,i);
      if (k != i && k != j) FACT(b[k],==,k);
    }
  }
}

FACTS(BReachable) {
  for (int i=0; i<CARDS; ++i) {
    Deck b;
    deckInit(b);
    B(b,i);

    Deck reach;
    deckInit(reach);

    T(reach,(i+1)%CARDS);
    P(reach,0);
    T(reach,21);
    Q(reach,0);
    R(reach);
    X(reach);
    T(reach,1);
    X(reach);
    T(reach,CARDS-1);
    T(reach,(2*CARDS-(i+1)) % CARDS);
    
    DECK_EQ(reach,b);
  }
}

void S(Deck deck, int i, int j) { // swap i and j
  int tmp = deck[i];
  deck[i]=deck[j];
  deck[j]=tmp;
}

FACTS(S) {
  for (int i=0; i<CARDS; ++i) {
    for (int j=0; j<CARDS; ++j) {
      Deck s;
      deckInit(s);
      S(s,i,j);
      
      for (int k=0; k<CARDS; ++k) {
	if (k == i) FACT(s[k],==,j);
	if (k == j) FACT(s[k],==,i);
	if (k != i && k != j) FACT(s[k],==,k);
      }
    }
  }
}

FACTS(SReachable) {
  for (int i=0; i<CARDS; ++i) {
    for (int j=0; j<CARDS; ++j) {
      Deck s;
      deckInit(s);
      S(s,i,j);
      
      Deck reach;
      deckInit(reach);

      if (i != j) {
	int a = (i < j) ? i : j;
	int b = (i > j) ? i : j;

	T(reach,a);	
	for (int k=0; k<b-a; ++k) {
	  B(reach,k);
	}
	for (int k=b-a-2; k>=0; --k) {
	  B(reach,k);
	}
	T(reach,(CARDS-a));
      }
      DECK_EQ(reach,s);
    }
  }
}

static int CYCLE_LENGTHS [] =
  {
   27,  30,   9, 110,  12,  90,  99, 234, 105,  12,
   60, 126, 115,  56,  20, 174,  12,  66, 105,  20,
   39,  40,  39,  60,  72, 150,  60,  40,  39, 264,
   36, 380,  75,  20,  60,  40, 182, 190, 440,  24
  };


static int PERFECT_CYCLE_LENGTHS [] =
  {
  36, 465,  36, 176,  30,  39, 180, 105, 390,  48,
 330, 175, 140,  88,  30,  96,  60,  39,  36, 220,
  38, 279, 380,  84,1848, 420, 780,  84, 380, 264,
 132, 240,1400,  30,  60,  35, 308, 145,1848, 168
  };



int deckcmp(Deck a, Deck b) {
  int cmp = memcmp(a,b,CARDS);
  if (cmp < 0) return -1;
  if (cmp > 0) return  1;
  return 0;
}


FACTS(Cycles) {
  int ok = 1;
  
  Deck t[CARDS];
  for (int c = 0; c<CARDS; ++c) {
    deckInit(t[c]);
    T(t[c],c);
  }

  for (int perfect = 0; perfect < 2; ++perfect) {
    for (int c = 0; c < CARDS; ++c) {
      Deck deck;
      deckInit(deck);
      int i = 0,eq=-1;
      while (eq == -1) {
	P(deck,c);
	if (perfect) {
	  S(deck,0,19);
	}
	++i;
	for (int k=0; k<CARDS; ++k) {
	  if (deckcmp(deck,t[k])==0) {
	    eq = k;
	  }
	}
      }
      int length = perfect ? PERFECT_CYCLE_LENGTHS[c] : CYCLE_LENGTHS[c];

      if (perfect) {
	FACT(i,>=,30);
      }

      if (i != length) {
	ok = 0;
      }
      //      fprintf(stderr,"%4d%s",i, (c==CARDS-1 ? "" : ","));
      //      if (c % 10 == 9) { fprintf(stderr,"\n"); }
      //i != CARDS-1) fprintf(stderr
      //cycles %d = %d (not %d) eq=%d\n",
	//	      c,i,CYCLE_LENGTHS[c],eq);
    }
  }
  FACT(ok,==,1);
}


FACTS_EXCLUDE(TestCycles) {
  int ok = 1;
  
  Deck t[CARDS];
  for (int c = 0; c<CARDS; ++c) {
    deckInit(t[c]);
    T(t[c],c);
  }

  int maxLen=-1,maxS0=0,maxS1=0;

  for (int s0=0; s0<CARDS; ++s0) {
    for (int s1=s0+1; s1<CARDS; ++s1) {
      int minLenS = INT_MAX;
      for (int c = 0; c < CARDS; ++c) {
	Deck deck;
	deckInit(deck);
	int i = 0,eq=-1;
	while (eq == -1) {
	  P(deck,c);
	  S(deck,s0,s1);
	  //      int tmp=deck[0];
	  //      for (int k=0; k<CARDS/2-1; ++k) {
	  //	deck[k]=deck[k+1];
	  //      }
	  //      deck[CARDS/2-1]=tmp;
	  ++i;
	  for (int k=0; k<CARDS; ++k) {
	    if (deckcmp(deck,t[k])==0) {
	      eq = k;
	    }
	  }
	}
	if (i < minLenS) {
	  minLenS=i;
	}
      }
      if (minLenS > maxLen) {
	maxLen = minLenS;
	maxS0=s0;
	maxS1=s1;
      }
    }
  }
  fprintf(stderr,"max len = %d s0=%d s1=%d\n",maxLen,maxS0,maxS1);

    //    if (i != CYCLE_LENGTHS[c]) {
    //      fprintf(stderr,"cycles %d = %d (not %d) eq=%d\n",
    //	      c,i,CYCLE_LENGTHS[c],eq);
    //      ok = 0;
    //    }
  //  }
  //  FACT(ok,==,1);
}

FACTS(InverseCycles) {
  for (int c = 0; c < CARDS; ++c) {
    Deck deck;
    Deck id;
    deckInit(deck);
    deckInit(id);
    int n=CYCLE_LENGTHS[c];
    for (int i=0; i<n; ++i) {
      if (i != 0) { DECK_NE(deck,id); }
      else { DECK_EQ(deck,id); }
      Q(deck,c);
      if (i != n-1) { DECK_NE(deck,id); }
      else { DECK_EQ(deck,id); }
    }
  }
}

uint8_t PERMS1[1][1]=
  {
   {0}
  };


uint8_t PERMS2[2][2]=
  {
   {0,1},
   {1,0}
  };

uint8_t PERMS3[6][3]=
  {
   {0,1,2},{0,2,1},
   {1,0,2},{1,2,0},
   {2,0,1},{2,1,0}
  };

uint8_t PERMS4[24][4]=
  {
   {0,1,2,3},{0,1,3,2},{0,2,1,3},{0,2,3,1},{0,3,1,2},{0,3,2,1},
   {1,0,2,3},{1,0,3,2},{1,2,0,3},{1,2,3,0},{1,3,0,2},{1,3,2,0},
   {2,0,1,3},{2,0,3,1},{2,1,0,3},{2,1,3,0},{2,3,0,1},{2,3,1,0},
   {3,0,1,2},{3,0,2,1},{3,1,0,2},{3,1,2,0},{3,2,0,1},{3,2,1,0}
  };

// A set of (1*2*6*24)^4 distinct permutations
// to test the big deck set with.
// They are otherwise not important.

void Z(Deck deck, int64_t i) {
  Deck tmp;
  for (int j=0; j<4; ++j) {
    int i1 = 0;
    int i2 = i % 2;
    i = i/2;
    int i3 = i % 6;
    i = i/6;
    int i4 = i % 24;
    i = i/24;
    
    tmp[10*j+0+0]=deck[10*j+0+PERMS1[i1][0]];
    tmp[10*j+1+0]=deck[10*j+1+PERMS2[i2][0]];
    tmp[10*j+1+1]=deck[10*j+1+PERMS2[i2][1]];
    tmp[10*j+3+0]=deck[10*j+3+PERMS3[i3][0]];
    tmp[10*j+3+1]=deck[10*j+3+PERMS3[i3][1]];
    tmp[10*j+3+2]=deck[10*j+3+PERMS3[i3][2]];  
    tmp[10*j+6+0]=deck[10*j+6+PERMS4[i4][0]];
    tmp[10*j+6+1]=deck[10*j+6+PERMS4[i4][1]];
    tmp[10*j+6+2]=deck[10*j+6+PERMS4[i4][2]];
    tmp[10*j+6+3]=deck[10*j+6+PERMS4[i4][3]];
  }

  for (int k=0; k<CARDS; ++k) {
    deck[k]=tmp[k];
  }
}

FACTS(Z) {
  for (int64_t j0=0; j0<4; ++j0) {
    for (int64_t j1=0; j1<4; ++j1) {
      for (int64_t i0=0; i0<2*6*24; ++i0) {
	for (int64_t i1=0; i1<2*6*24; ++i1) {
	  int64_t k0 = i0+((j0 > 0) ? pow(2*6*24,j0) : 0);
	  int64_t k1 = i1+((j1 > 0) ? pow(2*6*24,j1) : 0);

	  if ((k0 == k1) != (i0 == i1 && j0 == j1)) {
	    printf("k0=%d, k1=%d i0=%d i1=%d j0=%d j1=%d\n",(int) k0,(int) k1,(int) i0,(int) i1,(int) j0,(int) j1);
	  }

	  Deck d0,d1;
	  deckInit(d0);
	  deckInit(d1);	    
	  Z(d0,k0);
	  Z(d1,k1);
	  FACT(memcmp(d0,d1,CARDS)!=0,==,k0!=k1);
	    
	  int b0[40],b1[40];
	  for (int i=0; i<CARDS; ++i) {
	    b0[i]=0;
	    b1[i]=0;
	  }
	  for (int i=0; i<CARDS; ++i) {
	    FACT(0,<=,d0[i]);
	    FACT(d0[i],<,CARDS);
	    FACT(0,<=,d1[i]);
	    FACT(d1[i],<,CARDS);
	    ++b0[d0[i]];
	    ++b1[d1[i]];
	  }
	  for (int i=0; i<CARDS; ++i) {
	    FACT(b0[i],==,1);
	    FACT(b1[i],==,1);
	  }
	}
      }
    }
  }  
}

typedef struct {
  int pbins;
  uint32_t nbins;
  uint32_t *counts;
  uint32_t *offsets;
  Card *cards;
  FILE *file;
} DeckSet;

void DeckSetInit(DeckSet *me, int pbins, FILE *file) {
  me->pbins = pbins;
  me->nbins = 1;
  for (int i=0; i<pbins; ++i) {
    me->nbins  *= CARDS;
  }

  me->counts = (uint32_t*)calloc(sizeof(uint32_t),me->nbins);
  me->offsets = (uint32_t*)calloc(sizeof(uint32_t),me->nbins);
  me->cards = NULL;
  me->file = file;
}

void DeckSetCount(DeckSet *me, Deck deck) {
  int k=0;
  for (int i=0; i<me->pbins; ++i) {
    k=CARDS*k+deck[i];
  }
  ++me->counts[k];
}

void DeckSetCounted(DeckSet *me) {
  for (int i=0; i<me->nbins; ++i) {
    me->offsets[i] = ((i > 0) ? me->offsets[i-1] : 0) + me->counts[i];
  }
  if (me->file == NULL) {
    me->cards = (Card*) calloc(me->offsets[me->nbins-1],CARDS);
    assert(me->cards != NULL);
  }
  memset(me->counts,0,me->nbins*sizeof(uint32_t));  
}

void DeckSetSave(DeckSet *me) {
  if (me->file == NULL) return;
  
  int seekOk = fseek(me->file,0L,SEEK_SET);
  assert(seekOk==0);
  for (int i=0; i<me->nbins; ++i) {
    me->counts[i]=htonl(me->counts[i]);
  }
  for (int i=0; i<me->nbins; ++i) {
    me->offsets[i]=htonl(me->offsets[i]);
  }
  int writeOk = fwrite(me->counts,sizeof(uint32_t),me->nbins,me->file);
  assert(writeOk==me->nbins);
  writeOk = fwrite(me->offsets,sizeof(uint32_t),me->nbins,me->file);
  assert(writeOk==me->nbins);
  for (int i=0; i<me->nbins; ++i) {
    me->counts[i]=ntohl(me->counts[i]);
  }
  for (int i=0; i<me->nbins; ++i) {
    me->offsets[i]=ntohl(me->offsets[i]);
  }
}

void DeckSetLoad(DeckSet *me) {
  if (me->file == NULL) return;
  int seekOk = fseek(me->file,0L,SEEK_SET);
  assert(seekOk==0);
  int readOk = fread(me->counts,sizeof(uint32_t),me->nbins,me->file);
  assert(readOk==me->nbins);
  readOk = fread(me->offsets,sizeof(uint32_t),me->nbins,me->file);
  assert(readOk==me->nbins);

  for (int i=0; i<me->nbins; ++i) {
    me->counts[i]=ntohl(me->counts[i]);
  }
  for (int i=0; i<me->nbins; ++i) {
    me->offsets[i]=ntohl(me->offsets[i]);
  }
}

void DeckSetAdd(DeckSet *me, Deck deck) {
  int k=0;
  for (int i=0; i<me->pbins; ++i) {
    k=CARDS*k+deck[i];
  }
  uint64_t offset = ((k > 0) ? me->offsets[k-1] : 0)+me->counts[k];
  if (me->file == NULL) {
    memcpy(me->cards+CARDS*offset,deck,CARDS);
  } else {
    offset = CARDS*offset + me->nbins*sizeof(uint32_t);
    int seekOk = fseek(me->file,offset,SEEK_SET);
    assert(seekOk==0);
    int writeOk = fwrite(deck,CARDS,1,me->file);
    assert(writeOk==1);
  }
  ++me->counts[k];
}

int deckComp(Card *a, Card *b) {
  return memcmp(a,b,CARDS);
}

uint32_t unique(uint32_t n, Card *cards) {
  uint32_t i=0,j=1;
  while (j < n) {
    if (memcmp(cards+i*CARDS,cards+j*CARDS,CARDS)==0) {
      ++j;
    } else {
      ++i;
      if (i != j) {
	memcpy(cards+i*CARDS,cards+j*CARDS,CARDS);
      }
      ++j;
    }
  }
  return i+1;
}

uint32_t DeckSetSort(DeckSet *me) {
  uint32_t dups  = 0;
  int maxCount = 0;
  for (int k=0; k<me->nbins; ++k) {
    if (me->counts[k] > maxCount) maxCount = me->counts[k];
  }

  if (maxCount < 2) {
    return dups;
  }

  Card *cards = NULL;
  if (me->file != NULL) {
    cards = (Card*) malloc(CARDS*maxCount);
    assert(cards != NULL);
  }
  
  for (int k=0; k<me->nbins; ++k) {
    if (me->counts[k] < 2) continue;
    uint64_t offset = ((k > 0) ? me->offsets[k-1] : 0);
    if (me->file == NULL) {
      qsort(me->cards+CARDS*offset,me->counts[k],CARDS,
	    (int (*)(const void *, const void *))deckComp);
      uint32_t count=unique(me->counts[k],me->cards+CARDS*offset);
      dups += (me->counts[k]-count);
      me->counts[k]=count;
    } else {
      offset = CARDS*offset + me->nbins*sizeof(uint32_t);
      int seekOk = fseek(me->file,offset,SEEK_SET);
      assert(seekOk==0);
      int readOk = fread(cards,CARDS,me->counts[k],me->file);
      assert(readOk==me->counts[k]);
      qsort(cards,me->counts[k],CARDS,
	    (int (*)(const void *, const void *))deckComp);
      uint32_t count=unique(me->counts[k],cards);      
      dups += (me->counts[k]-count);
      me->counts[k]=count;
      seekOk = fseek(me->file,offset,SEEK_SET);
      assert(seekOk==0);    
      int writeOk = fwrite(cards,CARDS,me->counts[k],me->file);
      assert(writeOk==me->counts[k]);
    }
  }
  
  free(cards);

  return dups;
  
}

void DeckSetClose(DeckSet *me) {
  DeckSetSave(me);
  free(me->cards);
  free(me->counts);
  free(me->offsets);
}


int DeckSetContains(DeckSet *me, Deck deck) {
  int k=0;
  for (int i=0; i<me->pbins; ++i) {
    k = CARDS*k + deck[i];
  }
  if (me->counts[k] == 0) return 0;

  int64_t lo = ((k > 0) ? ((int64_t)me->offsets[k-1]) : ((int64_t)0))  - 1;
  int64_t hi = lo + me->counts[k] + 1;

  while (hi-lo >= 2) {
    int64_t mid = (lo+hi)/2;
    Deck tmp;
    uint64_t offset = mid;
    int cmp = 0;
    if (me->file == NULL) {
      cmp=deckComp(deck,me->cards+offset*CARDS);
    } else {
      offset = CARDS*offset + me->nbins*sizeof(uint32_t);
      int seekOk = fseek(me->file,offset,SEEK_SET);
      assert(seekOk==0);
      int readOk = fread(tmp,CARDS,1,me->file);
      assert(readOk==1);
      cmp = deckComp(deck,tmp);
    }
    if (cmp == 0) return 1;
    if (cmp < 0) {
      hi = mid;
    } else {
      lo = mid;
    }
  }
  return 0;
}

FACTS(DeckSet) {
  for (int tmp = 0; tmp<2; ++tmp) {
    for (int pbins = 1; pbins < 4; ++pbins) {
      DeckSet *ds = (DeckSet*) malloc(sizeof(DeckSet));
      int dups = 0;
      assert (ds != NULL);
      FILE *file = NULL;
      if (tmp) {
	file=tmpfile();
	assert(file != NULL);
      }
      DeckSetInit(ds,pbins,file);

      for (int64_t j=0; j<1; ++j) {
	for (int64_t i=0; i<2*6*24; ++i) {
	  int64_t k = i+((j>0) ? pow(2*6*24,j) : 0);

	  if (k % 17 == 0) continue;
	  Deck deck;

	  deckInit(deck);
	  Z(deck,k);
	  DeckSetCount(ds,deck);
	  if (k % 19 == 0 && j == 0 && i < 100) {
	    DeckSetCount(ds,deck);
	    ++dups;
	  }
	}
      }

      DeckSetCounted(ds);

      for (int64_t j=0; j<1; ++j) {
	for (int64_t i=2*6*24-1; i>=0; --i) {
	  int64_t k = i+((j>0) ? pow(2*6*24,j) : 0);
	  if (k % 17 == 0) continue;
	  Deck deck;
	  deckInit(deck);
	  Z(deck,k);
	  DeckSetAdd(ds,deck);
	  if (k % 19 == 0 && j == 0 && i < 100) {
	    DeckSetAdd(ds,deck);
	  }
	}
      }

      int dups2=DeckSetSort(ds);

      for (int64_t j=0; j<1; ++j) {
	for (int64_t i=0; i<2*6*24; ++i) {
	  int64_t k = i+((j>0) ? pow(2*6*24,j) : 0);
	  Deck deck;
	  deckInit(deck);
	  Z(deck,k);
	  int ans = DeckSetContains(ds,deck);
	  FACT(ans,==,k % 17 != 0);
	}
      }

      FACT(dups,==,dups2);

      DeckSetClose(ds);
      if (file != NULL) {
	fclose(file);
      }
    }
  }
}

void Neighbors(DeckSet *ds,Deck deck, int perfect, int dir, int dist, int count, double *progress, double done) {
  if (dist > 0) {
    Deck tmp,next;
    for (int c=0; c<CARDS; ++c) {
      if (dir == 1) {
	deckCut(deck,c,tmp);
	deckBackFrontShuffle(tmp,next);
	if (perfect) {
	  int save=next[19];
	  next[19]=next[0];
	  next[0]=save;
	}
      } else {
	for (int d=0; d<CARDS; ++d) {
	  next[d]=deck[d];
	}
	if (perfect) {
	  int save=next[19];
	  next[19]=next[0];
	  next[0]=save;
	}
        deckBackFrontUnshuffle(next,tmp);
	deckCut(tmp,CARDS-c,next);
      }

      if (progress != NULL) {
	if (floor((*progress*20)/done) != floor(((*progress+1)*20)/done)) {
	  fprintf(stderr,"%0.1f done.\n",((*progress)*100)/done);
	}
	*progress += 1.0;
      }
      if (count) {
	DeckSetCount(ds,next);
      } else {
	DeckSetAdd(ds,next);
      }
      Neighbors(ds,next,perfect,dir,dist-1,count,progress,done);
    }
  }
}

double timer() {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME,&ts);
  return ts.tv_sec + ts.tv_nsec*1e-9;
}

void datetime(double timer) {
  struct timespec ts;
  ts.tv_sec = timer;
  ts.tv_nsec = (timer-ts.tv_sec)*1e9;
  char buff[100];
  strftime(buff,sizeof(buff),"%D %T",gmtime(&ts.tv_sec));
  fprintf(stderr,"%s.%09ld UTC\n",buff,ts.tv_nsec);
}

int dups(int perfect, int dir, int dist) {
  Deck deck;
  deckInit(deck);

  FILE *file = dist > 5 ? tmpfile() : NULL;
  int pbins = dist > 5 ? 5 : 4;

  DeckSet *ds = (DeckSet*) malloc(sizeof(DeckSet));
  DeckSetInit(ds,pbins,file);

  int count = 1;
  double progress = 0;
  double done = (pow((double)CARDS,(double)(dist+1))-1)/(CARDS-1);

  fprintf(stderr,"%f steps.\n",done);
  fprintf(stderr,"counting deck bins in neighborhood.\n");
  double t0=timer();
  datetime(t0);
  Neighbors(ds,deck,perfect,dir,dist,count,&progress,done);
  DeckSetCounted(ds);
  double t1=timer();
  datetime(t1);
  fprintf(stderr,"counting took %f seconds\n",t1-t0);
  
  count = 0;
  progress = 0;
  fprintf(stderr,"adding decks to neighborhood.\n");
  Neighbors(ds,deck,perfect,dir,dist,count,&progress,done);
  double t2=timer();
  datetime(t2);
  fprintf(stderr,"adding took %f seconds\n",t2-t1);

  double est = (t2-t1)*(log(done/ds->nbins)/(log(2)*ds->pbins));
  fprintf(stderr,"sorting time estimate is %f seconds\n",est);  
  int dups = DeckSetSort(ds);
  DeckSetClose(ds);
  fclose(file);
  free(ds);

  double t3=timer();
  fprintf(stderr,"sorting took %f seconds\n",t3-t2);  
  datetime(t3);
  return dups;
}

FACTS(Neighborhood5) {
  int perfect = 0;
  int n = 5;
  int collisions = dups(perfect,1,n);
  FACT(collisions,==,0);
}

FACTS(PerfectNeighborhood5) {
  int perfect = 1;
  int dir = 1;
  int dist = 5;
  int collisions = dups(perfect,dir,dist);
  FACT(collisions,==,0);
}

// About 200GB disk space, 10GB RAM, and a WEEK of runtime...
FACTS_EXCLUDE(Neighborhood6) {
  int perfect = 0;
  int dir = 1;
  int dist = 6;
  int collisions = dups(perfect,dir,dist);
  FACT(collisions,==,0);
}

// About 200GB disk space, 10GB RAM, and a WEEK of runtime...
FACTS_EXCLUDE(PerfectNeighborhood6) {
  int perfect = 1;
  int dir = 1;
  int dist = 6;
  int collisions = dups(perfect,dir,dist);
  FACT(collisions,==,0);
}

FACTS_REGISTER_ALL() {
    FACTS_REGISTER(RandStats);
    FACTS_REGISTER(FaceAndSuitNo);
    FACTS_REGISTER(FaceFromNo);
    FACTS_REGISTER(SuitFromNo);
    FACTS_REGISTER(CardFromFaceSuitNo);
    FACTS_REGISTER(Add);
    FACTS_REGISTER(Subtract);
    FACTS_REGISTER(Init);
    FACTS_REGISTER(Cut);
    FACTS_REGISTER(BackFrontShuffle);
    FACTS_REGISTER(FindCard);
    FACTS_REGISTER(PseudoShuffle);
    FACTS_REGISTER(Pads);
    FACTS_REGISTER(Ciphers);
    FACTS_REGISTER(Encode);
    FACTS_REGISTER(Envelope);
    FACTS_REGISTER(BackFrontUnshuffle);
    FACTS_REGISTER(P);
    FACTS_REGISTER(PReachable);
    FACTS_REGISTER(I);
    FACTS_REGISTER(IReachable);
    FACTS_REGISTER(Q);
    FACTS_REGISTER(QReachable);
    FACTS_REGISTER(T);
    FACTS_REGISTER(TReachable);
    FACTS_REGISTER(R);
    FACTS_REGISTER(RReachable);
    FACTS_REGISTER(X);
    FACTS_REGISTER(XReachable);
    FACTS_REGISTER(B);
    FACTS_REGISTER(BReachable);
    FACTS_REGISTER(S);
    FACTS_REGISTER(SReachable);
    FACTS_REGISTER(Cycles);
    FACTS_REGISTER(TestCycles);
    FACTS_REGISTER(InverseCycles);
    FACTS_REGISTER(Z);
    FACTS_REGISTER(DeckSet);
    FACTS_REGISTER(Neighborhood5);
    FACTS_REGISTER(PerfectNeighborhood5);
    FACTS_REGISTER(Neighborhood6);
    FACTS_REGISTER(PerfectNeighborhood6);
}

FACTS_MAIN

