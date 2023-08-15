#pragma once
#include <string>
#include <array>
#include <bitset>

namespace AES {
std::string trim(const std::string str);
std::string encrypt(std::string key, std::string text);
std::string decrypt(std::string key, std::string text);
}
