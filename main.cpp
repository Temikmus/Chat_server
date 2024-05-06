#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <winsock2.h>
#include "Client.h"

std::vector<Client> clients;
std::mutex clientsMutex;

std::vector<SOCKET> find_id(const std::vector<Client>& clients1, int id)
{
    std::vector<SOCKET> answer;
    for (int i=0; i<clients1.size(); i++)
    {
        if (clients1[i].id==id)
            answer.push_back(clients1[i].socket);
    }
    return answer;
}

void clientHandler(SOCKET clientSocket) {
    // Обработка клиента
    // Пример чтения сообщений от клиента и отправки ответов
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            break;
        }
        buffer[bytesReceived] = '\0'; // Добавляем завершающий символ строки
        int id_send=buffer[0]-'0';
        std::cout<<buffer[0]<<" ";
        std::cout<<id_send<<"\n";
        std::cout << "Сообщение от клиента: " << buffer << std::endl;
        std::vector<SOCKET> answer= find_id(clients, id_send);
        //buffer[0]=' ';
        std::cout<<"len_answer="<<answer.size();
        std::string message=buffer;
        for (int i=0; i<answer.size(); i++)
        {
            std::cout<<"send_socket:"<<answer[i]<<"\n";
            //std::cout<<"send:  "<<send(answer[i], buffer, sizeof(buffer), 0);
            std::cout<<"send:  "<<send(answer[i], &(message[0]), message.length(), 0);
        }
        // Отправка ответа клиенту (здесь можно добавить логику обработки сообщения)
        //send(clientSocket, buffer, strlen(buffer), 0);
    }

    // Удаляем клиента из списка после завершения соединения
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            if (it->socket == clientSocket) {
                clients.erase(it);
                break;
            }
        }
    }

    closesocket(clientSocket);
}

int main() {
    // Инициализация Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Создание сокета сервера
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Настройка адреса сервера
    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(8080);

    // Привязка сокета к адресу сервера
    bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

    // Прослушивание подключений
    listen(serverSocket, SOMAXCONN);

    std::cout << "Сервер запущен. Ожидание подключений...\n";

    while (true) {
        // Принятие нового подключения
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        // Добавление клиента в список
        std::lock_guard<std::mutex> lock(clientsMutex);
        Client temporary_client;
        temporary_client.socket=clientSocket;
        clients.push_back(temporary_client);
        std::cout<<"Подключился новый клиент. ID:"<<clients[clients.size()-1].id<<"\n";
        std::cout<<"Socket:"<<clientSocket<<"\n";
        // Запуск обработчика клиента в отдельном потоке
        std::thread clientThread(clientHandler, clientSocket);
        clientThread.detach(); // Отсоединяем поток, чтобы не ждать его завершения
    }

    // Закрытие сокета сервера и завершение работы Winsock
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}


