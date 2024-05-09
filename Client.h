#include <iostream>
#include <vector>
#include <winsock2.h>

#pragma once

class Client{
public:
    Client();
    void print() const;
    int id;
    std::string name;
    std::string email;
    std::string password;
    SOCKET socket;
private:
    static int id_count;
};
