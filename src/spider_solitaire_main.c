#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>


#include "spider_solitaire.h"
#include "utf8.h"
#include "cio.h"

FILE *fmemopen(void*,int,const char *);

const char *base58="123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";



int CIOWriteUtf8(CIO *cio, const char *utf8, int utf8len) {
  while (utf8len > 0) {
    int declen = utf8declen(utf8,utf8len);
    if (declen > 0) {
      int status = CIOWrite(cio,utf8decval(utf8,declen));
      if (status == -1) {
	return -1;
      }
    } else {
      return -2;
    }
    utf8 += declen;
    utf8len -= declen;
  }
  return 0;
}

void CIOArraySetUtf8(CIOArray *array, const char *utf8) {
  array->position = 0;
  array->size = 0;
  int status = CIOWriteUtf8(&array->base,utf8,strlen(utf8));
  if (status != 0) {
    fprintf(stderr,"invalid utf8 (%s) or out of memory.",utf8);
    exit(1);
  }
  array->position=0;
}

struct CIOUTF8Struct;
typedef struct CIOUTF8Struct CIOUTF8;
struct CIOUTF8Struct {
  CIO base;
  CIOArray buffer;
  wchar_t buffer0[4];
  FILE *io;
};

int CIOUTF8Peek(CIOUTF8 *me, int offset) {
  unsigned char buf[6];
  while (me->buffer.size <= offset) {
    int decoded = 0;
    for (int len = 1; len<=4; ++len) {
      int c = fgetc(me->io);
      if (c == -1) {
	break;
      }
      buf[len-1]=c;
      int declen = utf8declen(buf,len);
      if (declen > 0) {
	CIOWrite(&me->buffer,utf8decval(buf,declen));
	decoded = 1;
	break;
      }
    }
    *(uint32_t*)buf=0;    
    if (!decoded) {
      return -1;
    }
  }
  return ((wchar_t*)me->buffer.data)[offset];
}

int CIOUTF8Read(CIOUTF8 *me) {
  int ans = CIOUTF8Peek(me,0);
  if (ans != -1) {
    wchar_t *data = (wchar_t*)me->buffer.data;
    int size = me->buffer.size;
    for (int i = 1; i<size; ++i) {
      data[i-1]=data[i];
    }
    --me->buffer.size;
    --me->buffer.position;
  }
  return ans;
}

int CIOUTF8Write(CIOUTF8 *me, int wc) {
  char buf[8];
  int enclen=utf8enclen(wc);
  utf8encval(buf,wc,enclen);
  buf[enclen]=0;
  if (fwrite(buf,1,enclen,me->io) != enclen) {
    return -1;
  }
  return 0;
}

int CIOUTF8Close(CIOUTF8 *me) {
  CIOClose(&me->buffer);
}

void CIOUTF8Init(CIOUTF8 *me, FILE *io) {
  CIOInit(&me->base);
  CIOArrayWideCharInit(&me->buffer,me->buffer0,0,0,sizeof(me->buffer0)/sizeof(wchar_t),INT_MAX);
  me->base.read = (CIOReadPtr)&CIOUTF8Read;
  me->base.peek = (CIOPeekPtr)&CIOUTF8Peek;  
  me->base.write = (CIOWritePtr)&CIOUTF8Write;
  me->base.close = (CIOClosePtr)&CIOUTF8Close;
  me->io=io;
}

void DeckKey(Deck deck, const char *key) {

  FILE *in = fmemopen((void*)key,strlen(key), "r");  
  CIOUTF8 codeIn;
  CIOUTF8Init(&codeIn,in);
  
  CIO null;
  CIOInit(&null);

  CIOTranslate trans;
  deckInit(deck);
  CIOTranslateInit(&trans,&null,deck,1);
  encodeIO(&codeIn.base,&trans.base);
  CIOClose(&trans);
  CIOClose(&null);  
  CIOClose(&codeIn);
}

struct CIOCardsFmtStruct;
typedef struct CIOCardsFmtStruct CIOCardsFmt;
struct CIOCardsFmtStruct {
  CIO base;
  CIOUTF8 utf8;
  int mode;
};

int CIOCardsFmtWrite(CIOCardsFmt *me, int card) {
  if (card < 0 || card >= CARDS) {
    return -1;
  }
  
  int ws[4];

  if (me->mode == '#') {
    ws[0]='0'+(card/10);
    ws[1]='0'+(card%10);
    ws[2]=0;
  } else if (me->mode == 'b') {
    ws[0]=base58[card];
    ws[1]=0;
  } else if (me->mode == 'c') {
    ws[0]=cardFaceFromNo(cardFaceNo(card));
    ws[1]=cardSuiteFromNo(cardSuiteNo(card));
    ws[2]=0;
  } else {
    return -1;
  }

  for (int i=0; ws[i] != 0; ++i) {
    if (CIOWrite(&me->utf8,ws[i]) == -1) return -1;
  }
  return 0;
}

