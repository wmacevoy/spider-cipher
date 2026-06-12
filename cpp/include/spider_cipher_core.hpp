#ifndef SPIDER_CIPHER_HPP
#define SPIDER_CIPHER_HPP

#include <cstdint>
#include <array>
#include <functional>

namespace spider_cipher {

  //
  // spider-cipher template with a goals of
  //
  // * reasonable efficiency
  // * constant time
  // * no residuals
  //
  
  template <uint8_t _size=40, bool _is_perfect = false>
  class deck {
  public: static const uint8_t size = _size;
  public: static const bool is_perfect = _is_perfect;

  private: std::array < uint8_t, size > cards;
    
  public: uint8_t card(int8_t at) const { return (*this)[(at + size) % size]; }
  public: uint8_t& card(int8_t at) { return (*this)[(at + size) % size]; }
    
  public: void reset(bool reverse = false) {
    if (!reverse) {
      for (uint8_t i=0; i<size; ++i) cards[i]=i;
    } else {
      for (uint8_t i=0; i<size; ++i) cards[i]=size-1 - i;
    }
  }
  public: deck(bool reverse = false) { reset(reverse); }
  public: deck(const deck &copy) : cards(copy.cards) {}
  public: ~deck() { reset(); }

  public: void swap(int8_t i, int8_t j) {
      i = ((i+size) % size);
      j = ((j+size) % size);
      uint8_t a = cards[i];
      uint8_t b = cards[j];
      cards[i] = b;
      cards[j] = a;
      a = 0;
      b = 0;
    }

  public: void cut_to(const uint8_t at, deck &to) const {
      for (uint8_t i = 0; i < size; ++i) {
	to.cards[i] = cards[(i + at) % size];
      }
    }

  public: void back_front_shuffle_to(deck &to) const {
      for (uint8_t i = 0; i < size / 2; ++i)
	{
	  to.cards[(size/2)+i] = cards[2 * i];
	  to.cards[(size/2)-1-i] = cards[2 * i+1];
	}
      if (size % 2 != 0)
	{
	  to.cards[size - 1] = cards[size - 1];
	}
    }

  public: uint8_t find(uint8_t card) const {
      uint8_t at = 0;
      for (uint8_t i = 0; i < size; ++i)
	{
	  at = at | (i & -(cards[i] == card));
	}
      return at;
    }
    
  public: uint8_t noise() const {
      uint8_t tag = (cards[2] + (size - 1)) % size;
      uint8_t at = find(tag);
      uint8_t noise = cards[(at + 1) % size];
      tag = 0;
      at = 0;
      return noise;
    }

  public: void update(uint8_t clear) {
      uint8_t cut_card = (cards[0] + clear) % size;
      uint8_t cut_at = find(cut_card);

      deck work;
      cut_to(cut_at,work);
      work.back_front_shuffle_to(*this);
      if (is_perfect) {
	// perfect spider shuffle swaps shuffled positions 0 and size/2-1
	// (the same as swapping the 2nd and bottom cards before the shuffle)
	cut_card = cards[size/2-1];
	cards[size/2-1]=cards[0];
	cards[0]=cut_card;
      }
      cut_card=0;
      cut_at=0;
    }

  public: uint8_t scramble(uint8_t clear) const {
      return (clear + noise()) % size;
    }

  public: uint8_t unscramble(uint8_t scrambled) const {
      return (scrambled + size-noise()) % size;
    }

  public: void shuffle(std::function<unsigned (unsigned)> random) {
      for (uint8_t i = 0 ; i<size-1; ++i) {
	swap(i,i+random(size-i));
      }
  }

  public: void arrange(const deck &by) {
    std::array < uint8_t , size > arranged;
    for (uint8_t i = 0; i < size; ++i) {
      arranged[i]=cards[by.cards[i]];
    }
    cards=arranged;
    for (uint8_t i = 0; i < size; ++i) {
      arranged[i]=0;
    }
  };

  public: bool operator==(const deck &to) const {
    return cards == to.cards;
  }

  };
}
#endif
