#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <thread>
#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;

vector<sockaddr_in> group_queue;

void* SocketHandler(void*);
int main(){
    int portNum = 65457;
    struct sockaddr_in server_addr, client_addr;
    int server;
    socklen_t addr_size = 0;
    sockaddr_in sadr;
    char buffer[1024];
    int bufSize = 1024;

    server = socket(AF_INET, SOCK_DGRAM, 0);
    int* csock = &server;
    if(server == -1){
        printf("Error initializing socket %d\n", errno);
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portNum);
    memset(&(server_addr.sin_zero), 0, 8);
    server_addr.sin_addr.s_addr = INADDR_ANY ;
    bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr));    

    socklen_t len;
    bool found = false;
    while(1){
        len = sizeof(client_addr);
        recvfrom(server, buffer, bufSize,0, (struct sockaddr*)&client_addr, &len);

        if(strcmp(buffer, "#REG") == 0) { // 등록
            found = false;
            for(int i=0, end = group_queue.size(); i<end; ++i){ // 포트 번호 비교를 통해 데이터 비교
                if(group_queue[i].sin_port == client_addr.sin_port)
                    found = true;
            }
            if (!found){
                group_queue.push_back(client_addr); //등록
                std::cout<<"> client registered ('127.0.0.1', " <<  ntohs(client_addr.sin_port) << ")\n";
            }
        } else if (strcmp(buffer, "#DEREG") ==0 || strcmp(buffer, "quit") == 0){
            found = false;
            for(int i=0, end = group_queue.size(); i<end; ++i){ // 삭제
                if(group_queue[i].sin_port == client_addr.sin_port){
                    found = true;
                    group_queue.erase(group_queue.begin() + i);
                }
            }
            if (found){
                std::cout<<"> client de-registered ('127.0.0.1', " <<  ntohs(client_addr.sin_port) << ")\n";
            }
        } else{
            found = false;
            for(int i=0, end = group_queue.size(); i<end; ++i){
                if(group_queue[i].sin_port == client_addr.sin_port){
                    found = true;
                }
            }
            if(group_queue.size() == 0){
                std::cout<<"> no clients to echo\n";
                continue;
            }

            if (found){ // 전체 보내기
                for(int i=0, end = group_queue.size(); i<end; ++i){
                    sendto(server, buffer, bufSize, 0, (struct sockaddr *)&group_queue[i], sizeof(group_queue[i]));
                }
                std::cout<<"> received ( "<< buffer<<" ) and echoed to "<< group_queue.size() << " clients \n";
            } else {
                std::cout<<"> ignores a message from un-registered client\n";
            }
        }

    }

    cout<<"> echo-server is de-activated";

    close(server);

    return 0;
}