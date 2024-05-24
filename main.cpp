#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <winsock2.h>
#include "Client.h"
#include <fstream>
#include <filesystem>
#include <cstdio>
#include <algorithm>

//TODO CHANGE CHAR TO STRING EVERYWHERE!!!!!!!!!!! check format of email, ������� ����� ᮮ�饭�� �᫨ �� ��諨 � ���, �᫨ ��� ⠪��� ������ � group � �����, �㭪�� show_online, ����� ᮮ�饭��

namespace fs = std::filesystem;

std::vector<Client> clients;
std::mutex clientsMutex;

void show_history(SOCKET clientSocket, const std::vector<std::string>& filenames)
{
    std::string directory = "C:\\Users\\Public\\include";
    std::string fullPath;
    for(int i=0; i<filenames.size(); i++)
    {
        fullPath = directory + "\\" + filenames[i];
        if (fs::exists(fullPath)) {
            std::string line;
            std::ifstream in(fullPath);
            if(in.is_open())
            {
                int i=0;
                while(std::getline(in,line))
                {
                    i++;
                    if(i%2==0)
                        line+="\n";
                    send(clientSocket, line.c_str(), line.size(), 0);
                }
            }
            return;
        }
    }
    std::string err="������ �� �� �������.";
    send(clientSocket, err.c_str(), err.size(), 0);
}

void delete_history(SOCKET clientSocket, const std::vector<std::string>& filenames)
{
    std::string directory = "C:\\Users\\Public\\include";
    for (const auto& filename : filenames) {
        std::string fullPath = directory + "\\" + filename;
        if (fs::exists(fullPath)) {
            try {
                fs::remove(fullPath);
                std::cout << "Deleted file: " << fullPath << std::endl;
                std::string err="����� �� �ᯥ譮 ��饭�!";
                send(clientSocket, err.c_str(), err.size(), 0);
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to delete file: " << fullPath << " - " << e.what() << std::endl;
                std::string err="�� ����稫��� ������ �����.";
                send(clientSocket, err.c_str(), err.size(), 0);
            }
            return;
        }
    }
    std::string err="������ �� �� �������.";
    send(clientSocket, err.c_str(), err.size(), 0);
}

void show_online(SOCKET clientSocket)
{
    std::string buff="� ����� ������ �� �ࢥ�: ";
    for (const auto & client : clients)
    {
        if(!(client.socket==clientSocket || client.name=="noname"))
            buff=buff+client.name+" ";
    }
    send(clientSocket, buff.c_str(), buff.size(), 0);
}

void History_of_chats(const std::string& message, const std::vector<std::string>& filenames) {
    // ��।��塞 ���� � ��४�ਨ � ������ ��� 䠩��
    std::string directory = "C:\\Users\\Public\\include";
    std::string fullPath;
    for(int i=0; i<filenames.size(); i++)
    {
        fullPath = directory + "\\" + filenames[i];
        if (fs::exists(fullPath)) {
            // ���� �������, ���뢠�� ��� � ०��� ����������
            std::ofstream outFile(fullPath, std::ios::app);
            if (outFile.is_open()) {
                outFile << message << std::endl;
                std::cout << "Message appended to the file." << std::endl;
            } else {
                std::cerr << "Failed to open the file for appending." << std::endl;
            }
            return;
        }
    }
    // ���� �� �������, ᮧ���� ���� 䠩� � �����뢠�� ᮮ�饭��
    std::ofstream outFile(fullPath);
    if (outFile.is_open()) {
        outFile << message << std::endl;
        std::cout << "File created and message written." << std::endl;
    } else {
        std::cerr << "Failed to create the file." << std::endl;
    }
}

