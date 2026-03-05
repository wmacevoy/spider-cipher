#ifndef SPIDER_CIPHER_HASH_HPP
#define SPIDER_CIPHER_HASH_HPP

#include <spider_cipher_core.hpp>

namespace spider_cipher {

  template <bool reverse = false, uint8_t _size=40, bool _is_perfect = false>
  class hash {
  public: static const uint8_t size = _size;
  public: static const bool is_perfect = _is_perfect;
  private: deck<size,is_perfect> state;
  public: void reset() {
    state.reset(reverse);
  }
  public: hash() { reset(); }

  public: void update(uint8_t clear) {
    state.update(clear);
  }

  public: void digest(uint8_t len, uint8_t *tag) {
    deck<size,is_perfect> work(state);
    /* first pass */
    for (uint8_t i = 0; i < len; ++i) {
      tag[i] = work.scramble(size-1);
      work.update(size-1);
    }
    /* second pass: mix with additional scramble to prevent cycle-based collisions */
    for (uint8_t i = 0; i < len; ++i) {
      tag[i] = (tag[i] + work.scramble(size-1)) % size;
      work.update(size-1);
    }
  }

  public: bool operator==(const hash<reverse,size,is_perfect> &to) const {
      return state == to.state;
  }
    
  };
}
#endif
