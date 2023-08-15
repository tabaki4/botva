#pragma once
#include "BigIntegerLibrary.hh"
#include <random>
#include <sstream>
#include <array>
#include <cctype>

namespace RSA{
BigUnsigned get_interprime(BigUnsigned n);
std::array<BigUnsigned, 4> from_set(std::string rsa_set);
std::string get_AES_key(std::array<BigUnsigned, 4> raw_arr);
std::array<BigUnsigned, 4> get_arr(BigUnsigned n);
std::string to_set(std::array<BigUnsigned, 4> arr);
std::array<BigUnsigned, 4> encrypt(std::array<BigUnsigned, 4> arr, BigUnsigned public_key, BigUnsigned n);
}
