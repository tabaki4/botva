#include "BigIntegerLibrary.hh"
#include <random>
#include <sstream>
#include <array>
#include <cctype>
#define MIN_PRIME 17
#define MIN_PRIME_I 7

namespace RSA {

char set_delim{ '#' };
std::random_device rd;
std::mt19937 gen(rd());

void split_64(uint64_t r, unsigned long& a, unsigned long& rem) {
    a = r / ULONG_MAX;
    rem = r % ULONG_MAX;
}

uint64_t BU_to_64(BigUnsigned BU) {
    unsigned long lower = (BU % ULONG_MAX).toUnsignedLong();
    unsigned long upper = (BU / ULONG_MAX).toUnsignedLong();
    uint64_t res = ULONG_MAX * upper + lower;
    return res;
}

BigUnsigned get_interprime(BigUnsigned n) {
    std::uniform_int_distribution<uint64_t> dis(
        MIN_PRIME,
        std::numeric_limits<uint64_t>::max()
    );
    BigUnsigned x = ULONG_MAX;
    unsigned long b{}, rem{};
    split_64(dis(gen), b, rem);
    x *= b;
    x += rem;
    while (gcd(x, n) != 1) {
        x = ULONG_MAX;
        b = 0;
        rem = 0;
        split_64(dis(gen), b, rem);
        x *= b;
        x += rem;
    }
    return x;
}

std::array<BigUnsigned, 4> get_arr(BigUnsigned n) {
    std::array<BigUnsigned, 4> res{};
    for (unsigned i = 0; i < 4; i++) {
        res[i] = get_interprime(n);
    }
    return res;
}

std::array<BigUnsigned, 4> from_set(std::string rsa_set) {
    std::array<BigUnsigned, 4> result;
    std::stringstream s_stream(rsa_set);
    for (uint8_t i = 0; i < 4; i++) {
        std::string substr;
        getline(s_stream, substr, set_delim);
        result[i] = std::stoi(substr);
    }
    return result;
}

std::string to_set(std::array<BigUnsigned, 4> arr) {
    std::string str_delim(1, set_delim);
    std::stringstream ss;
    std::array<std::string, 4> s_arr;
    for (unsigned i = 0; i < 4; i++) {
        s_arr[i] = bigUnsignedToString(arr[i]);
    }
    std::copy(s_arr.begin(), s_arr.end(), std::ostream_iterator<std::string>(ss, str_delim.c_str()));
    return ss.str();
}

std::array<BigUnsigned, 4> encrypt(std::array<BigUnsigned, 4> arr, BigUnsigned public_key, BigUnsigned n) {
    for (unsigned i = 0; i < 4; i++) {
        arr[i] = modexp(arr[i], public_key, n);
    }
    return arr;
}

std::string get_AES_key(std::array<BigUnsigned, 4> raw_arr) {
    std::string key;
    BigUnsigned raw_key = 1;
    for (int i = 0; i < 4; i++) {
        BigUnsigned msg = raw_arr[i];
        raw_key *= msg;
    }
    BigUnsignedInABase BU_key(raw_key, 16);
    std::string AES_key = std::string(BU_key);
    std::transform(AES_key.begin(), AES_key.end(), AES_key.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return AES_key;
}
}
