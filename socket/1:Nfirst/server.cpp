#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;


static int clientNum = 0;

int main()
{

    int server, client;
    int portNum = 65457;
    bool isExit = false;
    int bufsize = 1024;
    char buffer[bufsize];

    struct sockaddr_in server_addr;
    socklen_t size;

    server = socket(AF_INET, SOCK_STREAM, 0);

    if (server < 0) 
    {
        cout << "\nError establishing socket..." << endl;
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(portNum);


    if ((bind(server, (struct sockaddr*)&server_addr,sizeof(server_addr))) < 0) 
    {
        cout << "> bind() failed by exception" << endl;
        return -1;
    }

    size = sizeof(server_addr);
    cout << "echo-server is activated" << endl;

    if (listen(server, 1) == -1){
        cout<< "> listen() failed and program terminated" <<endl;
        close(client);
    }


    int clientCount = 1;
    client = accept(server,(struct sockaddr *)&server_addr,&size);
    clientNum++;
    if (client < 0) 
        cout << "=> Error on accepting..." << endl;
    
    cout << "Client connected by IP address 127.0.0.1 with Port number " << portNum << endl;
    while (1) 
    {
        send(client, buffer, bufsize, 0);
        while(1){
            cout << "received: ";
            recv(client, buffer, bufsize, 0);
            cout << buffer << endl;
            if(strcmp(buffer, "quit") == 0){
                clientNum--;
                break;
            }
            send(client, buffer, bufsize, 0);
        }
        client = accept(server,(struct sockaddr *)&server_addr,&size);
        if(client >0){
            cout << "Client connected by IP address 127.0.0.1 with Port number " << portNum << endl;
        }
        if(isExit)
            close(client);
    }
    cout << "echo-server is de-activated" <<endl;
    close(server);
    
    return 0;
}