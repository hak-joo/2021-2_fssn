#include <string>
#include <iostream>
#include <zmq.hpp>
#include <unistd.h>
#include <time.h>

using namespace std;

int main(){

    zmq::context_t context;
    zmq::socket_t publisher(context, zmq::socket_type::pub);
    publisher.bind("tcp://*:5557");
    zmq::socket_t collector(context, zmq::socket_type::pull);
    collector.bind("tcp://*:5558");

    zmq::message_t message;

    while(1){
        collector.recv(message);
        string received = string(static_cast<char*>(message.data()), message.size());
        cout<< "Server: publishing update => " << std::string(static_cast<char*>(message.data()), message.size()) << endl;
        memcpy(message.data(), received.data(), received.length());
        publisher.send(message);
    }

    return 0;
}
