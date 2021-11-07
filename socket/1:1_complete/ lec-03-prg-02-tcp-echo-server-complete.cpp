#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

int main()
{

    int client, server;
    int portNum = 65457;
    bool isExit = false;
    int bufsize = 1024;
    char buffer[bufsize];

    struct sockaddr_in server_addr;
    socklen_t size;

    client = socket(AF_INET, SOCK_STREAM, 0);

    if (client < 0) 
    {
        cout << "\nError establishing socket..." << endl;
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(portNum);


    if ((bind(client, (struct sockaddr*)&server_addr,sizeof(server_addr))) < 0) 
    {
        cout << "> bind() failed by exception" << endl;
        return -1;
    }

    size = sizeof(server_addr);
    cout << "echo-server is activated" << endl;

    if (listen(client, 1) == -1){
        cout<< "> listen() failed and program terminated" <<endl;
        close(server);
    }


    int clientCount = 1;
    server = accept(client,(struct sockaddr *)&server_addr,&size);

    if (server < 0) 
        cout << "=> Error on accepting..." << endl;
    
    cout << "Client connected by IP address 127.0.0.1 with Port number " << portNum << endl;
    while (server > 0) 
    {
        send(server, buffer, bufsize, 0);
        do{
            cout << "received: ";
            recv(server, buffer, bufsize, 0);
            cout << buffer << endl;
            if(strcmp(buffer, "quit") == 0){
                isExit = true;
                break;
            }
            send(server, buffer, bufsize, 0);

        } while(!isExit);

        close(server);

        isExit = false;
        break;
    }
    cout << "echo-server is de-activated" <<endl;
    close(client);
    
    return 0;
}