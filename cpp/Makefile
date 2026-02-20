CXXFLAGS=-std=c++11 -O -Iinclude -fPIC

all : bin/test_spider_cipher_core bin/test_spider_cipher_rng bin/test_spider_cipher_hash

bin/test_spider_cipher_core : src/test_spider_cipher_core.cpp include/spider_cipher_core.hpp
	mkdir -p bin
	$(CXX) -o $@ $(CXXFLAGS) src/test_spider_cipher_core.cpp

bin/test_spider_cipher_rng : src/test_spider_cipher_rng.cpp include/spider_cipher_rng.hpp include/spider_cipher_core.hpp
	mkdir -p bin
	$(CXX) -o $@ $(CXXFLAGS) src/test_spider_cipher_rng.cpp

bin/test_spider_cipher_hash : src/test_spider_cipher_hash.cpp include/spider_cipher_hash.hpp include/spider_cipher_core.hpp
	mkdir -p bin
	$(CXX) -o $@ $(CXXFLAGS) src/test_spider_cipher_hash.cpp
