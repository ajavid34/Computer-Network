#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#define MESSAGE_ID 0
#define HEADER_LENGTH 2
#define MAX_PAYLOAD_LENGTH 10000
#include "message.h"
#include <ostream> 
#include <fstream>
#include <map>
#include <pthread.h>
char buffer[256];

using namespace std;

map<string,uint16_t> users;

typedef struct {
    int sock;
} update_prime_struct;

pthread_mutex_t lock;

void print_list(vector<string>& user_list){
    clog << "\n";
    for (int i = 0; i < user_list.size(); i++)
        clog << "- <" << user_list[i] << ">" << endl;
}
string get_info(uint16_t id, int sock){
    string user_name;
    Header reply_message,message;
    message.message_type = INFO;
    message.message_id = MESSAGE_ID + 1;
    message.length = HEADER_LENGTH + HEADER_LENGTH;
    write(sock,(uint8_t*)&message,sizeof(Header));
    write(sock,(uint8_t*)&id,sizeof(id));
    read(sock, (uint8_t*)&reply_message, sizeof(Header));
    auto user_len = reply_message.length - 2;
    uint8_t payload[MAX_PAYLOAD_LENGTH + 1];
    read(sock, (uint8_t*)&payload, user_len);
    payload[user_len] = 0;
    user_name = string((char*)payload);
    users[user_name] = id;
    return user_name;

}
void update_list(int sock, bool not_print = 1){
    Header reply_message,message;
    message.message_type = LIST;
    message.message_id = MESSAGE_ID + 1;
    message.length = HEADER_LENGTH;
    write(sock,(uint8_t*)&message,sizeof(Header));
    read(sock, (uint8_t*)&reply_message, sizeof(Header));
    int num = (reply_message.length - 2) / 2;
    vector<uint16_t> user_list;
    vector<string> user_name_list;;
    uint16_t id;
    for (int i = 0; i < num; i++) {
        read(sock, (uint8_t*)&id, sizeof(id));
        user_list.push_back(id);
    }

    for (int i = 0; i < num; i++)
        user_name_list.push_back(get_info(user_list[i], sock));
    if (not_print)
        return;
    print_list(user_name_list);
}
string get_name(uint16_t id){
   for (auto &it : users) {
      if (it.second == id) 
        return it.first;
   }
   return "";
}

void send_message(int sock,istringstream& stream){
    Header reply_message,message;
    string username,temp;
    if(!users.count(username))
        update_list(sock);
    stream >> username;
    getline(stream, temp);
    char* mess_info = strcpy(new char[temp.length() + 1], temp.c_str());
    if (!users.count(username)) {
        clog << "user not found" << endl;
        return;
    }
    uint16_t id = users[username];
    message.message_type = SEND;
    message.message_id = MESSAGE_ID + 2;
    message.length = HEADER_LENGTH + HEADER_LENGTH + strlen(mess_info);
    write(sock,(uint8_t*)&message,sizeof(Header));
    write(sock,(uint8_t*)&id,sizeof(id));
    write(sock,mess_info,strlen(mess_info));
    read(sock, (uint8_t*)&reply_message, sizeof(Header));
}

void receive(int sock){
    Header reply_message,message;
    message.message_type = RECEIVE;
    message.message_id = MESSAGE_ID + 1;
    message.length = HEADER_LENGTH;
    write(sock,(uint8_t*)&message,sizeof(Header));
    read(sock, (uint8_t*)&reply_message, sizeof(Header));
    uint8_t mess_len = reply_message.length - 4;
    uint16_t id;
    read(sock, (uint8_t*)&id, sizeof(id));
    uint8_t payload[MAX_PAYLOAD_LENGTH + 1];
    read(sock, (uint8_t*)&payload, mess_len);
    payload[mess_len] = 0;
    string mess_info = string((char*)payload);
    if (id == 0 || mess_info.empty())
        return;
    string username = get_name(id);
    if (username == ""){
        update_list(sock);
        username = get_name(id);
    }

    clog << "\n" << username << ": " << mess_info << endl;
}
void connect(char* username, int sock) {
    Header reply_message,message;
    message.message_type = CONNECT;
    message.message_id = MESSAGE_ID;
    message.length = HEADER_LENGTH + strlen(username);
    write(sock,(uint8_t*)&message,sizeof(Header));
    write(sock,username,strlen(username));
    read(sock, (uint8_t*)&reply_message, sizeof(Header));
}

void* update(void * sock) {
    update_prime_struct* sock_struct = (update_prime_struct*) sock;
    int socket = sock_struct->sock;
    while (true)
    {
        pthread_mutex_lock(&lock);
        receive(socket);
        fflush(stdout);
        pthread_mutex_unlock(&lock);
        fflush(stdout);
        sleep(2);
    }
}


int main(int argc, char** argv) {
    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;
    string temp = argv[1];
    int PORT = stoi(temp.substr(temp.find(":") + 1, temp.length()));
    char* username = argv[2];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
    if ((client_fd
         = connect(sock, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    connect(username,sock);
    pthread_t thread_id;
    update_prime_struct aux;
    aux.sock = sock;
    pthread_mutex_init( &lock, NULL);
    pthread_create(&thread_id, NULL, update, &aux);
    while (true) {
        istringstream stream;
        string massage_type;
        bzero(buffer, 255);
        fgets(buffer, 255, stdin);
        stream.rdbuf()->pubsetbuf(buffer, sizeof(buffer));
        stream >> massage_type;
        if (massage_type == "list")
        {
            pthread_mutex_lock(&lock);
            update_list(sock, 0);
            pthread_mutex_unlock(&lock);
        }
           

        else if (massage_type == "send")
        {
            pthread_mutex_lock(&lock);
            send_message(sock,stream);
            pthread_mutex_unlock(&lock);
        }
            

        else if (massage_type == "exit") 
            break;

        fflush(stdout);

    }
    pthread_mutex_destroy(&lock);

    return EXIT_SUCCESS;
}
