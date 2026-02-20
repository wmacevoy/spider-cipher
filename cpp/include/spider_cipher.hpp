#ifndef SPIDER_CIPHER_HPP
#define SPIDER_CIPHER_HPP

namespace spider_cipher {

template <uint8_t _size=40, bool _is_perfect = false>
struct deck : std::array < uint8_t, size > {
  static const uint8_t size = _size;
  static const bool is_perfect = _is_perfect;
  void reset() { for (uint8_t i=0; i<size; ++i) (*this)[i]=i; }
  deck() { reset(); }
  deck(const &copy) : std::array(copy) {}
  ~deck() { reset(); }

  void swap(int8_t i, int8_t j) {
    i = ((i+size) % size);
    j = ((j+size) % size);
    uint8_t a = (*this)[i];
    uint8_t b = (*this)[j];
    (*this)[i] = b;
    (*this)[j] = a;
    a = 0;
    b = 0;
  }

  void cut_to(const uint8_t at, deck &to) const {
    for (uint8_t i = 0; i < size; ++i) {
      to[i] = (*this)[(i + at) % size];
    }
  }

  void back_front_shuffle_to(deck &to) const {
    for (uint8_t i = 0; i < size / 2; ++i)
    {
      to[(size/2)+i] = (*this)[2 * i];
      to[(size/2)-1-i] = (*this)[2 * i+1];
    }
    if (size % 2 != 0)
    {
      to[size - 1] = (*this)[size - 1];
    }
    if (is_perfect) {
      to.swap(0,size-1);
    }
  }

  uint8_t find(uint8_t card) const {
    uint8_t at = 0;
    for (uint8_t i = 0; i < size; ++i)
      {
	at = at | (i & -((*this)[i] == card));
      }
    return at;
  }
  uint8_t noise() const {
    uint8_t tag = ((*this)[2] + (size - 1)) % size;
    uint8_t at = find(tag);
    uint8_t noise = (*this)[(at + 1) % size];
    tag = 0;
    at = 0;
    return noise;
  }

  void update(uint8_t clear) {
    uint8_t cut_card = ((*this)[0] + clear) % size;
    uint8_t cut_at = find(cut_card);

    deck work;
    cut_to(cut_at,work);
    work.back_front_shuffle(*this);
    cut_card=0;
    cut_at=0;
  }

  uint8_t scramble(uint8_t clear) const {
    return (clear + noise()) % size;
  }

  uint8_t unscramble(uint8_t scrambled) const {
    return (scrambled + size-noise()) % size;
  }

  void shuffle(unsigned rng(unsigned  n, void *rng_context), void *context = NULL) {
    for (uint8_t i = 0 ; i<size; ++i) {
      swap(i,i+rng(n-i,rng_context));
    }
  }
};

  template <function<unsigned()> src,uint8_t size=40, bool is_perfect = false>
  struct rng {
    deck<size,is_perfect> pool;
    rng() {
      for (uint8_t i=0; i < pool::size; ++i) {
	next();
      }
    }

    uint8_t next() {
      uint8_t scramble = 0;
      for (uint8_t i = 0; i<2; ++i) {
	uint8_t clear = (src() % size);
	scramble = (scramble + pool.scramble(clear)) % size;
	pool.update(clear);
      }
      return scramble;
    }

  };    
}
}
#endif
