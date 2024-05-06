#include <iostream>
#include <vector>
#include <winsock2.h>
#include "Client.h"
#pragma once

int Client::id_count=0;

Client::Client() {
    id=id_count++;
    name="noname";
}
