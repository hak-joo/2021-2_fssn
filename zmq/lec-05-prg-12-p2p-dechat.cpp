#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string>
#include <iostream>
#include <zmq.hpp>
#include <unistd.h>
#include <vector>
#include <thread>


using namespace std;

string get_local_ip();
string search_nameserver(string ip_mask, string local_ip_addr, string port_nameserver);
void beacon_nameserver(string local_ip_addr, string port_nameserver);
void user_manager_nameserver(string local_ip_addr, string port_subscribe);
void relay_server_nameserver(string local_ip_addr, string port_chat_publisher, string port_chat_collector);
vector<string> split(string s, string divid);
void user_registeration(string ip_addr_p2p_server, string port_subscribe, string ip_addr, string user_name, string port_chat_publisher, string port_chat_collector);

int main(int argc, char* argv[]){

    string ip_addr_p2p_server ="";
    string port_nameserver = "9001";
    string port_chat_publisher = "9002";
    string port_chat_collector = "9003";
    string port_subscribe = "9004";

    string user_name = argv[1];


    string ip_addr = get_local_ip();
   
    vector<string> temp_list = split(ip_addr, ".");


    string ip_mask = temp_list[0] + "." + temp_list[1] + "." + temp_list[2] + "."; 
    cout<<"searching for p2p server."<<endl;
    string name_server_ip_addr = search_nameserver(ip_mask, ip_addr, port_nameserver);
    
    if(name_server_ip_addr == "") {
        ip_addr_p2p_server = ip_addr;
        cout<<"p2p server is not found, and p2p server mode is activated."<<endl;
        std::thread beacon_thread([&ip_addr, &port_nameserver](){
            beacon_nameserver(ip_addr, port_nameserver);
        });
        std::thread db_thread([&ip_addr, &port_subscribe](){
            user_manager_nameserver(ip_addr, port_subscribe);
        });
        std::thread relay_thread([&ip_addr, &port_chat_publisher, &port_chat_collector](){
            relay_server_nameserver(ip_addr, port_chat_publisher, port_chat_collector);
        });
        std::thread user_thread([&ip_addr_p2p_server, &port_subscribe, &ip_addr, &user_name, &port_chat_publisher, &port_chat_collector](){
            user_registeration(ip_addr_p2p_server, port_subscribe, ip_addr, user_name, port_chat_publisher, port_chat_collector);
        });

        beacon_thread.join();
        cout<<"p2p beacon server is activated." << endl;
        db_thread.join();
        cout<<"p2p subscriber database server is activated."<<endl;
        relay_thread.join();
        cout<<"p2p message relay server is activated." <<endl;  
        user_thread.join();


    } else{
        ip_addr_p2p_server = name_server_ip_addr;
        cout<<"p2p server found at "<<ip_addr_p2p_server<< ", and p2p client mode is activated."<<endl;
        user_registeration(ip_addr_p2p_server, port_subscribe, ip_addr, user_name, port_chat_publisher, port_chat_collector);
    }

    return 0;
}
string get_local_ip(){
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    sockaddr_in loopback;

    if (sock == -1) {
        std::cerr << "Could not socket\n";
        return "127.0.0.1";
    }

    std::memset(&loopback, 0, sizeof(loopback));
    loopback.sin_family = AF_INET;
    loopback.sin_addr.s_addr = 1337;   // can be any IP address
    loopback.sin_port = htons(9);      // using debug port

    if (connect(sock, reinterpret_cast<sockaddr*>(&loopback), sizeof(loopback)) == -1) {
        close(sock);
        return "127.0.0.1";
    }

    socklen_t addrlen = sizeof(loopback);
    if (getsockname(sock, reinterpret_cast<sockaddr*>(&loopback), &addrlen) == -1) {
        close(sock);
        return "127.0.0.1";
    }
    int name = getsockname(sock, reinterpret_cast<sockaddr*>(&loopback), &addrlen);
    close(sock);

    char buf[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &loopback.sin_addr, buf, INET_ADDRSTRLEN) == 0x0) {
        return "127.0.0.1";
    } else {
        return buf;
    }
}

string search_nameserver(string ip_mask, string local_ip_addr, string port_nameserver){
    
    zmq::message_t msg;
    zmq::context_t context;
    zmq::socket_t req = zmq::socket_t(context, zmq::socket_type::sub);
    for(int i=0; i<255; i++){
        string target_ip_addr = "tcp://"+ip_mask + to_string(i)+ ":" + port_nameserver;
        req.connect(target_ip_addr);
        req.setsockopt(ZMQ_RCVTIMEO, 2000);
        req.setsockopt(ZMQ_SUBSCRIBE, "NAMESERVER", 10);
    }
    req.recv(msg);
    string res = string(static_cast<char*>(msg.data()), msg.size());
    if(res.size() >0){
        vector<string> res_list= split(res, ":");
        if(res_list[0] == "NAMESERVER"){
            return res_list[1];
        }
    }

    return "";
}

void beacon_nameserver(string local_ip_addr, string port_nameserver){
    zmq::context_t context;
    zmq::socket_t socket(context, zmq::socket_type::pub);
    
    socket.bind("tcp://"+local_ip_addr+":"+port_nameserver);
    string message = "NAMESERVER:"+local_ip_addr;
    while(1){
        usleep(1000000);
        zmq::message_t msg{message.length()};
        memcpy(msg.data(), message.data(), message.size());
        socket.send(msg);
        
    }
}

