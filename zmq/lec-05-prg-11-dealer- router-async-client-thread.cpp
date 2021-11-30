#include <string>
#include <iostream>
#include <zmq.hpp>
#include <unistd.h>
#include <vector>
#include <thread>
#include <time.h>

using namespace std;


class ClientTask{
    public:
        ClientTask(char* argv)
        :identity(argv),
        reqs(0),
        context(1),
        socket(context, zmq::socket_type::dealer){}
        void *recvHandle(void);
        void start();

    private:
        string identity;
        int reqs;
        zmq::context_t context;
        zmq::socket_t socket;
};


int main(int argc, char *argv[]) {
    ClientTask client(argv[1]);
    client.start();

    return 0;
}


void ClientTask::start(){

    socket.setsockopt(ZMQ_IDENTITY, identity);
    socket.connect("tcp://127.0.0.1:5570");
    cout<<"Client " << identity << " started" << endl;
    string input;
    zmq::message_t msg;
    zmq::message_t iden;

    std::thread recvThread(&ClientTask::recvHandle, this);
    while(1) {
        usleep(1000000);
        reqs += 1;
        cout<<"Req #" << reqs << " sent... " <<endl;
        input = "b'request#" + to_string(reqs) + "'";            

        zmq::message_t msg{input.size()};
        memcpy(msg.data(), input.data(), input.length());
        socket.send(msg);                
    }
}

void* ClientTask::recvHandle(void){
    socket.setsockopt(ZMQ_IDENTITY, identity);

    cout<<"thread start";
    zmq::pollitem_t p[] = {
        {socket, 0, ZMQ_POLLIN, 0}
    };
    zmq::message_t msg;
    while(1){
        int rc = zmq::poll(p, 1, 100);

        if(p[0].revents & ZMQ_POLLIN){
            socket.recv(msg);
            cout<<msg<<endl;
            cout<< identity<< " received: "<< string(static_cast<char*>(msg.data()), msg.size()) << endl;
        }
    }
    
}