#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define DEFAULT_N_MAX 4096

/* This proves <τ,β> = S_n for even n, 2 <= n <= n_max
   (--all runs n = 2, 4, ..., n_max-2; an explicit n may equal n_max):
   τ is the unit translation (cut): (τ x)_i = x_{(i+1) mod n}
   β is the monge (back-front) shuffle:
     (β x)_{n/2+i} = x_{2i},  (β x)_{n/2-1-i} = x_{2i+1},  0 <= i < n/2 */

/* n_max should not be too large - there is no practical use of n_max > 256 */

/* deck_t here is flexible for n, which must be positive, even, and n <= n_max

   This version is just to prove facts about the generic spider graph
   for different sizes of the deck. Use spider_cipher.c (or a deck
   of 40 cards) in actual use. */

int n_max = DEFAULT_N_MAX;

typedef struct {
  int n;
  int *cards;
} deck_t;

deck_t *deck_new(int n) {
  assert((n > 0) && (n % 2 == 0) && (n <= n_max));
  deck_t *me = (deck_t*)malloc(sizeof(deck_t));
  assert(me != NULL);
  me->n=n;
  me->cards = (int*) malloc(sizeof(int)*n);
  assert(me->cards != NULL);
  for (int i=0; i<n; ++i) {
    me->cards[i]=i;
  }
  return me;
}

void deck_delete(deck_t *me) {
  free(me->cards);
  free(me);
}

deck_t *deck_copy(deck_t *me) {
  int n = me->n;
  deck_t *cp = deck_new(n);
  for (int i=0; i<n; ++i) {
    cp->cards[i]=me->cards[i];
  }
  return cp;
}

void deck_cut(deck_t *me, int c) {
  int n = me->n;
  if (!((0 <= c) && (c < n))) {
    printf("n=%d, c=%d\n",n,c);
    assert((0 <= c) && (c < n));
  }
  deck_t *cp = deck_copy(me);
  for (int i=0; i<n; ++i) {
    me->cards[i]=cp->cards[(c+i) % n];
  }
  deck_delete(cp);
}

void deck_back_front_shuffle(deck_t *me) {
  int n2 = me->n/2;
  deck_t *cp = deck_copy(me);
  for (int i=0; i<n2; ++i) {
    me->cards[n2+i]=cp->cards[2*i];
    me->cards[n2-1-i]=cp->cards[2*i+1];
  }
  deck_delete(cp);
}

void deck_inv_back_front_shuffle(deck_t *me) {
  int n2 = me->n/2;
  deck_t *cp = deck_copy(me);
  for (int i=0; i<n2; ++i) {
    me->cards[2*i]=cp->cards[n2+i];
    me->cards[2*i+1]=cp->cards[n2-1-i];
  }
  deck_delete(cp);
}

void deck_reverse(deck_t *me) {
  int n = me->n;
  deck_t *cp = deck_copy(me);
  for (int i=0; i<n; ++i) {
    me->cards[i]=cp->cards[n-i-1];
  }
  deck_delete(cp);
}

void deck_exchange_pairs(deck_t *me) {
  int n = me->n;
  deck_t *cp = deck_copy(me);
  for (int i=0; i<n; ++i) {
    me->cards[i]=cp->cards[((i%2)==0) ? i + 1 : i - 1];
  }
  deck_delete(cp);
}

void deck_exchange(deck_t *me,int x, int y) {
  int n = me->n;
  assert(0 <= x && x < n);
  assert(0 <= y && y < n);
  
  int tmp = me->cards[x];
  me->cards[x] = me->cards[y];
  me->cards[y] = tmp;
  
}

void deck_shuffle(deck_t *me, int c) {
  deck_cut(me,c);
  deck_back_front_shuffle(me);
}

void deck_invert(deck_t *me) {
  int n = me->n;
  deck_t *cp = deck_copy(me);
  for (int i=0; i<n; ++i) {
    me->cards[cp->cards[i]]=i;
  }
  deck_delete(cp);
}

void deck_print(deck_t *me, FILE *out) {
  int n=me->n;
  for (int i=0; i<n; ++i) {
    fprintf(out,"%s%02d",(i != 0) ? " " : "",me->cards[i]);
  }
}

int deck_equal(deck_t *me, deck_t *to) {
  if (me->n != to->n) { return 0; }
  int n = me->n;
  for (int i = 0; i<n; ++i) {
    if (me->cards[i] != to->cards[i]) { return 0; }
  }
  return 1;
}

void deck_assert(deck_t *me, const char *op, deck_t *expected, FILE *out) {
  int equal=deck_equal(me,expected);
  if (out != NULL) {
    fprintf(out,(equal ? "equal: " : "not equal: "));
    deck_print(me,out);
    fprintf(out,"%s",op);
    deck_print(expected,out);
    fprintf(out,"\n");
  }

  if (strcmp(op,"==") == 0) {
    assert(equal);
  } else if (strcmp(op,"!=") == 0) {
    assert(!equal);
  } else {
    assert("unsupported op" == NULL);
  }
}

void test_all_n(void (*test_n)(int n,FILE *out),FILE *out) {
  for (int n = 2; n < n_max; n += 2) {
    test_n(n, out);
  }
}

