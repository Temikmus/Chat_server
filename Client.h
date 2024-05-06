#include <iostream>
#include <vector>
#include <winsock2.h>

#pragma once

class Client{
public:
    Client();
    int id;
    std::string name;
    SOCKET socket;
private:
    static int id_count;
};
