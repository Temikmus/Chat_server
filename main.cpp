#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <winsock2.h>

std::vector<SOCKET> clients;
std::mutex clientsMutex;

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
        std::cout << "Сообщение от клиента: " << buffer << std::endl;

        // Отправка ответа клиенту (здесь можно добавить логику обработки сообщения)
        // send(clientSocket, response, strlen(response), 0);
    }

    // Удаляем клиента из списка после завершения соединения
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
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(clientSocket);
        }

        // Запуск обработчика клиента в отдельном потоке
        std::thread clientThread(clientHandler, clientSocket);
        clientThread.detach(); // Отсоединяем поток, чтобы не ждать его завершения
    }

    // Закрытие сокета сервера и завершение работы Winsock
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}


