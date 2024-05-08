#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <winsock2.h>
#include "Client.h"
#include <fstream>
#include <cstdio>
//TODO CHANGE CHAR TO STRING EVERYWHERE!!!!!!!!!!! Exit command, different check of format message, registration with checking differences of names, broadcast about new user, set name command

std::vector<Client> clients;
std::mutex clientsMutex;

std::string find_name_from_socket(SOCKET clientSocket)
{
    for (long long i=0; i<clients.size(); i++)
    {
        if (clients[i].socket==clientSocket)
            return clients[i].name;
    }
    return "Unknown error in name of sender";
}

Client* find_client_from_socket(SOCKET clientSocket)
{
    for (int i=0; i<clients.size(); i++)
    {
        if (clients[i].socket==clientSocket)
        {
            return (&clients[i]);
        }
    }
}

Client* find_client_from_email(const std::string& email_user)
{
    for (int i=0; i<clients.size(); i++)
    {
        if (clients[i].email==email_user)
        {
            return (&clients[i]);
        }
    }
}

Client* find_client_from_name(const std::string& name_user)
{
    for (int i=0; i<clients.size(); i++)
    {
        if (clients[i].name==name_user)
        {
            return (&clients[i]);
        }
    }
}

Client* find_client_from_password(const std::string& password_user)
{
    for (int i=0; i<clients.size(); i++)
    {
        if (clients[i].password==password_user)
        {
            return (&clients[i]);
        }
    }
}

void exit(SOCKET clientSocket)
{
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (it->socket == clientSocket) {
            clients.erase(it);
            break;
        }
    }
}

void set_name(SOCKET clientSocket, const std::string& text, long long i)
{
    std::string user_name;
    while(text[i]!='\0')
    {
        user_name+=text[i];
        i++;
    }
}

void set_email(SOCKET clientSocket, const std::string& text, long long i)
{
    Client* current_client= find_client_from_socket(clientSocket);
    std::string user_email;
    while(text[i]!='\0')
    {
        user_email+=text[i];
        i++;
    }
    std::string line;
    std::ifstream in(R"(C:\Users\Public\include\emails.txt)");
    int key=1;
    if(in.is_open())
    {
        while(std::getline(in,line))
        {
            std::cout<<"was opened email1";
            //key=1;
            if (line==user_email)
            {
                key=0;
                std::string err="Данный email уже используется. Войдите в свой аккаунт или зарегистрируйтесь под новым email.";
                send(clientSocket, err.c_str(), err.size(), 0);
                break;
            }
        }
    }
    else
    {
        std::cout<<"Не получилось открыть файл emails.txt";
    }
    in.close();
    if (current_client->email=="email")
    {
        if (key)
        {
            std::cout<<"was there 2";
            std::ofstream out(R"(C:\Users\Public\include\emails.txt)", std::ios::app);
            if (out.is_open())
            {
                std::cout<<"was there 3";
                out<<user_email<<std::endl;
            }
            out.close();
            std::ofstream out1(R"(C:\Users\Public\include\data_clients.txt)", std::ios::app);
            if (out1.is_open())
            {
                std::cout<<"was there 4";
                out1<<user_email<<":password:noname"<<std::endl;
            }
            out1.close();
            std::cout<<"was there 5";
            current_client->email=user_email;
        }
    }
    else
    {
        if(key)
        {
            std::cout<<"wassssssss";
            std::ifstream in1(R"(C:\Users\Public\include\data_clients.txt)");
            std::ofstream out2(R"(C:\Users\Public\include\temporary_data_clients.txt)");
            long long position=0;
            //std::lock_guard<std::mutex> lock(clientsMutex);
            if(in1.is_open() && out2.is_open())
            {
                while(std::getline(in1,line))
                {
                    std::cout<<"lego";
                    if(line.find(current_client->email)!=std::string::npos)
                    {
                        out2<<user_email<<":"<<current_client->password<<":"<<current_client->name<<std::endl;
                        std::cout<<"sheeesh";
                    }
                    else
                    {
                        out2<<line<<std::endl;
                        std::cout<<"ppppppp";
                    }
                    //position++;
                }
            }
            std::cout<<rename(R"(C:\Users\Public\include\data_clients.txt)", R"(C:\Users\Public\include\popa.txt)");
            rename(R"(C:\Users\Public\include\temporary_data_clients.txt)", R"(C:\Users\Public\include\data_clients.txt)");
            rename(R"(C:\Users\Public\include\popa.txt)", R"(C:\Users\Public\include\temporary_data_clients.txt)");
            in1.close();
            out2.close();
            current_client->email=user_email;
            //TODO CHANGE EMAILS FILE and check about rename
        }
    }
}