void user_manager_nameserver(string local_ip_addr, string port_subscribe){
    zmq::context_t context;
    zmq::socket_t socket(context, zmq::socket_type::rep);
    zmq::message_t msg;
    socket.bind("tcp://"+local_ip_addr+":"+port_subscribe);
    cout<<"local p2p db server activated at tcp://" << local_ip_addr << ":" << port_subscribe <<endl;
    vector<string> user_db;
    string sendMsg = "ok";

    while(1){
        socket.recv(msg);
        string user_req = string(static_cast<char*>(msg.data()), msg.size());
        user_db.push_back(user_req);
        cout<<"user registration "<< user_req<<endl;
        zmq::message_t rep_msg{sendMsg.length()};
        memcpy(rep_msg.data(), sendMsg.data(), sendMsg.size());
        socket.send(rep_msg);
    }
}

void relay_server_nameserver(string local_ip_addr, string port_chat_publisher, string port_chat_collector){
    zmq::context_t context;
    zmq::socket_t publisher(context, zmq::socket_type::pub);
    publisher.bind("tcp://"+local_ip_addr+":"+port_chat_publisher);
    zmq::socket_t collector(context, zmq::socket_type::pull);
    collector.bind("tcp://"+local_ip_addr+":"+port_chat_collector);
    cout<<"local p2p relay server activated at tcp://" << local_ip_addr<<":"<<port_chat_publisher<<" & "<<port_chat_collector<<endl;
    zmq::message_t msg;
    while(1){
        collector.recv(msg);
        string message = string(static_cast<char*>(msg.data()), msg.size());
        cout<<"p2p-relay:<==>"<< message<<endl;
        message = "RELAY:" + message;
        zmq::message_t sendMsg{message.length()};
        memcpy(sendMsg.data(), message.data(), message.size());
        publisher.send(sendMsg);
    }

}

void user_registeration(string ip_addr_p2p_server, string port_subscribe, string ip_addr, string user_name, string port_chat_publisher, string port_chat_collector){
    cout<<"starting user registration procedure."<<endl;

    zmq::context_t db_client_context;
    zmq::socket_t db_client_socket(db_client_context, zmq::socket_type::req);
    
    cout<<"db_client"<< "tcp://"+ip_addr_p2p_server+":"+port_subscribe<<endl;
    db_client_socket.connect("tcp://"+ip_addr_p2p_server+":"+port_subscribe);
    string db_msg = ip_addr + ":" + user_name;
 
    zmq::message_t db_msg_t{db_msg.length()};
    zmq::message_t db_msg_rep;
    cout<<db_msg<<endl;
    memcpy(db_msg_t.data(), db_msg.data(), db_msg.size());
    db_client_socket.send(db_msg_t);

    db_client_socket.recv(db_msg_rep);

    if(string(static_cast<char*>(db_msg_rep.data()), db_msg_rep.size()) == "ok"){
        cout<<"user registration to p2p server completed"<<endl;
    } else {
        cout<<"user registration to p2p server failed"<<endl;
    }
    cout<<"starting message transfer procedure"<<endl;

    zmq::context_t relay_client;
    zmq::socket_t p2p_rx(relay_client, zmq::socket_type::sub);
    p2p_rx.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    p2p_rx.connect("tcp://"+ip_addr_p2p_server+ ":"+port_chat_publisher);
    zmq::socket_t p2p_tx(relay_client, zmq::socket_type::push);
    p2p_tx.connect("tcp://"+ip_addr_p2p_server+ ":"+port_chat_collector);

    cout<<"starting autonomous message transmit and receive scenario"<<endl;
    
    zmq::pollitem_t p[] = {
            {p2p_rx, 0, ZMQ_POLLIN, 0}
        };
    zmq::message_t p2p_msg;
    srand((unsigned)time(NULL));

    while(1){
        int rc = zmq::poll(p,1,100);
        
        if(p[0].revents & ZMQ_POLLIN){
            p2p_rx.recv(p2p_msg);
            string received = string(static_cast<char*>(p2p_msg.data()), p2p_msg.size());
            cout<<"p2p-recv::<<== " << split(received,":")[1]<< ":" << split(received,":")[2]<<endl;
        } else {
            int randNum = rand() % 100 + 1;
            if(randNum <10){
                usleep(3000000);
                
                string msg = "("+user_name+","+ip_addr+":ON)";
                zmq::message_t p2p_on_msg{msg.length()};
                memcpy(p2p_on_msg.data(), msg.data(), msg.size());
                p2p_tx.send(p2p_on_msg);
                cout<<"p2p-send::==>> " << msg<<endl;
            } else{
                usleep(3000000);
                string msg = "("+user_name+","+ip_addr+":OFF)";
                zmq::message_t p2p_off_msg{msg.length()};
                memcpy(p2p_off_msg.data(), msg.data(), msg.size());
                p2p_tx.send(p2p_off_msg);
                cout<<"p2p-send::==>> " << msg<<endl;
            }
        }
    }
    cout<<"closing p2p chatting program"<<endl;
    db_client_socket.close();
    p2p_rx.close();
    p2p_tx.close();
}

vector<string> split(string s, string divid){
    vector<string> v;
    char *c = strtok((char*)s.c_str(), divid.c_str());
    while(c){
        v.push_back(c);
        c=strtok(NULL, divid.c_str());
    }
    return v;
}