#include <string>
#include <iostream>
#include <zmq.hpp>
#include <unistd.h>
#include <vector>
#include <thread>
#include <time.h>

using namespace std;


int main(){
    zmq::context_t context;

    zmq::socket_t subscriber(context, zmq::socket_type::sub);
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    subscriber.connect("tcp://127.0.0.1:5557");

    zmq::socket_t pusher(context, zmq::socket_type::push);
    pusher.connect("tcp://127.0.0.1:5558");

    int random;
    string input;
    zmq::message_t message;
    zmq::pollitem_t p[] = {
        {subscriber, 0, ZMQ_POLLIN, 0}
    };
    srand((unsigned)time(NULL));
    while(1){
        int rc = zmq::poll(p ,1 ,100);
        if (rc != 0){
            if(p[0].revents !=0 & ZMQ_POLLIN){
                subscriber.recv(&message);
                cout<< "I: received message b'" << std::string(static_cast<char*>(message.data()), message.size())<<"'" << endl;
            }
        } else{
            random = rand () % 100 + 1;
            if(random < 10){
                input = to_string(random);
                zmq::message_t message(input.length());
                memcpy(message.data(), input.data(), input.length());
                pusher.send(message, 1);
                cout<<"I: sending message " << random << endl;
            }            
        }
    }
    return 0;
}