// �㭪�� ��� ࠧ������� ��ப� �� ��������� ᨬ����-ࠧ����⥫�
std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    for (char ch : str) {
        if (ch == delimiter) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += ch;
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

// �㭪�� ��� �����樨 ��� ����⠭���� ����
std::vector<std::string> generatePermutations(const std::string& input) {
    std::vector<std::string> names = split(input, '%');
    std::vector<std::string> permutations;

    // ������� ��� ��������� ����⠭����
    std::sort(names.begin(), names.end());
    do {
        std::string permutedString;
        for (size_t i = 0; i < names.size(); ++i) {
            permutedString += names[i];
            if (i != names.size() - 1) {
                permutedString += '%';
            }
        }
        permutations.push_back(permutedString);
    } while (std::next_permutation(names.begin(), names.end()));

    return permutations;
}

void print_users()
{
    for (const auto & client : clients)
    {
        client.print();
    }
}

std::string extract_name_from_string(const std::string& text, long long i)
{
    std::string user_name;
    while(text[i]!='\0')
    {
        user_name+=text[i];
        i++;
    }
    std::cout<<user_name;
    return user_name;
}

std::string find_name_from_socket(SOCKET clientSocket)
{
    for (auto & client : clients)
    {
        if (client.socket==clientSocket)
            return client.name;
    }
    return "Unknown error in name of sender";
}

Client* find_client_from_socket(SOCKET clientSocket)
{
    for (auto & client : clients)
    {
        if (client.socket==clientSocket)
        {
            return (&client);
        }
    }
}

void end_registration(SOCKET clientSocket)
{
    Client* current_client= find_client_from_socket(clientSocket);
    if (current_client->email=="email" || current_client->password=="password" || current_client->name=="noname")
    {
        std::string buff="��� ����砭�� ॣ����樨 ������ �� �����.";
        send(clientSocket, buff.c_str(), buff.size(), 0);
        return;
    }
    std::string mes=current_client->name+" ��ॣ����஢���� � ������稫�� � �ࢥ��." ;
    std::string buff="�� �ᯥ譮 ��ॣ����஢�����! ���� ���������� "+current_client->name;
    send(clientSocket, buff.c_str(), buff.size(), 0);
    show_online(clientSocket);
    for (auto & client : clients)
    {
        if (client.socket==clientSocket)
            continue;
        if (client.email=="email" || client.password=="password" || client.name=="noname")
            continue;
        send(client.socket, mes.c_str(), mes.size(), 0);
    }
}

void end_entry(SOCKET clientSocket)
{
    Client* current_client= find_client_from_socket(clientSocket);
    std::string mes=current_client->name+" ������稫�� � �ࢥ��." ;
    std::string buff="�� �ᯥ譮 ��諨 � ᢮� ��䨫�! � �����饭��� "+current_client->name+"!)";
    send(clientSocket, buff.c_str(), buff.size(), 0);
    show_online(clientSocket);
    for (auto & client : clients)
    {
        if (client.socket==clientSocket)
            continue;
        if (client.email=="email" || client.password=="password" || client.name=="noname")
            continue;
        send(client.socket, mes.c_str(), mes.size(), 0);
    }
}

Client* find_client_from_email(const std::string& email_user)
{
    for (auto & client : clients)
    {
        if (client.email==email_user)
        {
            return (&client);
        }
    }
}

Client* find_client_from_name(const std::string& name_user)
{
    for (auto & client : clients)
    {
        if (client.name==name_user)
        {
            return (&client);
        }
    }
}

Client* find_client_from_password(const std::string& password_user)
{
    for (auto & client : clients)
    {
        if (client.password==password_user)
        {
            return (&client);
        }
    }
}

void exit(SOCKET clientSocket)
{
    std::string user_name= find_name_from_socket(clientSocket);
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (it->socket == clientSocket) {
            clients.erase(it);
            break;
        }
    }
    for (auto & client : clients)
    {
        if (client.email=="email" || client.password=="password" || client.name=="noname")
            continue;
        std::string mes= "���짮��⥫� " +user_name+" ��襫.";
        send(client.socket, mes.c_str(), mes.size(), 0);
    }
    closesocket(clientSocket);
}

void Broadcast(SOCKET clientSocket, const std::string& mes)
{
    std::string name_sender= find_name_from_socket(clientSocket);
    std::string buff="Public chat. "+name_sender+":\n";
    buff+=mes;
    for (auto & client : clients)
    {
        if (client.socket==clientSocket)
            continue;
        if (client.email=="email" || client.password=="password" || client.name=="noname")
            continue;
        send(client.socket, buff.c_str(), buff.size(), 0);
    }
    std::vector<std::string> filenames;
    filenames.push_back("public");
    History_of_chats(buff, filenames);
}

