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

    int server, client;
    int portNum = 65457;
    bool isExit = false;
    int bufsize = 1024;
    char buffer[bufsize];

    struct sockaddr_in server_addr;
    socklen_t size;

    server = socket(AF_INET, SOCK_STREAM, 0);

    if (server < 0) 
        exit(1);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(portNum);


    bind(server, (struct sockaddr*)&server_addr,sizeof(server_addr)); 
    

    size = sizeof(server_addr);
    cout << "echo-server is activated" << endl;

    listen(server, 1);


    int clientCount = 1;
    client = accept(server,(struct sockaddr *)&server_addr,&size);

    cout << "Client connected by IP address 127.0.0.1 with Port number " << portNum << endl;
    while (client > 0) 
    {
        do{
            cout << "received: ";
            recv(client, buffer, bufsize, 0);
            cout << buffer << endl;
            if(strcmp(buffer, "quit") == 0){
                isExit = true;
                break;
            }
            send(client, buffer, bufsize, 0);

        } while(!isExit);

        close(client);

        isExit = false;
        break;
    }
    cout << "echo-server is de-activated" <<endl;
    close(server);
    
    return 0;
}