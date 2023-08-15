#pragma once
#include <string>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
class AEScon {
public:
    SOCKET sock{0};
    std::string AES_key;
    AEScon() {}
    AEScon(SOCKET sock1)
        : sock(sock1)
    {}
    std::string get_str_error();
    bool request(int flag = 0);
    bool accept(int flag = 0);
    bool AES_send(std::string msg, int flag = 0);
    std::string AES_recv(int flag = 0);
};
