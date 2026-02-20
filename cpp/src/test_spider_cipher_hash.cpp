#include <spider_cipher_hash.hpp>

#include <cassert>
#include <iostream>
#include <map>

const bool reverse = false;
const uint8_t size = 40;

class simple_code_book {
private: std::map < char , uint8_t > forward_map;
private: std::map < uint8_t, char > reverse_map;
  void link(char letter, uint8_t card = size) {
    if (card >= size) {
      card = forward_map.size();
    }
    forward_map[letter]=card;
    reverse_map[card]=letter;
  }
  void init() {
    for (char c = '0'; c <= '9'; ++c) {    
      link(c);
    }
    if (size >= 36) {
      for (char c = 'a'; c <= 'z'; ++c) {
	link(c);
      }
    }
    if (size >= 40) {
      link('-');
      link('.');
      link('?');
      link('!');
    }
    assert(forward_map.size() <= size);
  }

public:  uint8_t map(char c) {
    auto at = forward_map.find(c);
    if (at != forward_map.end()) {
      return at->second;
    } else {
      return size;
    }
  }

public: uint8_t unmap(uint8_t card) {
    auto at = reverse_map.find(card);
    if (at != reverse_map.end()) {
      return at->second;
    } else {
      return '*';
    }
  }
  
public: simple_code_book() { init(); }
  
};

simple_code_book book;

std::string hash(const std::string message) {
  spider_cipher::hash<reverse,size> h;
  for (auto letter : message) {
    uint8_t card = book.map(letter);
    h.update(card);
  }
  std::array<uint8_t,size> tag_cards;
  h.digest(size,&tag_cards[0]);
  std::string tag;
  for (auto card : tag_cards) {
    tag.push_back(book.unmap(card));
  }
  return tag;
}

int main(int argc, const char *argv[]) {
  {
    std::string message = "hello-world!-stop.";
    std::cout << "h(" << message << ")=" << hash(message) << std::endl;
  }

  {
    std::string message = "hello-world!-stop!";
    std::cout << "h(" << message << ")=" << hash(message) << std::endl;
  }

  return 0;
}