void set_name(SOCKET clientSocket, const std::string& text, long long i)
{
    Client* current_client= find_client_from_socket(clientSocket);
    std::string user_name;
    while(text[i]!='\0')
    {
        user_name+=text[i];
        i++;
    }
    if(user_name.empty())
    {
        std::string buff="���ࠢ���� �ଠ� ������";
        send(clientSocket, buff.c_str(), buff.size(), 0);
        return;
    }
    if(user_name.find(':')!=std::string::npos)
    {
        std::string err="����饭� �ᯮ�짮���� ':' � �����.";
        send(clientSocket, err.c_str(), err.size(), 0);
        return;
    }
    if (user_name=="noname")
    {
        std::string err="������ ��� ����饭� �ᯮ�짮����. ���஡�� ��㣮�.";
        send(clientSocket, err.c_str(), err.size(), 0);
        return;
    }
    std::string line;
    std::ifstream in(R"(C:\Users\Public\include\names.txt)");
    int key=1;
    if(in.is_open())
    {
        while(std::getline(in,line))
        {
            //key=1;
            if (line==user_name)
            {
                key=0;
                std::string err="������ ��� 㦥 �ᯮ������. ���஡�� ��㣮�.";
                send(clientSocket, err.c_str(), err.size(), 0);
                return;
            }
        }
    }
    else
    {
        std::cout<<"�� ����稫��� ������ 䠩� names.txt";
    }
    in.close();
    if (current_client->name=="noname")
    {
        if (key)
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            std::ofstream out(R"(C:\Users\Public\include\names.txt)", std::ios::app);
            if (out.is_open())
            {
                out<<user_name<<std::endl;
            }
            out.close();
            std::ifstream in1(R"(C:\Users\Public\include\data_clients.txt)");
            std::ofstream out2(R"(C:\Users\Public\include\temporary_data_clients.txt)", std::ios::trunc);
            if(in1.is_open() && out2.is_open())
            {
                while(std::getline(in1,line))
                {
                    if(line.find(current_client->email)!=std::string::npos)
                    {
                        out2<<current_client->email<<":"<<current_client->password<<":"<<user_name<<std::endl;
                    }
                    else
                    {
                        out2<<line<<std::endl;
                    }
                }
            }
            in1.close();
            out2.close();
            rename(R"(C:\Users\Public\include\data_clients.txt)", R"(C:\Users\Public\include\popa.txt)");
            rename(R"(C:\Users\Public\include\temporary_data_clients.txt)", R"(C:\Users\Public\include\data_clients.txt)");
            rename(R"(C:\Users\Public\include\popa.txt)", R"(C:\Users\Public\include\temporary_data_clients.txt)");
            if (current_client->name!="noname")
                Broadcast(clientSocket, current_client->name+" ᬥ��� ���, ⥯��� �� "+user_name);
            current_client->name=user_name;
        }
    }
    else
    {
        if(key)
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            std::ifstream in1(R"(C:\Users\Public\include\data_clients.txt)");
            std::ofstream out2(R"(C:\Users\Public\include\temporary_data_clients.txt)", std::ios::trunc);
            if(in1.is_open() && out2.is_open())
            {
                while(std::getline(in1,line))
                {
                    if(line.find(current_client->email)!=std::string::npos)
                    {
                        out2<<current_client->email<<":"<<current_client->password<<":"<<user_name<<std::endl;
                    }
                    else
                    {
                        out2<<line<<std::endl;
                    }
                }
            }
            in1.close();
            out2.close();
            rename(R"(C:\Users\Public\include\data_clients.txt)", R"(C:\Users\Public\include\popa.txt)");
            rename(R"(C:\Users\Public\include\temporary_data_clients.txt)", R"(C:\Users\Public\include\data_clients.txt)");
            rename(R"(C:\Users\Public\include\popa.txt)", R"(C:\Users\Public\include\temporary_data_clients.txt)");
            std::ifstream in2(R"(C:\Users\Public\include\names.txt)");
            std::ofstream out3(R"(C:\Users\Public\include\temporary_names.txt)", std::ios::trunc);
            if(in2.is_open() && out3.is_open())
            {
                while(std::getline(in2,line))
                {
                    if(line.find(current_client->name)!=std::string::npos)
                    {
                        out3<<user_name<<std::endl;
                    }
                    else
                    {
                        out3<<line<<std::endl;
                    }
                }
            }
            in2.close();
            out3.close();
            rename(R"(C:\Users\Public\include\names.txt)", R"(C:\Users\Public\include\popa.txt)");
            rename(R"(C:\Users\Public\include\temporary_names.txt)", R"(C:\Users\Public\include\names.txt)");
            rename(R"(C:\Users\Public\include\popa.txt)", R"(C:\Users\Public\include\temporary_names.txt)");
            if (current_client->name!="noname")
                Broadcast(clientSocket, current_client->name+" ᬥ��� ���, ⥯��� �� "+user_name);
            current_client->name=user_name;
        }
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
    if(user_email.empty())
    {
        std::string buff="���ࠢ���� �ଠ� ������";
        send(clientSocket, buff.c_str(), buff.size(), 0);
        return;
    }
    if(user_email.find(':')!=std::string::npos)
    {
        std::string err="����饭� �ᯮ�짮���� ':' � email.";
        send(clientSocket, err.c_str(), err.size(), 0);
        return;
    }
    std::string line;
    std::ifstream in(R"(C:\Users\Public\include\emails.txt)");
    int key=1;
    if(in.is_open())
    {
        while(std::getline(in,line))
        {
            //key=1;
            if (line==user_email)
            {
                key=0;
                std::string err="����� email 㦥 �ᯮ������. ������ � ᢮� ������ ��� ��ॣ��������� ��� ���� email.";
                send(clientSocket, err.c_str(), err.size(), 0);
                break;
            }
        }
    }
    else
    {
        std::cout<<"�� ����稫��� ������ 䠩� emails.txt";
    }
    in.close();
    if (current_client->email=="email")
    {
        if (key)
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            std::ofstream out(R"(C:\Users\Public\include\emails.txt)", std::ios::app);
            if (out.is_open())
            {
                out<<user_email<<std::endl;
            }
            out.close();
            std::ofstream out1(R"(C:\Users\Public\include\data_clients.txt)", std::ios::app);
            if (out1.is_open())
            {
                out1<<user_email<<":password:noname"<<std::endl;
            }
            out1.close();
            current_client->email=user_email;
        }
    }
    else
    {
        if(key)
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            std::ifstream in1(R"(C:\Users\Public\include\data_clients.txt)");
            std::ofstream out2(R"(C:\Users\Public\include\temporary_data_clients.txt)", std::ios::trunc);
            if(in1.is_open() && out2.is_open())
            {
                while(std::getline(in1,line))
                {
                    if(line.find(current_client->email)!=std::string::npos)
                    {
                        out2<<user_email<<":"<<current_client->password<<":"<<current_client->name<<std::endl;
                    }
                    else
                    {
                        out2<<line<<std::endl;
                    }
                }
            }
            in1.close();
            out2.close();
            rename(R"(C:\Users\Public\include\data_clients.txt)", R"(C:\Users\Public\include\popa.txt)");
            rename(R"(C:\Users\Public\include\temporary_data_clients.txt)", R"(C:\Users\Public\include\data_clients.txt)");
            rename(R"(C:\Users\Public\include\popa.txt)", R"(C:\Users\Public\include\temporary_data_clients.txt)");
            std::ifstream in2(R"(C:\Users\Public\include\emails.txt)");
            std::ofstream out3(R"(C:\Users\Public\include\temporary_emails.txt)", std::ios::trunc);
            if(in2.is_open() && out3.is_open())
            {
                while(std::getline(in2,line))
                {
                    if(line.find(current_client->email)!=std::string::npos)
                    {
                        out3<<user_email<<std::endl;
                    }
                    else
                    {
                        out3<<line<<std::endl;
                    }
                }
            }
            in2.close();
            out3.close();
            rename(R"(C:\Users\Public\include\emails.txt)", R"(C:\Users\Public\include\popa.txt)");
            rename(R"(C:\Users\Public\include\temporary_emails.txt)", R"(C:\Users\Public\include\emails.txt)");
            rename(R"(C:\Users\Public\include\popa.txt)", R"(C:\Users\Public\include\temporary_emails.txt)");
            current_client->email=user_email;
        }
    }
}

