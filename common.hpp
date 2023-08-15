#pragma once
#include <map>

extern std::map<std::string, std::string> g_keys;
struct cfg_struct {
    std::string ip;
    int UDP_port{};
};
