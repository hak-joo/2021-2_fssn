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

using namespace std;

int main()
{
   
    int client;
    int portNum = 65457;
    bool isExit = false;
    int bufsize = 1024;
    char buffer[bufsize];
    char* ip = "127.0.0.1";

    struct sockaddr_in server_addr;

    client = socket(AF_INET, SOCK_STREAM, 0);


    if (client < 0) 
    {
        cout << "\nError establishing socket..." << endl;
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portNum);
    server_addr.sin_addr.s_addr = inet_addr(ip);
  
    connect(client,(struct sockaddr *)&server_addr, sizeof(server_addr));


    cout<< "echo-client is activated" << endl;

    do {
        cin >> buffer;
        send(client, buffer, bufsize, 0);
        if(strcmp(buffer, "quit") == 0 ){
            isExit = true;
            break;
        }
        recv(client, buffer, bufsize, 0);
        
        cout<<"echoed: " << buffer << endl;

    } while(!isExit);

    cout << "echo-client is de-activated" <<endl;
    close(client);
    return 0;
}