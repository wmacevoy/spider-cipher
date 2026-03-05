#include <spider_cipher_rng.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>

int main(int argc, const char *argv[]) {
  spider_cipher::os_rng<> rng;

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
