#ifndef SPIDER_CIPHER_RNG_HPP
#define SPIDER_CIPHER_RNG_HPP

#include <spider_cipher_core.hpp>
#include <functional>

namespace spider_cipher {
  
  template <uint8_t _size = 40, bool _is_perfect = false>
  class rng {
  public: static const uint8_t size = _size;
  public: static const bool is_perfect = _is_perfect;
  private: deck<size,is_perfect> pool;
  private: const std::function<uint8_t()> base; // base() -> rng 0 .. size-1
  public: rng(std::function<uint8_t()> _base) :base(_base) {
    for (uint8_t i = 0; i<size; ++i) { pool.update(base()); }
  }
  public:  uint8_t next() {
      uint8_t scrambled = pool.scramble(size-1);
      pool.update(base());
      pool.update(base());
      return scrambled;
    }

  };

}
#endif
