#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include "facts.h"
#include "spider_solitaire.h"

#define PBINS 5
#define NBINS (CARDS*CARDS*CARDS*CARDS*CARDS)

typedef struct {
  uint32_t counts[NBINS];  
  uint32_t offsets[NBINS];
  FILE *file;
} DeckSet;

void DeckSetInit(DeckSet *me, FILE *file) {
  memset(me->offsets,0,NBINS*sizeof(uint32_t));
  memset(me->counts,0,NBINS*sizeof(uint32_t));  
  me->file = file;
}

void DeckSetCount(DeckSet *me, Deck deck) {
  int k=0;
  for (int i=0; i<PBINS; ++i) {
    k=CARDS*k+deck[i];
  }
  ++me->counts[k];
}

void DeckSetOffsets(DeckSet *me) {
  for (int i=0; i<NBINS; ++i) {
    me->offsets[i] = ((i > 0) ? me->offsets[i-1] : 0) + me->counts[i];
  }
}

void DeckSetResetCounts(DeckSet *me) {
  memset(me->counts,0,NBINS*sizeof(uint32_t));
}

void DeckSetSave(DeckSet *me) {
  int seekOk = fseek(me->file,0L,SEEK_SET);
  assert(seekOk==0);
  int writeOk = fwrite(me->offsets,sizeof(uint32_t),NBINS,me->file);
  assert(writeOk==NBINS);
}

void DeckSetLoad(DeckSet *me) {
  int seekOk = fseek(me->file,0L,SEEK_SET);
  assert(seekOk==0);
  int readOk = fread(me->offsets,sizeof(uint32_t),NBINS,me->file);
  assert(readOk==NBINS);
  for (int i=0; i<NBINS; ++i) {
    me->counts[i]=me->offsets[i]-((i > 0) ? me->offsets[i-1] : 0);
  }
}

void DeckSetAdd(DeckSet *me, Deck deck) {
  int k=0;
  for (int i=0; i<PBINS; ++i) {
    k=CARDS*k+deck[i];
  }
  uint64_t offset = ((k > 0) ? me->offsets[k-1] : 0)+me->counts[k];
  offset = CARDS*offset + NBINS*sizeof(uint32_t);
  int seekOk = fseek(me->file,offset,SEEK_SET);
  assert(seekOk==0);
  int writeOk = fwrite(deck,CARDS,1,me->file);
  assert(writeOk==1);
  ++me->counts[k];
}

int deckComp(Card *a, Card *b) {
  return memcmp(a,b,CARDS);
}

void DeckSetSort(DeckSet *me) {
  int maxCount = 0;
  for (int k=0; k<NBINS; ++k) {
    if (me->counts[k] > maxCount) maxCount = me->counts[k];
  }

  if (maxCount < 2) {
    return;
  }

  Card *cards = (Card*) malloc(CARDS*maxCount);
  assert(cards != NULL);
  
  for (int k=0; k<NBINS; ++k) {
    if (me->counts[k] < 2) continue;
    uint64_t offset = ((k > 0) ? me->offsets[k-1] : 0);
    offset = CARDS*offset + NBINS*sizeof(uint32_t);
    int seekOk = fseek(me->file,offset,SEEK_SET);
    assert(seekOk==0);
    int readOk = fread(cards,CARDS,me->counts[k],me->file);
    assert(readOk==me->counts[k]);
    qsort(cards,me->counts[k],CARDS,
	  (int (*)(const void *, const void *))&deckComp);
    seekOk = fseek(me->file,offset,SEEK_SET);
    assert(seekOk==0);    
    int writeOk = fwrite(cards,CARDS,me->counts[k],me->file);
    assert(writeOk==me->counts[k]);
  }

  free(cards);
}

void DeckSetClose(DeckSet *me) {
  DeckSetSave(me);
  fclose(me->file);
}


