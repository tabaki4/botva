#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <map>
#include <direct.h>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sys/types.h>
#include <errno.h>
#include "common.hpp"
#include "AES.hpp"
#include "DH.hpp"
#include "client.hpp"
#include "AEScon.hpp"
#include "mp.hpp"

extern struct cfg_struct g_cfg;
std::map<std::string, std::string> g_keys;

std::string ip = "192.168.1.79";
constexpr int UDP_port = 781;

using namespace AES;
using namespace DH;
using namespace client;
using namespace mp;

void err(std::string str) {
    perror("ERROR");
}

void first_use(cfg_struct cfg) {
    std::cout << "Do you already have an account? (y/n) ";
    std::string sign;
    std::cin >> sign;
    std::string nickname, passwd;
    if (sign == "n") {
        AEScon server_con = sign_up_con(cfg.ip, cfg.UDP_port);
        while (true) {
            std::cout << "Enter your nickname: ";
            std::cin >> nickname;
            std::cout << "Enter your password: ";
            std::cin >> passwd;
            std::string response = sign_up(server_con, nickname, passwd);
            std::cout << response << '\n';
            if (response == "Success")
                break;
            if (response == "An error occured")
                exit(1);
        }
    }
    else {
        std::cout << "Enter your nickname: ";
        std::cin >> nickname;
        std::cout << "Enter your password: ";
        std::cin >> passwd;
    }
    std::ofstream cfg_file("cfg");
    if (cfg_file.is_open()) {
        cfg_file << nickname << '\n' << passwd << '\n';
        cfg_file.flush();
        cfg_file.close();
    }
    else {
        std::cout << "bobby";
    }
}

int main() {
    cfg_struct cfg;
    cfg.ip = "192.168.1.79";
    cfg.UDP_port = 781;
    WSADATA WSAdata;
    WORD wVersion = MAKEWORD(2, 2);
    WSAStartup(wVersion, &WSAdata);

    errno = 0;
    std::cout << std::flush;
    srand(time(0));
    if (!std::filesystem::exists("cfg"))
        first_use(cfg);
    std::string nickname, passwd;

    std::ifstream cfg_file;
    cfg_file.open("cfg");
    std::getline(cfg_file, nickname);
    std::getline(cfg_file, passwd);

    AEScon server_con = UDP_init(ip, UDP_port, "login");
    AEScon client_con(server_con);
    AEScon l_con = login(cfg.ip, server_con, nickname, passwd);
    std::thread recv_Thread(recv_thread, l_con);
    recv_Thread.detach();
    while (true) {
        std::string to;
        std::cout << "Who do you want to send message to: ";
        std::cin >> to;
        while (true) {
            std::string msg;
            std::cout << "Enter your message: ";
            std::cin >> msg;
            server_con.AES_send("message");
            server_con.AES_send(to);
            std::string success = server_con.AES_recv();
            if (success == "Success") {
                client_con.AES_send(msg);
                std::cout << "message sent\n" << std::flush;
            }
            else if (success == "Error")
                std::cout << server_con.AES_recv() << '\n' << std::flush;
        }
    }
    
	return 0;
}
