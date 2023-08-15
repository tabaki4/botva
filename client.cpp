#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <array>
#include <sstream>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tchar.h>
#include "DH.hpp"
#include "AES.hpp"
#include "mp.cpp"
#include "AEScon.hpp"
#include "common.hpp"
#include "RSA.hpp"
#include "md5.hpp"
#include "BigIntegerLibrary.hh"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

char delim{ ';' };
char to_fixed_symbol{ '@' };

using namespace std::this_thread;
using namespace std::chrono;
using namespace DH;
using namespace AES;
using namespace mp;

namespace client {
void error(std::string msg) {
    perror(msg.c_str());
    exit(1);
}


PCWSTR toPCW(std::string str) {
    std::wstring stemp = std::wstring(str.begin(), str.end());
    PCWSTR sw = stemp.c_str();
    return sw;
}

AEScon UDP_init(std::string ip, int UDP_port, std::string mode) {
    sockaddr_in UDP_serv_addr;
    inet_pton(AF_INET, ip.c_str(), &UDP_serv_addr.sin_addr.s_addr);
    UDP_serv_addr.sin_port = htons(UDP_port);
    UDP_serv_addr.sin_family = AF_INET;
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &UDP_serv_addr, buf, sizeof(buf));

    SOCKET UDP_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    mp_sendto(UDP_sock, mode, UDP_serv_addr);
    sockaddr serv_addr;
    std::string str_port = mp_recvfrom(UDP_sock, &serv_addr);
    int login_port = std::stoi(str_port);
    
    sockaddr_in TCP_serv_addr;
    inet_pton(AF_INET, ip.c_str(), &TCP_serv_addr.sin_addr.s_addr);
    TCP_serv_addr.sin_port = login_port;
    TCP_serv_addr.sin_family = AF_INET;
    socklen_t TCP_serv_len = sizeof(TCP_serv_addr);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    connect(sock, (sockaddr *) &TCP_serv_addr, TCP_serv_len);
    AEScon connection{ sock };
    connection.request();
    return connection;
}

std::string to_fixed_length(std::string str, unsigned short len) {
    for (unsigned short i = str.length(); i < len; i++)
        str += to_fixed_symbol;
    return str;
}

std::vector<std::string> from_ch(std::string str) {
    std::string str_delim(1, delim);
    std::vector<std::string> res;
    boost::split(res, str, boost::is_any_of(str_delim), boost::token_compress_on);
    return res;
}

std::string to_ch(std::vector<const char*> vec, unsigned short len) {
    std::vector<char*> non_const_vec;
    for (unsigned short i = 0; i < vec.size(); i++) {
        const char* tmp = vec[i];
        char* non_const_tmp = const_cast<char*>(tmp);
        non_const_vec.push_back(non_const_tmp);
    }
    std::string str_delim(1, delim);
    std::stringstream ss;
    std::copy(non_const_vec.begin(), non_const_vec.end(), std::ostream_iterator<char*>(ss, str_delim.c_str()));
    std::string str = ss.str();
    str = to_fixed_length(str, len);
    return str;
}

void delete_from_array(char* arr, unsigned short pos) {
    for (unsigned short i = pos; i < 255; i++) {
        arr[i] = arr[i + 1];
    }
}

void recv_thread(AEScon server_con) {
    AEScon client_con(server_con);
    client_con.AES_key = "";
    SOCKET sock = server_con.sock;
    while (true) {
        std::string data = mp_recv(sock);
        if (data == "marko")
            mp_send(sock, "polo");
        else if (data == "message") {
            std::string nickname_from = mp_recv(server_con.sock);
            if (g_keys.find(nickname_from) != g_keys.end())
                server_con.AES_send("Success");
            else {
                server_con.AES_send("key");
                auto p_arr = get_p_arr();
                auto private_keys = get_private_key_arr();
                auto my_shared_keys = get_shared_key_arr(p_arr, private_keys);
                mp_send(sock, to_set(my_shared_keys));
                std::string shared_keys_set = mp_recv(sock);
                auto shared_keys = from_set(shared_keys_set);
                client_con.AES_key = get_AES_key(p_arr, private_keys, shared_keys);
            }
            std::string msg = client_con.AES_recv();
            std::cout << '\n' << nickname_from << ": " << msg << '\n' << std::flush;
        }
        else
            if (data == "key") {
                std::string nickname_from = server_con.AES_recv();
                std::string p_set = mp_recv(sock);
                std::string shared_keys_set = mp_recv(sock);
                auto p_arr = from_set(p_set);
                auto shared_keys = from_set(shared_keys_set);
                auto private_keys = get_private_key_arr();
                auto my_shared_keys = get_shared_key_arr(p_arr, private_keys);
                mp_send(sock, to_set(my_shared_keys));
                g_keys[nickname_from] = get_AES_key(p_arr, private_keys, shared_keys);
            }
    }
}

