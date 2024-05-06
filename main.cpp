#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <winsock2.h>
#include "Client.h"

//TODO CHANGE CHAR TO STRING EVERYWHERE!!!!!!!!!!! Exit command, different check of format message, registartion with checking differences of names

std::vector<Client> clients;
std::mutex clientsMutex;

std::string find_name_from_socket(SOCKET clientSocket)
{
    for (int i=0; i<clients.size(); i++)
    {
        if (clients[i].socket==clientSocket)
            return clients[i].name;
    }
    return "Unknown error in name of sender";
}


std::vector<SOCKET> find_socket_from_name(const std::vector<std::string>& send_to_names)
{
    std::vector<SOCKET> answer;
    for (int j=0; j<send_to_names.size(); j++)
    {
        for (int i=0; i<clients.size(); i++)
        {
            if (clients[i].name==send_to_names[i])
            {
                answer.push_back(clients[i].socket);
                break;
            }
        }
    }
    return answer;
}

void Broadcast(SOCKET clientSocket, const std::string& mes)
{
    std::string name_sender= find_name_from_socket(clientSocket);
    std::string buff="Сообщение от: "+name_sender+"\n";
    buff+=mes;
    for (int i=0; i<clients.size(); i++)
    {
        if (clients[i].socket==clientSocket)
            continue;
        send(clients[i].socket, &(buff[0]), buff.length(), 0);
    }
}

void Group_message(SOCKET clientSocket, const std::string& mes, const std::vector<std::string>& send_to_names)
{
    std::vector<SOCKET> socket_send=find_socket_from_name(send_to_names);
    std::string name_sender= find_name_from_socket(clientSocket);
    std::string buff="Сообщение от: "+name_sender+"\n";
    buff+=mes;
    for (int i=0; i<socket_send.size(); i++)
        send(socket_send[i], &(buff[0]), buff.length(), 0);
}

void Private_message(SOCKET clientSocket, const std::string& mes, const std::vector<std::string>& send_to_names)
{
    if (send_to_names.size()==0 || send_to_names.size()>1)
    {
        std::string buff="Вы ввели команду pravate, однако кол-во людей кому вы хотите отправить не равно 1\n";
        send(clientSocket, &(buff[0]), buff.length(), 0);
    }
    else
    {
        std::string name_sender= find_name_from_socket(clientSocket);
        std::string buff="Сообщение от: "+name_sender+"\n";
        buff+=mes;
        std::vector<SOCKET> socket_send=find_socket_from_name(send_to_names);
        send(socket_send[0], &(buff[0]), buff.length(), 0);
    }
}



void Reformat_message(const std::string& text, std::string& where, std::vector<std::string>& send_to_names, std::string& mes)
{
    long long i=0;
    while(text[i]!=':')
    {
        where+=text[i];
        i++;
    }
    i++;
    while(text[i]!=':')
    {
        std::string temporary_name;
        while(text[i]!=',')
        {
            temporary_name+=text[i];
            i++;
        }
        send_to_names.push_back(temporary_name);
        i++;
    }
    i++;
    while(text[i]!='\0')
    {
        mes+=text[i];
        i++;
    }
}

//Формат сообщения:
//куда(public, private, group); кому(имя,имя,...); сообщение
//"public:dasha,masha,sasha:Hello"

void clientHandler(SOCKET clientSocket) {
    // Обработка клиента
    // Пример чтения сообщений от клиента и отправки ответов
    std::string buffer;
    while (true) {
        int bytesReceived = recv(clientSocket, &(buffer[0]), buffer.length(), 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            break;
        }
        buffer[bytesReceived] = '\0'; // Добавляем завершающий символ строки
        std::string where="";
        std::vector<std::string> send_to_names;
        std::string mes="";
        Reformat_message(buffer, where, send_to_names, mes);
        if (where=="public")
        {
            Broadcast(clientSocket, mes);
        }
        else if(where=="group")
        {
            Group_message(clientSocket, mes, send_to_names);
        }
        else
        {
            Private_message(clientSocket, mes, send_to_names);
        }
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


