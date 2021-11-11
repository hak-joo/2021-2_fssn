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
#include <vector>
using namespace std;

vector<int> group_queue;

void* SocketHandler(void*);
void* QuitHandler(void*);
static int threadNum = 0;
int main(){

    int portNum = 65457;
    int* threadNumPointer = &threadNum;

    struct sockaddr_in server_addr;

    int server;
    int * p_int ;
    int err;

    char buffer[1024];
    int bufSize = 1024;

    socklen_t addr_size = 0;
    int* csock;
    sockaddr_in sadr;
    pthread_t thread_id=0;


    server = socket(AF_INET, SOCK_STREAM, 0);
    if(server == -1){
        printf("Error initializing socket %d\n", errno);
        goto FINISH;
    }
    
    p_int = (int*)malloc(sizeof(int));
    *p_int = 1;
        
    if( (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
        (setsockopt(server, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
        printf("Error setting options %d\n", errno);
        free(p_int);
        goto FINISH;
    }
    free(p_int);

    server_addr.sin_family = AF_INET ;
    server_addr.sin_port = htons(portNum);
    memset(&(server_addr.sin_zero), 0, 8);
    server_addr.sin_addr.s_addr = INADDR_ANY ;
    
    if( bind( server, (sockaddr*)&server_addr, sizeof(server_addr)) == -1 ){
        fprintf(stderr,"Error binding to socket, make sure nothing else is listening on this port %d\n",errno);
        goto FINISH;
    }
    if(listen( server, 10) == -1 ){
        fprintf(stderr, "Error listening %d\n",errno);
        goto FINISH;
    }


    pthread_create(&thread_id, 0, &QuitHandler, (void*)threadNumPointer);
    addr_size = sizeof(sockaddr_in);
    while(true){
        csock = (int*)malloc(sizeof(int));
        if((*csock = accept( server, (sockaddr*)&sadr, &addr_size))!= -1){
            std::cout<< "client connected by IP address 127.0.0.1 with Port number";
            threadNum++;
            pthread_create(&thread_id,0, &SocketHandler, (void*)csock );
            cout<<"hello";
            pthread_detach(thread_id);
            cout<<"HI";
        }
        else{
            fprintf(stderr, "Error accepting %d\n", errno);
        }
    }

    
FINISH:
;
}

void* SocketHandler(void* lp){
    int *server = (int*)lp;
    group_queue.push_back(*server);
    std::thread::id this_id = std::this_thread::get_id();
    char buffer[1024];
    int bufSize = 1024;
    int bytecount;
    bool isExit = false;
    send(*server, buffer, bufSize, 0);
    recv(*server, buffer, bufSize, 0);
    std::cout << buffer <<std::endl;
    do{
        
        recv(*server, buffer, bufSize, 0);

        if(strcmp(buffer, "quit") == 0){
            threadNum--;
            isExit = true;
            goto FINISH;
        } else{
            std::cout<<"received ( "<< buffer << " ) and echoed to " << threadNum << " clients"<<std::endl;
        }
        for(int i=0, end = group_queue.size(); i<end; ++i){
            send(group_queue[i], buffer, bufSize, 0);
        }
    } while(!isExit);
    // 큐 안에 클라이언트 데이터 삭제 및 threadNum 감소
    for(int i=0, end = group_queue.size(); i<end; ++i){
        if(group_queue[i] == *server){
            group_queue.erase(group_queue.begin() + i);
        }
    }
    free(server);
FINISH:
    free(server);
    return 0;
}

void* QuitHandler(void* lp){
    int* threadNum = (int*)lp;
    char input[100];
    while(1){
        cin>>input;
        if(strcmp(input, "quit") == 0){
            if(*threadNum != 0 ){
                cout<<"active threads are remained: " << *threadNum << endl;
            } else{
                exit(1);
            }
        }
    }

}