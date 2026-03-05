#ifndef SPIDER_CIPHER_RNG_HPP
#define SPIDER_CIPHER_RNG_HPP

#include <functional>
#include <random>
#include <spider_cipher_core.hpp>

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

  
  template <uint8_t _size = 40>
  uint8_t os_base_rng() {
    static std::random_device urandom;
    return urandom() % _size;
  }
  
  template <uint8_t _size = 40, bool _is_perfect = false>
  class os_rng : public rng<_size,_is_perfect> {
  public: os_rng() : rng<_size,_is_perfect>(std::function<uint8_t()>(os_base_rng<_size>)) {}
  };

}
#endif
