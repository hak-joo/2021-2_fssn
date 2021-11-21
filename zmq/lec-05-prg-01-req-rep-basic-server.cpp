#include <string>
#include <iostream>
#include <zmq.hpp>
#include <unistd.h>

using namespace std;

int main(){

    zmq::context_t context;
    zmq::socket_t server(context, zmq::socket_type::rep);
    zmq::message_t request;
    server.bind("tcp://*:5555");
    string input = "World";

    while(1){
        server.recv(&request);
        std::string rqst = std::string(static_cast<char*>(request.data()), request.size());
        std::cout << "Received request: [" << std::string(static_cast<char*>(request.data()), request.size())  << "]" << std::endl;
        usleep(1000000);
        zmq::message_t reply{input.length()};
        memcpy(reply.data(), input.data(), input.length());
        server.send(reply);
    }

    return 0;
}
