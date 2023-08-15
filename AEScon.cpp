#include "AEScon.hpp"
#include <string>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sys/types.h>
#include <iostream>
#include <array>
#include <errno.h>
#include "mp.hpp"
#include "DH.hpp"
#include "AES.hpp"

using namespace mp;
using namespace DH;
using namespace AES;

std::string AEScon::get_str_error() {
    char ch_error_msg[256];
    strerror_s(ch_error_msg, errno); 
    std::string error_msg = ch_error_msg;
    return error_msg;
}

bool AEScon::request(int flag) {
    auto p_arr = get_p_arr();
    auto private_key_arr = get_private_key_arr();
    std::string my_shared_key_set = to_set(get_shared_key_arr(p_arr, private_key_arr));
    std::string p_set = to_set(p_arr);
    
    if(!mp_send(AEScon::sock, p_set, flag))
        return false;
    if(!mp_send(AEScon::sock, my_shared_key_set, flag))
        return false;

    std::string shared_key_set = mp_recv(AEScon::sock, flag);
    auto shared_key_arr = from_set(shared_key_set);

    AEScon::AES_key = get_AES_key(p_arr, private_key_arr, shared_key_arr);
    return true;
}

bool AEScon::accept(int flag) {
    std::string p_set = mp_recv(AEScon::sock, flag);
    std::string shared_key_set = mp_recv(AEScon::sock, flag);

    auto p_arr = from_set(p_set);
    auto shared_key_arr = from_set(shared_key_set);

    auto private_key_arr = get_private_key_arr();
    auto my_shared_key_arr = get_shared_key_arr(p_arr, private_key_arr);
    std::string my_shared_key_set = to_set(my_shared_key_arr);
    if(!mp_send(AEScon::sock, my_shared_key_set, flag))
        return false;

    AEScon::AES_key = get_AES_key(p_arr, private_key_arr, shared_key_arr);
    return true;
}

bool AEScon::AES_send(std::string msg, int flag) {
    std::string encrypted_msg = encrypt(AEScon::AES_key, msg);
    return mp_send(AEScon::sock, encrypted_msg, flag);
}

std::string AEScon::AES_recv(int flag) {
    std::string encrypted_msg = mp_recv(AEScon::sock, flag);
    return decrypt(AEScon::AES_key, encrypted_msg);
}
 