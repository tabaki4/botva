#include "mp.hpp"
#include <string>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sys/types.h>
#include <sstream>
#include <vector>
#include <iostream>

namespace mp {
void error(std::string msg) {
    std::stringstream ss;
    ss << WSAGetLastError();
    fprintf(stderr, ss.str().c_str());
    std::terminate();
}

std::string mp_recv(SOCKET sock, int flag) {
    char ch_len[4];
    recv(sock, ch_len, sizeof(ch_len), MSG_PEEK);
    if (WSAGetLastError() != 0) {
        error("ERROR recieving length mp_recv");
    }
    int len = atoi(ch_len);
    
    char* ch_data = new char[4 + len];
    recv(sock, ch_data, 4 + len, flag);
    if (WSAGetLastError() != 0) {
        error("ERROR recieving data mp_recv");
    }
    std::string raw_data = ch_data;
    std::string data = raw_data.substr(4, len);
    //std::cout << "recieved data: " << data << '\n' << "length of data: " << data.length() << '\n' << std::flush;
    return data;
}

bool mp_send(SOCKET sock, std::string raw_msg, int flag) {
    long n{};
    unsigned long len = raw_msg.length();
    std::stringstream ss;
    ss << len;
    std::string str_len = ss.str();
    std::string leading_zeros (4 - str_len.length(), '0');
    std::string msg = leading_zeros + str_len + raw_msg;
    //std::cout << "msg: " << msg << '\n' << "length: " << msg.length() << '\n' << std::flush;
    n = send(sock, msg.c_str(), 4 + len, flag);
    if (n == -1) {
        error("ERROR sending mp_send");
        return false;
    }
    return true;
}

bool mp_send(SOCKET sock, std::vector<std::string> raw_msg_vec, int flag) {
    long n{};
    std::string msg;
    for (int i = 0; i < raw_msg_vec.size(); i++) {
        std::string raw_msg = raw_msg_vec[i];
        unsigned long len = raw_msg.length();
        std::stringstream ss;
        ss << len;
        std::string str_len = ss.str();
        std::string leading_zeros (4 - str_len.length(), '0');
        msg = msg + leading_zeros + str_len + raw_msg;
    }
    n = send(sock, msg.c_str(), msg.length(), flag);
    if (n == -1) {
        error("ERROR sending mp_sendto");
        return false;
    }
    return true;
}

std::string mp_recvfrom(SOCKET sock, sockaddr* cli_addr1, int flag) {
    char ch_len[4];
    sockaddr cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    recvfrom(sock, ch_len, sizeof(ch_len), MSG_PEEK, &cli_addr, &cli_len);
    if (WSAGetLastError() != 10040) {
        error("ERROR recieving length mp_recvfrom");
    }
    WSASetLastError(0);
    int len = atoi(ch_len);
    
    char *ch_data = new char[4+len];
    recvfrom(sock, ch_data, len + 4, flag, &cli_addr, &cli_len);
    if (WSAGetLastError() != 0) {
        error("ERROR recieving data mp_recvfrom");
    }
    *cli_addr1 = cli_addr;
    std::string raw_data = ch_data;
    std::string data = raw_data.substr(4, len);
    return data;
}

bool mp_sendto(SOCKET sock, std::string raw_msg, sockaddr_in cli_addr, int flag) {
    unsigned long len = raw_msg.length();
    std::stringstream ss;
    ss << len;
    std::string str_len = ss.str();
    std::string leading_zeros (4 - str_len.length(), '0');
    std::string msg = leading_zeros + str_len + raw_msg;
    
    sendto(sock, msg.c_str(), 4 + len, flag, (sockaddr*) & cli_addr, sizeof(cli_addr));
    if (WSAGetLastError() != 0) {
        error("ERROR sending mp_sendto");
        return false;
    }
    return true;
}

bool mp_sendto(SOCKET sock, std::vector<std::string> raw_msg_vec, sockaddr_in cli_addr, int flag) {
    std::string msg;
    for (int i = 0; i < raw_msg_vec.size(); i++) {
        std::string raw_msg = raw_msg_vec[i];
        unsigned long len = raw_msg.length();
        std::stringstream ss;
        ss << len;
        std::string str_len = ss.str();
        std::string leading_zeros (4 - str_len.length(), '0');
        msg = msg + leading_zeros + str_len + raw_msg;
    }
    sendto(sock, msg.c_str(), msg.length(), flag, (sockaddr*) & cli_addr, sizeof(cli_addr));
    if (WSAGetLastError() != 0) {
        error("ERROR sending mp_sendto");
        return false;
    }
    return true;
}

bool ping(SOCKET sock) {
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *) & timeout, sizeof(timeout));
    mp_send(sock, "marko");
    char ch_len[4];
    recv(sock, ch_len, sizeof(ch_len), MSG_PEEK);
    if (WSAGetLastError() != 0) {
        error("ERROR recieving length mp_recv");
    }
    int len = atoi(ch_len);
    char *ch_data = new char[4+len];
    recv(sock, ch_data, 4 + len, 0);
    if (WSAGetLastError() != 0) {
        error("ERROR recieving data mp_recv");
    }
    std::string raw_data = ch_data;
    std::string data = raw_data.substr(4, len);
    std::cout << "\nresponse for marko: " << data << "\nlength: " << data.length() << '\n' << std::flush;
    
    struct timeval timeout1;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *) & timeout1, sizeof(timeout1));
    
    if(data != "polo" || WSAGetLastError() != 0)
        return false;
    else
        return true;
}
}
