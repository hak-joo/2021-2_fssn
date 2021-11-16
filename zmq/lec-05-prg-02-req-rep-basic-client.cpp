#include <string>
#include <iostream>
#include <zmq.hpp>

using namespace std;

int main(){
    zmq::context_t context;
    cout<<"Connecting to hello world server..."<< endl;
    zmq::socket_t client(context, zmq::socket_type::req);
    client.connect("tcp://127.0.0.1:5555");
    zmq::message_t reply;
    string input = "Hello";
    zmq::message_t message(input.length());


    for (int i = 0; i < 10; i++){
        cout<<"Sending request " << i <<"..."<<endl;
        
        zmq::message_t message(input.length());
        memcpy(message.data(), input.data(), input.length());
        client.send(message); 
        client.recv(reply);
        std::string rpl = std::string(static_cast<char*>(reply.data()), reply.size());

        cout<<"Received reply " << input << " [ "<< rpl << " ]"<<endl;
    }
    return 0;
}