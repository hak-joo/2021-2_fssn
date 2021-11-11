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

using namespace std;

void* SocketHandler(void*);
static int threadNum = 0;
int main(){

    int portNum = 65457;

    struct sockaddr_in server_addr, client_addr;

    int server;
    int * p_int ;
    int err;

    socklen_t addr_size = 0;
    int* csock;
    sockaddr_in sadr;
    pthread_t thread_id=0;
    char buffer[1024];
    int bufSize = 1024;

    server = socket(AF_INET, SOCK_DGRAM, 0);
    if(server == -1){
        printf("Error initializing socket %d\n", errno);
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portNum);
    memset(&(server_addr.sin_zero), 0, 8);
    server_addr.sin_addr.s_addr = INADDR_ANY ;
    bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr));    

    //Now lets do the server stuff
    socklen_t len;
    while(1){
        len = sizeof(client_addr);
        recvfrom(server, buffer, bufSize,0, (struct sockaddr*)&client_addr, &len);
        cout<<"> echoed: " << buffer << endl;
        sendto(server, buffer, bufSize,0, (struct sockaddr*)&client_addr, len);
    }

    cout<<"> echo-server is de-activated";

    close(server);
}