void set_password(SOCKET clientSocket, const std::string& text, long long i)
{
    std::string user_password;
    while(text[i]!='\0' && i<text.size())
    {
        user_password+=text[i];
        i++;
    }
    if(user_password.empty())
    {
        std::string buff="���ࠢ���� �ଠ� ������";
        send(clientSocket, buff.c_str(), buff.size(), 0);
        return;
    }
    if(user_password.find(':')!=std::string::npos)
    {
        std::string err="����饭� �ᯮ�짮���� ':' � ��஫�.";
        send(clientSocket, err.c_str(), err.size(), 0);
        return;
    }
    if (user_password.length()<6)
    {
        std::string err="��஫� ���������. ������ �� ����� 6 ᨬ�����.";
        send(clientSocket, err.c_str(), err.size(), 0);
        return;
    }
    if (user_password=="password")
    {
        std::string err="����� ��஫� ����饭. ���஡�� ��㣮�.";
        send(clientSocket, err.c_str(), err.size(), 0);
        return;
    }
    Client* current_client= find_client_from_socket(clientSocket);
    std::string line;
    std::ifstream in1(R"(C:\Users\Public\include\data_clients.txt)");
    std::ofstream out2(R"(C:\Users\Public\include\temporary_data_clients.txt)", std::ios::trunc);
    std::lock_guard<std::mutex> lock(clientsMutex);
    if(in1.is_open() && out2.is_open())
    {
        while(std::getline(in1,line))
        {
            if(line.find(current_client->email)!=std::string::npos)
            {
                out2<<current_client->email<<":"<<user_password<<":"<<current_client->name<<std::endl;
            }
            else
            {
                out2<<line<<std::endl;
            }
        }
    }
    in1.close();
    out2.close();
    rename(R"(C:\Users\Public\include\data_clients.txt)", R"(C:\Users\Public\include\popa.txt)");
    rename(R"(C:\Users\Public\include\temporary_data_clients.txt)", R"(C:\Users\Public\include\data_clients.txt)");
    rename(R"(C:\Users\Public\include\popa.txt)", R"(C:\Users\Public\include\temporary_data_clients.txt)");
    current_client->password=user_password;
}

