#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <winsock2.h>

std::vector<SOCKET> clients;
std::mutex clientsMutex;

void clientHandler(SOCKET clientSocket) {
    // ��ࠡ�⪠ ������
    // �ਬ�� �⥭�� ᮮ�饭�� �� ������ � ��ࠢ�� �⢥⮢
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            break;
        }
        buffer[bytesReceived] = '\0'; // ������塞 �������騩 ᨬ��� ��ப�
        std::cout << "����饭�� �� ������: " << buffer << std::endl;

        // ��ࠢ�� �⢥� ������� (����� ����� �������� ������ ��ࠡ�⪨ ᮮ�饭��)
        // send(clientSocket, response, strlen(response), 0);
    }

    // ����塞 ������ �� ᯨ᪠ ��᫥ �����襭�� ᮥ�������
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            if (*it == clientSocket) {
                clients.erase(it);
                break;
            }
        }
    }

    closesocket(clientSocket);
}

int main() {
    // ���樠������ Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // �������� ᮪�� �ࢥ�
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // ����ன�� ���� �ࢥ�
    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(8080);

    // �ਢ離� ᮪�� � ����� �ࢥ�
    bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

    // ����訢���� ������祭��
    listen(serverSocket, SOMAXCONN);

    std::cout << "��ࢥ� ����饭. �������� ������祭��...\n";

    while (true) {
        // �ਭ�⨥ ������ ������祭��
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);

        // ���������� ������ � ᯨ᮪
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(clientSocket);
        }

        // ����� ��ࠡ��稪� ������ � �⤥�쭮� ��⮪�
        std::thread clientThread(clientHandler, clientSocket);
        clientThread.detach(); // ��ᮥ���塞 ��⮪, �⮡� �� ����� ��� �����襭��
    }

    // �����⨥ ᮪�� �ࢥ� � �����襭�� ࠡ��� Winsock
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}