static const char *WS=" \t\n\r.,-_()[]";

int CIOCardsFmtRead(CIOCardsFmt *me, int card) {
  for (;;) {
    int c = CIOPeek(&me->utf8,0);
    if (strchr(WS,c) == NULL) break;
    CIORead(&me->utf8);
  }
  if (me->mode == '#') {
    int c1=CIOPeek(&me->utf8,0)-'0';
    if (0 <= c1 && c1 < 4) {
      CIORead(&me->utf8);
      int c0=CIOPeek(&me->utf8,0)-'0';
      if (0 <= c0 && c0 < 10) {
	CIORead(&me->utf8);	
	int card = c1*10+c0;
	return card;
      }
    }
  } else if (me->mode == 'b') {
    int b = CIOPeek(&me->utf8,0);
    const char *p=strchr(base58,b);
    if (p != NULL) {
      CIORead(&me->utf8);      
      card = p-base58;
      return card;
    }
  } else if (me->mode == 'c') {
    int face = CIOPeek(&me->utf8,0);
    int faceNo=cardFaceNoFromFace(face);
    if (faceNo != -1) {
      CIORead(&me->utf8);
      int suite = CIOPeek(&me->utf8,0);
      int suiteNo=cardSuiteNoFromSuite(suite);
      if (suiteNo != -1) {
	CIORead(&me->utf8);	
	return cardFromFaceSuiteNo(faceNo,suiteNo);
      }
    }
  }
  return -1;
}

void CIOCardsFmtInit(CIOCardsFmt *me,int mode, FILE *io) {
  CIOInit(&me->base);
  CIOUTF8Init(&me->utf8,io);
  me->base.read = (CIOReadPtr) &CIOCardsFmtRead;
  me->base.write = (CIOWritePtr) &CIOCardsFmtWrite;
  me->mode=mode;
}

int main(int argc, char *argv[]) {
  int mode = '#';
  const char *key = NULL;
  for (int argi=1; argi<argc; ++argi) {
    const char *arg=argv[argi];
    
    {
      const char *op="--key=";
      int oplen = strlen(op);
      if (strncmp(arg,op,oplen)==0) {
	key = arg+oplen;
	continue;
      }
    }

    {
      const char *op="--decimal";
      if (strcmp(arg,op)==0) {
	mode = '#';
	continue;
      }
    }
    
    {
      const char *op="--base58";
      if (strcmp(arg,op)==0) {
	mode = 'b';
	continue;
      }
    }

    {
      const char *op="--cards";
      if (strcmp(arg,op)==0) {
	mode = 'c';
	continue;
      }
    }
    
    {
      const char *op="--encrypt=";
      int oplen = strlen(op);      
      if (strncmp(arg,op,oplen)==0) {
	const char *plain=arg+oplen;
	Deck deck;
	DeckKey(deck,key);

	FILE *in=NULL;
	if (strcmp(plain,"-") != 0) {
	  in = fmemopen((char*)plain, strlen(plain), "r");
	} else {
	  in = stdin;
	}

	CIOUTF8 codeIn;
	CIOUTF8Init(&codeIn,in);
	
	CIORand rcg;
	CIORandInit(&rcg);

	CIOCardsFmt cardOut;
	CIOCardsFmtInit(&cardOut,mode,stdout);
	encryptEnvelopeIO(deck,&codeIn.base, &rcg.base, &cardOut.base);

	CIOClose(&cardOut);
	CIOClose(&rcg);
	CIOClose(&codeIn);
	deckInit(deck);
	continue;
      }
    }

    {
      const char *op="--decrypt=";
      int oplen = strlen(op);      
      if (strncmp(arg,op,oplen)==0) {
	const char *cipher=arg+oplen;
	Deck deck;
	DeckKey(deck,key);

	FILE *in=NULL;
	if (strcmp(cipher,"-") != 0) {
	  in = fmemopen((char*)cipher, strlen(cipher), "r");
	} else {
	  in = stdin;
	}
	
	CIOCardsFmt cardIn;
	CIOCardsFmtInit(&cardIn,mode,in);

	CIOUTF8 codeOut;
	CIOUTF8Init(&codeOut,stdout);
	decryptEnvelopeIO(deck,&cardIn.base, NULL, &codeOut.base);
	CIOClose(&codeOut);
	CIOClose(&cardIn);
	deckInit(deck);
	continue;
      }
    }
  }
  return 0;
}