void sign_up(SOCKET clientSocket) //registration
{
    Client* current_client= find_client_from_socket(clientSocket);
    current_client->email="email";
    current_client->name="noname";
    current_client->password="password";
    std::string err="�������� ������ email, password � name.";
    send(clientSocket, err.c_str(), err.size(), 0);
}

int check_line_from_data(const std::string& line,const std::string& user_email, const std::string& user_password)
{
    std::string str;
    int i=0;
    while(line[i]!=':')
    {
        str+=line[i];
        i++;
    }
    i++;
    if (str!=user_email)
        return 0;
    str.clear();
    while(line[i]!=':')
    {
        str+=line[i];
        i++;
    }
    if (str!=user_password)
        return 0;
    return 1;
}

void sign_in(SOCKET clientSocket,const std::string& user_email, const std::string& user_password, int i) //entry
{
    std::ifstream in1(R"(C:\Users\Public\include\data_clients.txt)");
    std::string line;
    std::lock_guard<std::mutex> lock(clientsMutex);
    if(in1.is_open())
    {
        while(std::getline(in1,line))
        {
            if(check_line_from_data(line, user_email, user_password))
            {
                Client* current_client = find_client_from_socket(clientSocket);
                current_client->email=user_email;
                current_client->password=user_password;
                current_client->name=extract_name_from_string(line, i);
                return;
            }
        }
    }
    std::string buff="����ୠ� ���� ��� ��஫�. �������� ������ ������ ��� ��ॣ���������.";
    send(clientSocket, buff.c_str(), buff.size(), 0);
    in1.close();
}


