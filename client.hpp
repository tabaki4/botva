#pragma once

#include <string>
#include <array>
#include <thread>
#include <vector>
#include "AEScon.hpp"

namespace client {
AEScon UDP_init(std::string ip, int UDP_port, std::string mode);
void recv_thread(AEScon server_con);
std::array<int, 2> get_ports(std::string nickname, std::string ip, int UDP_port);
SOCKET connect_to_TCP(int port, std::string ip);
void shut_down(SOCKET sock);
bool send_message(AEScon server_con, std::string nickname, std::string message);
AEScon login(std::string ip, AEScon connection, std::string nickname, std::string password);
AEScon sign_up_con(std::string ip, int port);
std::string sign_up(AEScon connection, std::string nickname, std::string passwd);
}
