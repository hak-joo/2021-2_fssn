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
        ClientTask(char* argv){
            identity = argv;
            reqs = 0;
        }
        void start(){
            zmq::context_t context;
            zmq::socket_t socket(context, zmq::socket_type::dealer);
            socket.setsockopt(ZMQ_IDENTITY, identity);
            socket.connect("tcp://127.0.0.1:5570");
            cout<<"Client " << identity << " started" << endl;
            string input;
            zmq::message_t msg;
            zmq::message_t iden;
            
            zmq::pollitem_t p[] = {
                {socket, 0, ZMQ_POLLIN, 0}
            };
            
            
            while(1) {
                usleep(1000000);
                int rc = zmq::poll(p ,1 ,1);
                cout<<"rc:: "<<rc<<endl;
               
                if(p[0].revents & ZMQ_POLLIN){
                    socket.recv(&iden);
                    socket.recv(&msg);
                    cout<<"Client #"<< identity<< " "<< string(static_cast<char*>(msg.data()), msg.size()) << endl;
                }
                else{
                    reqs += 1;
                    cout<<"Req #" << reqs << " sent... " <<endl;
                    input = "b'request#" + to_string(reqs) + "'";            

                    zmq::message_t msg(input.size());
                    memcpy(msg.data(), input.data(), input.length());
                    socket.send(msg, 1);
                    
                }                
            }
        }
    private:
        string identity;
        int reqs;
};


int main(int argc, char *argv[]) {
    ClientTask client(argv[1]);
    client.start();
    return 0;
}