std::array<int, 2> get_ports(std::string nickname, std::string ip, int UDP_port) {
    long n{};
    sockaddr_in UDP_serv_addr;
    InetPton(AF_INET, toPCW(ip), &UDP_serv_addr.sin_addr.s_addr);
    UDP_serv_addr.sin_port = htons(UDP_port);
    UDP_serv_addr.sin_family = AF_INET;

    SOCKET UDP_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (UDP_sock == -1) {
        error("ERROR opening socket get_ports");
    }

    nickname = nickname + ';';
    n = sendto(UDP_sock, nickname.c_str(), nickname.length(), 0, (sockaddr*)&UDP_serv_addr, sizeof(UDP_serv_addr));
    if (n == -1) {
        error("ERROR sending request for ports get_ports");
    }

    char raw_ports[16];
    socklen_t UDP_serv_addr_size = sizeof(UDP_serv_addr);
    n = recvfrom(UDP_sock, raw_ports, sizeof(raw_ports), 0, (sockaddr*)&UDP_serv_addr, &UDP_serv_addr_size);
    if (n == -1) {
        error("ERROR recieving ports get_ports");
    }
    std::string str_ports = raw_ports;
    std::vector<std::string> vec_ports = from_ch(str_ports);
    std::array<int, 2> arr_ports{std::stoi(vec_ports[0]), std::stoi(vec_ports[1])};

    return arr_ports;
}

SOCKET connect_to_TCP(int port, std::string ip) {
    long n{};
    sockaddr_in sock_addr;
    InetPton(AF_INET, toPCW(ip), &sock_addr.sin_addr.s_addr);
    sock_addr.sin_port = port;
    sock_addr.sin_family = AF_INET;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1) {
        error("ERROR opening socket connect_to_TCP");
    }

    socklen_t size = sizeof(sock_addr);
    n = connect(sock, (sockaddr*)&sock_addr, size);
    if (n == -1) {
        error("ERROR connecting to server connect_to_TCP");
    }
    std::cout << "\nsend thread connected to server" << std::flush;
    return sock;
}

void shut_down(SOCKET sock) {
    const char* sd_cmd = "/shut down";
    send(sock, sd_cmd, sizeof(sd_cmd), 0);
    shutdown(sock, 2);
    closesocket(sock);
}

bool send_message(AEScon server_con, std::string nickname, std::string message) {
    AEScon client_con(server_con);
    if (g_keys.find(nickname) == g_keys.end()) {
        server_con.AES_send("key");
        server_con.AES_send(nickname);
        std::string response = server_con.AES_recv();
        if (response == "Error") {
            std::cout << server_con.AES_recv() << '\n' << std::flush;
            return false;
        }
        auto p_arr = get_p_arr();
        std::string p_set = to_set(p_arr);
        auto private_keys = get_private_key_arr();
        std::string my_shared_keys_set = to_set(get_shared_key_arr(p_arr, private_keys));
        mp_send(server_con.sock, p_set);
        mp_send(server_con.sock, my_shared_keys_set);
        std::string shared_keys_set = mp_recv(server_con.sock);
        auto shared_keys = from_set(shared_keys_set);
        g_keys[nickname] = get_AES_key(p_arr, private_keys, shared_keys);
    }
    client_con.AES_key = g_keys[nickname];
    server_con.AES_send("message");
    std::string response = server_con.AES_recv();
    if (response == "key") {
        auto p_arr = get_p_arr();
        std::string p_set = to_set(p_arr);
        auto private_keys = get_private_key_arr();
        std::string my_shared_keys_set = to_set(get_shared_key_arr(p_arr, private_keys));
        mp_send(server_con.sock, p_set);
        mp_send(server_con.sock, my_shared_keys_set);
        std::string shared_keys_set = mp_recv(server_con.sock);
        auto shared_keys = from_set(shared_keys_set);
        g_keys[nickname] = get_AES_key(p_arr, private_keys, shared_keys);
    }
    client_con.AES_key = g_keys[nickname];
    if (!server_con.AES_send(nickname))
        error("ERROR sending nickname send_message");
    response = server_con.AES_recv();
    if (response == "Error") {
        std::cout << server_con.AES_recv() << '\n' << std::flush;
        return false;
    } else
    if (response == "Success") {
        if(!client_con.AES_send(message))
            error("ERROR sending message send_message");
        response = server_con.AES_recv();
        if (response == "Success")
            return true;
        else if (response == "Error") {
            std::cout << server_con.AES_recv() << '\n' << std::flush;
            return false;
        }
    }
    else {
        std::cout << "Invalid server response\n" << std::flush;
        exit(1);
    }
    return false;
}