void test_reverse_reachable_n(int n, FILE *out) {
  deck_t *reverse = deck_new(n);
  deck_reverse(reverse);
  /* r=[n-1,n-2,..,1,0] */
  
  deck_t *deck = deck_new(n);
  /* d=[0,1,..,n/2-2,n/2-1 : n/2,n/2+1,..,n-2,n-1], where : indicates midway */

  deck_back_front_shuffle(deck);
  /* d=[n-1,n-3,..3,1 : 0,2,4,..,n-4,n-2] */
  
  deck_cut(deck,n/2);
  /* d=[0,2,..,n-4,n-2 : n-1,n-3,..,3,1] */

  
  deck_inv_back_front_shuffle(deck);
  /* inverse takes from center back-front into new stack... */
  /* d=[n-1,n-2,n-3,n-4,..,3,2,1,0] */

  deck_assert(deck,"==",reverse,out);
  
  deck_delete(deck);
  deck_delete(reverse);
}

void test_exchange_pairs_reachable_n(int n, FILE *out) {
  deck_t *exchange_pairs = deck_new(n);
  deck_exchange_pairs(exchange_pairs);
  /* x=[1,0,3,2,5,4,..,n-3,n-4,n-1,n-2] */
  
  deck_t *deck = deck_new(n);
  /* d=[0,1,..,n/2-2,n/2-1 : n/2,n/2+1,..,n-2,n-1], where : indicates midway */  
  deck_back_front_shuffle(deck);
  /* d=[n-1,n-3,..,3,1 : 0,2,..,n-4,n-2] */  

  deck_reverse(deck);
  /* d=[n-2,n-4,..,2,0 : 1,3,..,n-3,n-1] */
  
  deck_inv_back_front_shuffle(deck);
  /* d=[1,0,3,2,..,n-3,n-4,n-1,n-2] */

  deck_assert(deck,"==",exchange_pairs,out);

  deck_delete(deck);
  deck_delete(exchange_pairs);
}

void test_exchange_01_reachable_n(int n, FILE *out) {
  deck_t *exchange_01 = deck_new(n);
  deck_exchange(exchange_01,0,1);
  /* S01=[1,0,2,3,4:5,6,7,8,9] (n=10) */
  /* s01=[1,0,2,3,..,n-4,n-3,n-2,n-1] */  

  deck_t *deck = deck_new(n);
  /* D=[0,1,2,3,4:5,6,7,8,9] */
  /* d=[0,1,2,3,..,n/2-2,n/2-1 : n/2,n/2+1,..,n-2,n-1] */  

  deck_cut(deck,1);
  /* D=[1,2,3,4,5:6,7,8,9,0] */
  /* d=[1,2,..,n/2-1,n/2 : n/2+1,n/2+2,..,n-2,n-1,0] */

  deck_back_front_shuffle(deck);
  /* D=[0,8,6,4,2:1,3,5,7,9] */
  /* d=[0,n-2,n-4,..,4,2:1,3,..,n-3,n-1] */
  
  deck_cut(deck,(n/2+1)%n);
  /* D=[3,5,7,9,0:8,6,4,2,1] */
  /* d=[3,5,7,..,n-3,n-1,0:n-2,n-4,n-6,..,4,2,1] */

  deck_inv_back_front_shuffle(deck);
  /* D=[8,0,6,9,4:7,2,5,1,3] */
  /* d=[n-2,0,n-4,n-1,n-6,n-3,..,4,7,2,5,1,3] */
  
  deck_reverse(deck);
  /* D=[3,1,5,2,7:4,9,6,0,8] */
  /* d=[3,1,5,2,7,..,n-3,n-6,n-1,n-4,0,n-2] */
  
  deck_exchange_pairs(deck);
  /* D=[1,3,2,5,4:7,6,9,8,0] */
  /* d=[1,3,2,5,4,7,..,n-6,n-3,n-4,n-1,n-2,0] */
  
  deck_cut(deck,1);
  /* D=[3,2,5,4,7:6,9,8,0,1] */
  /* d=[3,2,5,4,7,..,n-6,n-3,n-4,n-1,n-2,0,1] */
  
  deck_exchange_pairs(deck);
  /* D=[2,3,4,5,6:7,8,9,1,0] */
  /* d=[2,3,4,5,6,7,..,n-6,n-5,n-4,n-3,n-2,n-1,1,0] */

  deck_cut(deck,n-2);
  /* D=[1,0,2,3,4:5,6,7,8,9] */
  /* d=[1,0,2,3,4,5,6,7,..,n-6,n-5,n-4,n-3,n-2,n-1] */
  
  deck_assert(deck,"==",exchange_01,out);

  deck_delete(deck);
  deck_delete(exchange_01);
}

int main(int argc, const char *argv[]) {
  int verbose = 0;
  FILE *out = verbose ? stderr : NULL;
  for (int i=1; i<argc; ++i) {
    if (strcmp(argv[i],"--verbose")==0) {
      verbose = 1;
      out = stderr;
      continue;
    }
    if (strcmp(argv[i],"++verbose")==0) {
      verbose = 0;
      out = NULL;
      continue;
    }
    if (strcmp(argv[i],"--all")==0) {
      test_all_n(test_reverse_reachable_n,out);
      test_all_n(test_exchange_pairs_reachable_n,out);
      test_all_n(test_exchange_01_reachable_n,out);
      continue;
    }
    if (strcmp(argv[i],"--n_max")==0) {
      ++i;
      n_max = atoi(argv[i]);
      continue;
    }

    int n = atoi(argv[i]);

    if (n > 0 && (n % 2) == 0 && n <= n_max) {
      test_reverse_reachable_n(n,out);
      test_exchange_pairs_reachable_n(n,out);
      test_exchange_01_reachable_n(n,out);
    } else {
      fprintf(stderr,"n=%d is invalid (0 < n even <= %d)\n",n,n_max);
      assert(n > 0 && (n % 2) == 0 && n <= n_max);
    }
  }

  fprintf(stdout,"all tests pass\n");
  
  return 0;
}
