#pragma once
#include <string>
#include <vector>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>

namespace mp {
std::string mp_recv(SOCKET sock, int flag = 0);
bool mp_send(SOCKET sock, std::string raw_msg, int flag = 0);
bool mp_send(SOCKET sock, std::vector<std::string> raw_msg_vec, int flag = 0);
std::string mp_recvfrom(SOCKET sock, sockaddr* cli_addr, int flag = 0);
bool mp_sendto(SOCKET sock, std::string raw_msg, sockaddr_in cli_addr, int flag = 0);
bool mp_sendto(SOCKET sock, std::vector<std::string> raw_msg_vec, sockaddr_in cli_addr, int flag = 0);
bool ping(SOCKET sock);
}
