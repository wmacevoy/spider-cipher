#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

FILE *fmemopen(void*,int,const char *);

#include "spider_solitaire.h"
#include "utf8.h"
#include "cio.h"

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
  FILE *io;
};

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

void CIOUTF8Init(CIOUTF8 *me, FILE *io) {
  CIOInit(&me->base);
  me->base.write = (CIOWritePtr)&CIOUTF8Write;
  me->io=io;
}

void DeckKey(Deck deck, const char *utf8) {
  CIOArray key;
  CIOArrayWideCharInit(&key,NULL,0,0,0,INT_MAX);	
  CIOArraySetUtf8(&key,utf8);

  CIO null;
  CIOInit(&null);

  CIOTranslate trans;
  deckInit(deck);
  CIOTranslateInit(&trans,&null,deck,1);
  encodeIO(&key.base,&trans.base);
  CIOClose(&trans);
  CIOClose(&null);  
  CIOClose(&key);
}

struct CIOCardsFmtStruct;
typedef struct CIOCardsFmtStruct CIOCardsFmt;
struct CIOCardsFmtStruct {
  CIO base;
  int mode;
  FILE *io;
};

const char *base58="123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

int CIOCardsFmtWrite(CIOCardsFmt *me, int card) {
  if (card < 0 || card >= CARDS) {
    fprintf(me->io,"?(%d)",card);
  }

  if (me->mode == '#') {
    fprintf(me->io,"%02d",card);
  } else if (me->mode == 'b') {
    fprintf(me->io,"%c",base58[card]);
  } else if (me->mode == 'c') {
  
    int faceNo = cardFaceNo(card);
    int suiteNo = cardSuiteNo(card);

    wchar_t face = cardFaceFromNo(faceNo);
    wchar_t suite = cardSuiteFromNo(suiteNo);

    fprintf(me->io,"%02d",card);
    wchar_t ws[2]={face,suite};
    unsigned char buf[8];
    for (int i=0; i<2; ++i) {
      int enclen = utf8enclen(ws[i]);
      utf8encval(buf,ws[i],enclen);
      buf[enclen]=0;
      if (fwrite(buf,1,enclen,me->io) != enclen) {
	fprintf(stderr,"write failed.");
	exit(1);
      }
    }
  }

  int w = CIOGetWrites(me)+1;

  if (w % 5 == 0) {
    if (w % 10 == 0) {
      fprintf(me->io,"\n");
    } else {
      fprintf(me->io,"-");
    }
  } else {
    fprintf(me->io," ");
  }
  return 0;
}

int CIOCardsFmtRead(CIOCardsFmt *me, int card) {
  for (;;) {
    int c = fgetc(me->io);
    if (c != ' ' && c != '-' && c != '\r' && c != '\n') { ungetc(c,me->io); break; }
  }
  if (me->mode == '#') {
    int card = 0;
    if (fscanf(me->io,"%02d",&card) == 1) {
      return card;
    } else {
      return -1;
    }
  } else if (me->mode == 'b') {
    int card = 0;
    char b;
    if (fscanf(me->io,"%c",&b) == 1) {
      const char *p=strchr(base58,b);
      if (p != NULL) {
	card = p-base58;
	return card;
      }
    }
    return -1;
  } else if (me->mode == 'c') {
    wchar_t fs[2];
    
    for (int i=0; i<2; ++i) {
      unsigned char buf[8];
      int decoded = 0;
      for (int len = 1; len<=4; ++len) {
	int c = fgetc(me->io);
	if (c == -1) {
	  return -1;
	}
	buf[len-1]=c;
	int declen = utf8declen(buf,len);
	if (declen > 0) {
	  fs[i]=utf8decval(buf,declen);
	  decoded = 1;
	  break;
	}
      }
      if (!decoded) {
	return -1;
      }
    }

    int faceNo=cardFaceNoFromFace(fs[0]);
    int suiteNo=cardSuiteNoFromSuite(fs[1]);

    if (faceNo >= 0 && suiteNo >= 0) {
      int card = cardFromFaceSuiteNo(faceNo,suiteNo);
      return card;
    }
    return -1;
  }
  return -1;
}



void CIOCardsFmtInit(CIOCardsFmt *me,int mode, FILE *io) {
  CIOInit(&me->base);
  me->base.read = (CIOReadPtr) &CIOCardsFmtRead;
  me->base.write = (CIOWritePtr) &CIOCardsFmtWrite;
  me->mode=mode;
  me->io=io;
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
      const char *op="--encrypt=";
      int oplen = strlen(op);      
      if (strncmp(arg,op,oplen)==0) {
	const char *plain=arg+oplen;
	Deck deck;
	DeckKey(deck,key);

	CIOArray wcPlain;
	CIOArrayWideCharInit(&wcPlain,NULL,0,0,0,INT_MAX);	
	CIOArraySetUtf8(&wcPlain,plain);

	CIORand rcg;
	CIORandInit(&rcg);

	CIOCardsFmt fmt;
	CIOCardsFmtInit(&fmt,mode,stdout);
	encryptEnvelopeIO(deck,&wcPlain.base, &rcg.base, &fmt.base);

	CIOClose(&fmt);
	CIOClose(&rcg);
	CIOClose(&wcPlain);
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

	CIOArray u8Cipher;
	CIOArrayU8Init(&u8Cipher,NULL,0,0,0,INT_MAX);
	FILE *in=NULL;
	if (strcmp(cipher,"-") != 0) {
	  in = fmemopen((char*)cipher, strlen(cipher), "r");
	} else {
	  in = stdin;
	}
	
	CIOCardsFmt fmt;
	CIOCardsFmtInit(&fmt,mode,in);
	for (;;) {
	  int card = CIORead(&fmt);
	  if (card >= 0) {
	    CIOWrite(&u8Cipher,card);
	  } else {
	    break;
	  }
	}
	CIOClose(&fmt);
	if (in != stdin) fclose(in);
	u8Cipher.position = 0;

	CIOUTF8 utf8io;
	CIOUTF8Init(&utf8io,stdout);
	decryptEnvelopeIO(deck,&u8Cipher.base, NULL, &utf8io.base);
	CIOClose(&utf8io);
	CIOClose(&u8Cipher);
	deckInit(deck);
	continue;
      }
    }
  }
  return 0;
}
