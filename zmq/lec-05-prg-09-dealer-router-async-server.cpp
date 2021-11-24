#include <string>
#include <iostream>
#include <zmq.hpp>
#include <unistd.h>
#include <time.h>
#include <thread>

using namespace std;

static zmq::context_t context;

class ServerWorker{
    public:
        ServerWorker(int i_id)
        :id(i_id){}

        void work(){

            zmq::socket_t worker(context, zmq::socket_type::dealer);
            worker.connect("inproc://backend");
            cout<<"Worker#"<< id<< " started"<<endl;
            zmq::message_t msg;
            zmq::message_t iden;
            while(1){
                
                worker.recv(iden);
                worker.recv(msg);
                string received = string(static_cast<char*>(msg.data()), msg.size());
                string identity = string(static_cast<char*>(iden.data()), iden.size());
                cout<<"Worker#"<<id<<" received "<< received<<" from "<<identity<<endl;
                
                zmq::message_t sendMsg(received.size());
                memcpy(sendMsg.data(), received.data(), received.size());
                worker.send(&sendMsg, 0);

            }
        }
    private:
        int id;
};
void *ServerTask(void*);

int main(int argc, char* argv[]){
    int server_num = stoi(argv[1]);

    zmq::socket_t frontend(context, zmq::socket_type::router);
    frontend.bind("tcp://*:5570");
    zmq::socket_t backend(context, zmq::socket_type::dealer);
    backend.bind("inproc://backend");
    pthread_t thread_id = 0;
    for (int i=0; i<server_num; i++){
        ServerWorker *worker = new ServerWorker(i);
        pthread_create(&thread_id, 0, &ServerTask, (void *)worker);

    }
    zmq::proxy(frontend, backend, NULL);
    frontend.close();
    backend.close();

    return 0;
}

void *ServerTask(void* worker){
    ServerWorker *serverWorker = (ServerWorker*)worker;
    serverWorker->work();
    
}