std::vector<SOCKET> find_socket_from_name(const std::vector<std::string>& send_to_names)
{
    std::vector<SOCKET> answer;
    for (const auto & send_to_name : send_to_names)
    {
        for (auto & client : clients)
        {
            if (client.name==send_to_name)
            {
                answer.push_back(client.socket);
                break;
            }
        }
    }
    return answer;
}

void Group_message(SOCKET clientSocket, const std::string& mes, const std::vector<std::string>& send_to_names)
{
    std::vector<SOCKET> socket_send=find_socket_from_name(send_to_names);
    std::string name_sender= find_name_from_socket(clientSocket);
    std::string group_users=name_sender+" ";
    for(int i=0; i<send_to_names.size(); i++)
        group_users=group_users+send_to_names[i]+" ";
    std::string buff="Group: "+group_users+"\n"+name_sender+":\n";
    buff+=mes;
    for (unsigned long long i : socket_send)
        send(i, buff.c_str(), buff.size(), 0);
    std::string input=name_sender;
    for(const auto & send_to_name : send_to_names)
        input=input+"%"+send_to_name;
    std::vector<std::string> result = generatePermutations(input);
    History_of_chats(buff, result);
}

void Private_message(SOCKET clientSocket, const std::string& mes, const std::vector<std::string>& send_to_names)
{
    if (send_to_names.empty() || send_to_names.size()>1)
    {
        std::string buff="�� ����� ������� private, ������ ���-�� �� ���� �� ��� ��ࠢ��� �� ࠢ�� 1\n";
        send(clientSocket, buff.c_str(), buff.size(), 0);
    }
    else
    {
        print_users();
        std::string name_sender= find_name_from_socket(clientSocket);
        std::string buff="Private chat. "+name_sender+":\n";
        buff+=mes;
        std::vector<SOCKET> socket_send=find_socket_from_name(send_to_names);
        if (!socket_send.empty())
        {
            send(socket_send[0], buff.c_str(), buff.size(), 0);
            std::string input=name_sender;
            for(const auto & send_to_name : send_to_names)
                input=input+"%"+send_to_name;
            std::vector<std::string> result = generatePermutations(input);
            History_of_chats(buff, result);
        }
        else
        {
            buff="�� ��� ��ࠢ��� ᮮ�饭�� ���짮��⥫� ��� ������ "+send_to_names[0]+", �� ⠪��� ���짮��⥫� �� �������.";
            send(clientSocket, buff.c_str(), buff.size(), 0);
        }
    }
}



void Reformat_message(SOCKET clientSocket, const std::string& text, std::string& where, std::vector<std::string>& send_to_names, std::string& mes) //0-public,1-private,group
{
    long long i=0;
    while(text[i]!=':' && i<text.size())
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
    else if(where=="show_online")
    {
        show_online(clientSocket);
        return;
    }
    else if(where=="show_history")
    {
        std::string chat;
        while(text[i]!='\0')
        {
            chat+=text[i];
            i++;
        }
        if(chat.find(find_name_from_socket(clientSocket))==std::string::npos && chat!="public")
        {
            std::string err="� ��� ��� ����㯠 � �⮬� ���.";
            send(clientSocket, err.c_str(), err.size(), 0);
            return;
        }
        std::vector<std::string> result = generatePermutations(chat);
        show_history(clientSocket, result);
        return;
    }
    else if(where=="delete_history")
    {
        std::string chat;
        while(text[i]!='\0')
        {
            chat+=text[i];
            i++;
        }
        if(chat.find(find_name_from_socket(clientSocket))==std::string::npos)
        {
            std::string err="� ��� ��� ����㯠 � �⮬� ���.";
            send(clientSocket, err.c_str(), err.size(), 0);
            return;
        }
        std::vector<std::string> result = generatePermutations(chat);
        delete_history(clientSocket, result);
        return;
    }
    else if(where=="sign_in")
    {
        std::string mail;
        while(text[i]!=':' && i<text.size())
        {
            mail+=text[i];
            i++;
        }
        i++;
        while(text[i]!='\0' && i<text.size())
        {
            mes+=text[i];
            i++;
        }
        if(mail.find(':')!=std::string::npos || mes.find(':')!=std::string::npos)
        {
            std::string err="����饭� �ᯮ�짮���� ':' � ��஫�.";
            send(clientSocket, err.c_str(), err.size(), 0);
            return;
        }
        sign_in(clientSocket, mail, mes, i-7);
        return;
    }
    if (key)
    {
        while(text[i]!=':' && i<text.size())
        {
            std::string temporary_name;
            while(text[i]!=',' && text[i]!=':' && i<text.size())
            {
                temporary_name+=text[i];
                i++;
            }
            send_to_names.push_back(temporary_name);
            if(text[i]!=':')
                i++;
        }
        i++;
        if (send_to_names.empty())
        {
            where="";
            return;
        }
    }
    while(text[i]!='\0' && i<text.size())
    {
        mes+=text[i];
        i++;
    }
    if (mes.empty())
        where="";
}


