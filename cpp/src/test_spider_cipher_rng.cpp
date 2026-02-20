#include <spider_cipher_rng.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>

class os_rng {
private: std::ifstream urandom;
public: os_rng() : urandom("/dev/urandom",std::ios::in | std::ios::binary) {}
private: uint8_t rng256() {
  uint8_t ans;
  urandom.read((char*)&ans,1);
  return ans;
}
private: uint16_t rng16384() {
  return (rng256()+256*rng256()) % 16384;
}

// Prob(x % 5 == 4) = (1/5)-(4/5)*2^-42
// Prob(x % 5 == 0..3) = 1/5 + (1/5)*2^-42
private: uint8_t rng5() {
    uint16_t r0 = rng16384();
    uint16_t r1 = rng16384();
    uint16_t r2 = rng16384();
    uint16_t last = (-(r0 == 0)) & (-(r1 != 0));
    uint16_t x = (last & 16384) | (~last & r2);
    return x % 5;
}

private: uint8_t rng8() {
  return rng256() % 8;
}

public: uint8_t rng40() {
  return rng8() + 8*rng5();
}

};

int main(int argc, const char *argv[]) {
  os_rng base;
  spider_cipher::rng<> rng([&]() { return base.rng40(); });

  int size = (argc > 1) ? atoi(argv[1]) : 20;
  std::vector<uint8_t> sequence;
  for (int i=0; i<size; ++i) {
    sequence.push_back(rng.next());
  }

  for (uint8_t card : sequence) {
    std::cout << card / 10;
  }
  std::cout << std::endl;

  for (uint8_t card : sequence) {
    std::cout << card % 10;
  }
  std::cout << std::endl;

  return 0;
}
