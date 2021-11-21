#include <string>
#include <iostream>
#include <zmq.hpp>
#include <unistd.h>
#include <time.h>

using namespace std;

int main(){

    zmq::context_t context;
    zmq::socket_t server(context, zmq::socket_type::pub);

    server.bind("tcp://*:5556");
    string input;
    int zipcode, temperature, relhumidity;
    cout<<"Publishing updates at weather server...\n";
    
    while(1){
        zipcode = rand() % 100000 + 1;
        temperature = rand() % 215 - 80;
        relhumidity = rand() % 50 + 10;
        input = to_string(zipcode) + " " + to_string(temperature) + " " + to_string(relhumidity);
        zmq::message_t reply{input.length()};
        memcpy(reply.data(), input.data(), input.length());
        server.send(reply);
    }

    return 0;
}
