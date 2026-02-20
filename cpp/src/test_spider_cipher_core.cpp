#include <spider_cipher_core.hpp>

#include <map>
#include <cassert>
#include <iostream>

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

std::vector<uint8_t> encrypt(const std::string &key, const std::string &plain) {
  spider_cipher::deck<size> crypt;
  for (auto letter : key) {
    crypt.update(book.map(letter));
  }
  std::vector<uint8_t> cipher;
  for (char letter : plain) {
    uint8_t clear = book.map(letter);
    cipher.push_back(crypt.scramble(clear));
    crypt.update(clear);
  }
  return cipher;
}
  
std::string decrypt(const std::string &key, const std::vector<uint8_t> &cipher) {
  spider_cipher::deck<size> crypt;
  int tag = 0;
  for (auto letter : key) {
    crypt.update(book.map(letter));
  }
  std::string plain;
  for (uint8_t scrambled : cipher) {
    uint8_t clear = crypt.unscramble(scrambled);
    plain.push_back(book.unmap(clear));
    crypt.update(clear);
  }
  return plain;
}

int main() {
  std::string key = "super-secret-phrase";
  std::string message = "hello-world!-stop.";
  std::vector<uint8_t> cipher = encrypt(key,message);
  std::string decode = decrypt(key,cipher);

  for (uint8_t card : cipher) {
    std::cout << card / 10;
  }
  std::cout << std::endl;

  for (uint8_t card : cipher) {
    std::cout << card % 10;
  }
  std::cout << std::endl;
  
  std::cout << decode << std::endl;

  return 0;
}