int DeckSetContains(DeckSet *me, Deck deck) {
  int k=0;
  for (int i=0; i<PBINS; ++i) {
    k = CARDS*k + deck[i];
  }
  if (me->counts[k] == 0) return 0;

  int64_t lo = ((k > 0) ? ((int64_t)me->offsets[k-1]) : ((int64_t)0))  - 1;
  int64_t hi = me->offsets[k];

  while (hi-lo > 2) {
    int64_t mid = (lo+hi)/2;
    Deck tmp;
    uint64_t offset = mid;
    offset = CARDS*offset + NBINS*sizeof(uint32_t);
    int seekOk = fseek(me->file,offset,SEEK_SET);
    assert(seekOk==0);
    int readOk = fread(tmp,CARDS,1,me->file);
    assert(readOk==1);
    int cmp = deckComp(deck,tmp);
    if (cmp == 0) return 1;
    if (cmp < 0) {
      lo = mid;
    } else {
      hi = mid;
    }
  }
  return 0;
}


uint8_t PERMS0[0][0];

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

// a set of (1*2*6*24)^4 distinct permutations
// test the big deck set with. (they are otherwise not important)

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

FACTS(ZSectioned) {
  for (int64_t j0=0; j0<4; ++j0) {
    for (int64_t j1=0; j1<4; ++j1) {
	for (int64_t i0=0; i0<2*6*24; ++i0) {
	  for (int64_t i1=0; i1<2*6*24; ++i1) {
	    int64_t k0 = i0+pow(2*6*24,j0);
	    int64_t k1 = i1+pow(2*6*24,j1);
	    Deck d0,d1;
	    deckInit(d0);
	    deckInit(d1);
	    Z(d0,k0);
	    Z(d1,k1);
	    FACT(deckComp(d0,d1)!=0,==,k0 != k1);
	}
      }
    }
  }
}


FACTS(ZRandom) {
  srand(1);
  int nt=10000;
  for (int t = 0; t<nt; ++t) {
    int64_t i0=0,i1 = 0;
    for (int j=0; j<4; ++j) {
      i0 = (2*6*24)*i0 + rand() % (2*6*24);
      i1 = (2*6*24)*i1 + rand() % (2*6*24);      
    }
    
    Deck d0,d1;
    int b0[40],b1[40];
    deckInit(d0);
    deckInit(d1);
    Z(d0,i0);
    Z(d1,i1);
    for (int j=0; j<CARDS; ++j) {
      b0[j]=0;
      b1[j]=0;      
    }
    for (int j=0; j<CARDS; ++j) {
      FACT(0,<=,d0[j]);
      FACT(d0[j],<,CARDS);
      FACT(0,<=,d1[j]);
      FACT(d1[j],<,CARDS);
      ++b0[d0[j]];
      ++b1[d1[j]];
    }
    int eq = 1;
    
    for (int j=0; j<CARDS; ++j) {
      FACT(b0[j],==,1);
      FACT(b1[j],==,1);
    }

    FACT(deckComp(d0,d1)==0,==,(i0==i1));
  }
}

FACTS(DeckSet) {
  DeckSet *ds = (DeckSet*) malloc(sizeof(DeckSet));
  assert (ds != NULL);
  FILE *file = tmpfile();
  DeckSetInit(ds,file);

  int64_t n = 2*6*24;
  n = n*n*n*n;

  for (int64_t i=n-1; i >= 0; i -= 997) {
    Deck deck;
    deckInit(deck);
    Z(deck,i);
    DeckSetCount(ds,deck);
  }

  DeckSetOffsets(ds);

  for (int64_t i=n-1; i >= 0; i -= 997) {
    Deck deck;
    deckInit(deck);
    Z(deck,i);
    DeckSetAdd(ds,deck);
  }

  DeckSetSort(ds);
  DeckSetSave(ds);
  free(ds);


  DeckSet *ds2 = (DeckSet*) malloc(sizeof(DeckSet));
  assert (ds2 != NULL);  
  DeckSetInit(ds2,file);
  DeckSetLoad(ds2);

  for (int64_t i=n-1; i >= 0; i -= 997) {
    Deck deck;
    deckInit(deck);
    Z(deck,i);
    int ans = DeckSetContains(ds2,deck);
    FACT(ans,==,1);
    if (i < n-1) {
      Deck ip1;
      deckInit(ip1);
      Z(ip1,i+1);
      int ansip1 = DeckSetContains(ds2,ip1);
      FACT(ansip1,==,0);
    }
    if (i > 0) {
      Deck im1;
      deckInit(im1);
      Z(im1,i-1);
      int ansip1 = DeckSetContains(ds2,im1);
      FACT(ansip1,==,0);
    }
  }
  DeckSetClose(ds2);
  free(ds2);
}

FACTS_FINISHED
FACTS_MAIN
