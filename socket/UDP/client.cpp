#include <iostream>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <thread>


using namespace std;

void* ReceiveHandler(void*);

int main()
{
    int client;
    socklen_t len;
    int portNum = 65457; // NOTE that the port number is same for both client and server
    bool isExit = false;
    int bufsize = 1024;
    char buffer[bufsize];
    char* ip = "127.0.0.1";

    struct sockaddr_in server_addr, client_addr;
    pthread_t thread_id=0;

    client = socket(AF_INET, SOCK_DGRAM, 0);


    if (client < 0) 
    {
        cout << "\nError establishing socket..." << endl;
        exit(1);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portNum);
    server_addr.sin_addr.s_addr = inet_addr(ip);
  
    int *csock = &client;
    // pthread_create(&thread_id,0, &ReceiveHandler, (void*)csock);
    
    cout<< "echo-client is activated" << endl;

    do {
        cout<<"input: ";
        cin >> buffer;
        sendto(client, buffer, bufsize, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if(strcmp(buffer, "quit") == 0 ){
            cout << "echo-client is de-activated" <<endl;
            close(client);
            exit(1);
        }
        recvfrom(client, buffer, bufsize, 0, (struct sockaddr*)&client_addr, &len);
        cout<< "> received: " << buffer<<endl;

    } while(!isExit);
    // pthread_detach(thread_id);
    close(client);
    return 0;
}

void* ReceiveHandler(void* lp){
    int *csock = (int*)lp;
    char buffer[1024];
    int bufSize = 1024;
    int bytecount;
    bool isExit = false;
    
    do{
        recv(*csock, buffer, bufSize, 0);
        if(strcmp(buffer, "") != 0 ){
            std::cout<<"> received: "<< buffer << std::endl;
        }
        buffer[0] = '\0'; //버퍼 비우기
        if(csock == NULL){
            isExit = true;
            break;
        }   
    } while(!isExit);
    isExit = false;
}