void set_password(SOCKET clientSocket, const std::string& text, long long i)
{
    std::string user_password;
    while(text[i]!='\0')
    {
        user_password+=text[i];
        i++;
    }
}

void registration(SOCKET clientSocket)
{

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
        send(clients[i].socket, buff.c_str(), buff.size(), 0);
    }
}

void Group_message(SOCKET clientSocket, const std::string& mes, const std::vector<std::string>& send_to_names)
{
    std::vector<SOCKET> socket_send=find_socket_from_name(send_to_names);
    std::string name_sender= find_name_from_socket(clientSocket);
    std::string buff="Сообщение от: "+name_sender+"\n";
    buff+=mes;
    for (int i=0; i<socket_send.size(); i++)
        send(socket_send[i], buff.c_str(), buff.size(), 0);
}

void Private_message(SOCKET clientSocket, const std::string& mes, const std::vector<std::string>& send_to_names)
{
    if (send_to_names.empty() || send_to_names.size()>1)
    {
        std::string buff="Вы ввели команду pravate, однако кол-во людей кому вы хотите отправить не равно 1\n";
        send(clientSocket, buff.c_str(), buff.size(), 0);
    }
    else
    {
        std::string name_sender= find_name_from_socket(clientSocket);
        std::string buff="Сообщение от: "+name_sender+"\n";
        buff+=mes;
        std::vector<SOCKET> socket_send=find_socket_from_name(send_to_names);
        send(socket_send[0], buff.c_str(), buff.size(), 0);
    }
}



void Reformat_message(SOCKET clientSocket, const std::string& text, std::string& where, std::vector<std::string>& send_to_names, std::string& mes) //0-public,1-private,group
{
    long long i=0;
    while(text[i]!=':')
    {
        where+=text[i];
        i++;
    }
    i++;
    int key=1;
    if(where=="public")
        key=0;
    else if(where=="set_name")
    {
        set_name(clientSocket, text, i);
        return;
    }
    else if(where=="set_email")
    {
        set_email(clientSocket, text, i);
        return;
    }
    else if(where=="set_password")
    {
        set_password(clientSocket, text, i);
        return;
    }
    if (key)
    {
        while(text[i]!=':')
        {
            std::string temporary_name;
            while(text[i]!=',' && text[i]!=':')
            {
                temporary_name+=text[i];
                i++;
            }
            send_to_names.push_back(temporary_name);
            if(text[i]!=':')
                i++;
        }
        i++;
    }
    while(text[i]!='\0')
    {
        mes+=text[i];
        i++;
    }
}


void clientHandler(SOCKET clientSocket) {
    // Обработка клиента
    // Пример чтения сообщений от клиента и отправки ответов
    std::string buffer;
    char buff[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buff, sizeof(buff), 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            std::cout<<"SEND_MESSAGE_ERROR";
            break;
        }
        buffer=buff;
        if (buffer=="exit")
        {
            exit(clientSocket);
            continue;
        }
        else if (buffer=="registartion")
        {
            registration(clientSocket);
            continue;
        }
        buffer[bytesReceived] = '\0'; // Добавляем завершающий символ строки
        buff[bytesReceived]='\0';
        std::string where="";
        std::vector<std::string> send_to_names;
        std::string mes="";
        Reformat_message(clientSocket, buffer, where, send_to_names, mes);
        if (where=="public")
        {
            Broadcast(clientSocket, mes);
        }
        else if(where=="group")
        {
            Group_message(clientSocket, mes, send_to_names);
        }
        else if(where=="private")
        {
            Private_message(clientSocket, mes, send_to_names);
        }
        else if (where=="set_name" || where=="set_email" || where=="set_password")
            continue;
        else
        {
            std::string err="Неправильный формат данных";
            send(clientSocket, err.c_str(), err.size(), 0);
            std::cout<<"Wrong format";
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
        std::string err="Войдите в аккаунт или зарегистрируйтесь";
        send(clientSocket, err.c_str(), err.size(), 0);
//        for(int i=0; i<clients.size(); i++)
//            std::cout<<clients[i].email<<":"<<clients[i].password<<":"<<clients[i].name<<"\n";
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