AEScon login(std::string ip, AEScon connection, std::string nickname, std::string password) {
    // handshake

    std::string response;
    connection.AES_send(nickname);
    response = connection.AES_recv();
    if (response != "Success") {
        std::cout << response << '\n';
        exit(1);
    }
    std::string passwd_hash = md5(password);
    std::string challenge1 = md5(md5(password) + connection.AES_key);
    connection.AES_send(challenge1);
    response = connection.AES_recv();
    if (response != "Success") {
        std::cout << response << '\n';
        exit(1);
    }
    std::string valid_challenge2 = md5(md5(md5(password)) + connection.AES_key);
    std::string challenge2 = connection.AES_recv();
    if (challenge2 != valid_challenge2) {
        connection.AES_send("Handshake failed");
        std::cout << "Handshake failed\n";
        exit(1);
    }
    connection.AES_send("Success");

    // handshake end

    connection.AES_send(password);
    response = connection.AES_recv();
    if (response != "Success") {
        std::cout << response << '\n';
        exit(1);
    }

    int l_port = std::stoi(connection.AES_recv());
    sockaddr_in l_addr;
    inet_pton(AF_INET, ip.c_str(), &l_addr.sin_addr.s_addr);
    l_addr.sin_port = l_port;
    l_addr.sin_family = AF_INET;
    socklen_t l_addr_len = sizeof(l_addr);
    SOCKET l_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    connect(l_sock, (sockaddr*)&l_addr, l_addr_len);

    AEScon l_con(l_sock);
    l_con.request();
    return l_con;
}

AEScon sign_up_con(std::string ip, int port) {
    sockaddr_in udp_addr;
    inet_pton(AF_INET, ip.c_str(), &udp_addr.sin_addr.s_addr);
    udp_addr.sin_port = htons(port);
    udp_addr.sin_family = AF_INET;
    SOCKET udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    mp_sendto(udp_sock, "sign_up", udp_addr);
    int tcp_port = std::stoi(mp_recvfrom(udp_sock, (sockaddr*)&udp_addr));
    
    sockaddr_in tcp_addr;
    inet_pton(AF_INET, ip.c_str(), &tcp_addr.sin_addr.s_addr);
    tcp_addr.sin_port = tcp_port;
    tcp_addr.sin_family = AF_INET;
    socklen_t addr_len = sizeof(tcp_addr);
    SOCKET tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    connect(tcp_sock, (sockaddr*)&tcp_addr, addr_len);

    std::string raw_public_key = mp_recv(tcp_sock);
    std::string raw_n = mp_recv(tcp_sock);
    BigUnsigned public_rsa_key = stringToBigUnsigned(raw_public_key);
    BigUnsigned n = stringToBigUnsigned(raw_n);
    std::array<BigUnsigned, 4> raw_arr = RSA::get_arr(n);
    std::array<BigUnsigned, 4> enc_arr = RSA::encrypt(raw_arr, public_rsa_key, n);
    std::string rsa_set = RSA::to_set(enc_arr);
    mp_send(tcp_sock, rsa_set);

    std::string AES_key = RSA::get_AES_key(raw_arr);
    AEScon connection(tcp_sock);
    connection.AES_key = AES_key;
    
    return connection;
}

std::string sign_up(AEScon connection, std::string nickname, std::string passwd) {
    while (true) {
        connection.AES_send(nickname);
        connection.AES_send(passwd);
        std::string response = connection.AES_recv();
        return response;
    }
}
}