void clientHandler(SOCKET clientSocket) {
    // ��ࠡ�⪠ ������
    // �ਬ�� �⥭�� ᮮ�饭�� �� ������ � ��ࠢ�� �⢥⮢
    std::string buffer;
    char buff[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buff, sizeof(buff), 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            std::cout<<"SEND_MESSAGE_ERROR";
            break;
        }
        buffer.clear();
        buff[bytesReceived]='\0'; // ������塞 �������騩 ᨬ��� ��ப�
        buffer=buff;
        if (buffer=="exit")
        {
            exit(clientSocket);
            return;
        }
        else if (buffer=="sign_up")
        {
            sign_up(clientSocket);
            continue;
        }
        else if(buffer=="end_registration")
        {
            end_registration(clientSocket);
            continue;
        }
        else if(buffer=="end_entry")
        {
            end_entry(clientSocket);
            continue;
        }
        std::string where;
        std::vector<std::string> send_to_names;
        std::string mes;
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
        else if (where=="set_name" || where=="set_email" || where=="set_password" || where=="sign_in" || where=="sign_up" || where=="show_online" || where=="delete_history" || where=="show_history")
            continue;
        else
        {
            std::string err="���ࠢ���� �ଠ� ������";
            send(clientSocket, err.c_str(), err.size(), 0);
            std::cout<<"Wrong format";
        }
    }

    // ����塞 ������ �� ᯨ᪠ ��᫥ �����襭�� ᮥ�������
//    {
//        std::lock_guard<std::mutex> lock(clientsMutex);
//        for (auto it = clients.begin(); it != clients.end(); ++it) {
//            if (it->socket == clientSocket) {
//                clients.erase(it);
//                break;
//            }
//        }
//    }

    //closesocket(clientSocket);
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
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        // ���������� ������ � ᯨ᮪
        std::lock_guard<std::mutex> lock(clientsMutex);
        Client temporary_client;
        temporary_client.socket=clientSocket;
        clients.push_back(temporary_client);
        std::string err="������ � ������ ��� ��ॣ���������";
        send(clientSocket, err.c_str(), err.size(), 0);
//        for(int i=0; i<clients.size(); i++)
//            std::cout<<clients[i].email<<":"<<clients[i].password<<":"<<clients[i].name<<"\n";
        std::cout<<"������稫�� ���� ������. ID:"<<clients[clients.size()-1].id<<"\n";
        std::cout<<"Socket:"<<clientSocket<<"\n";
        // ����� ��ࠡ��稪� ������ � �⤥�쭮� ��⮪�
        std::thread clientThread(clientHandler, clientSocket);
        clientThread.detach(); // ��ᮥ���塞 ��⮪, �⮡� �� ����� ��� �����襭��
    }

    // �����⨥ ᮪�� �ࢥ� � �����襭�� ࠡ��� Winsock
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}


