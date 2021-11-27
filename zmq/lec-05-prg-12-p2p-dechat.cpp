#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string>
#include <string.h>
#include <iostream>
#include <zmq.hpp>
#include <unistd.h>
#include <vector>
#include <thread>

using namespace std;

string get_local_ip();
string search_nameserver(string ip_mask, string local_ip_addr, string port_nameserver);
void user_manager_nameserver(string local_ip_addr, string port_subscribe);
void relay_server_nameserver(string local_ip_addr, string port_chat_publisher, string port_chat_collector);


int main(int argc, char* argv[]){
    string ip_addr_p2p_server ="";
    string port_nameserver = "9001";
    string port_chat_publisher = "9002";
    string port_chat_collector = "9003";
    string port_subscribe = "9004";

    string user_name = argv[1];

    string ip_addr = get_local_ip();
    
    char *cstr = new char[ip_addr.length() + 1];
    strcpy(cstr, ip_addr.c_str());
    // do stuff
    delete [] cstr;
    
    char *temp = strtok(cstr,"."); //공백을 기준으로 문자열 자르기
    
    while (temp != NULL) { //널이 아닐때까지 반복
        printf("%s\n",temp); // 출력
        temp = strtok(NULL, " ");	//널문자를 기준으로 다시 자르기
    }

    string ip_mask = temp;

    cout<<"searching for p2p server."<<endl;
    string name_server_ip_addr = search_nameserver(ip_mask, ip_addr, port_nameserver);
    if(name_server_ip_addr == ""){
        ip_addr_p2p_server = ip_addr;
        cout<<"p2p server is not found, and p2p server mode is activated."<<endl;
        thread beacon_thread(beacon_nameserver, (ip_addr, port_nameserver));
        beacon_thread.join();
        cout<<"p2p beacon server is activated." << endl;

        thread db_thread(user_manager_nameserver, (ip_addr, port_subscribe));
        db_thread.join();
        cout<<"p2p subscriber database server is activated."<<endl;

        thread relay_thread(relay_server_nameserver, (ip_addr, port_chat_publisher, port_chat_collector));
        relay_thread.join();
        cout<<"p2p message relay server is activated." <<endl;        
    } else{
        ip_addr_p2p_server = name_server_ip_addr;
        cout<<"p2p server found at "<<ip_addr_p2p_server<< ", and p2p client mode is activated."<<endl;
    }
    cout<<"starting user registration procedure."<<endl;

    zmq::context_t db_client_context;
    zmq::socket_t db_client_socket(db_client_context, zmq::socket_type::req);
    zmq::message_t db_msg_t;
    db_client_socket.connect("tcp://"+ip_addr_p2p_server+":"+port_subscribe);
    string db_msg = ip_addr + ":" + user_name;
    memcpy(db_msg_t.data(), db_msg.data(), db_msg.size());
    db_client_socket.send(db_msg_t);

    db_client_socket.recv(db_msg_t);
    if(string(static_cast<char*>(db_msg_t.data()), db_msg_t.size()) == "ok"){
        cout<<"user registration to p2p server completed"<<endl;
    } else {
        cout<<"user registration to p2p server failed"<<endl;
    }
    cout<<"starting message transfer procedure"<<endl;

    zmq::context_t relay_client;
    zmq::socket_t p2p_rx(relay_client, zmq::socket_type::sub);
    p2p_rx.setsockopt(ZMQ_SUBSCRIBE, "b'RELAY");
    p2p_rx.connect("tcp://"+ip_addr_p2p_server+ ":"+port_chat_publisher);
    zmq::socket_t p2p_tx(relay_client, zmq::socket_type::push);
    p2p_tx.connect("tcp://"+ip_addr_p2p_server+ ":"+port_chat_publisher);

    cout<<"starting autonomous message transmit and receive scenario"<<endl;
    
    zmq::pollitem_t p[] = {
            {p2p_rx, 0, ZMQ_POLLIN, 0}
        };
    zmq::message_t p2p_msg;
    srand((unsigned)time(NULL));

    while(1){
        int rc = zmq::poll(p,1,1);
        cout<<"rc:: "<<rc<<endl;
        
        if(p[0].revents & ZMQ_POLLIN){
            p2p_rx.recv(p2p_msg);
            cout<<"p2p-recv::<<==" << string(static_cast<char*>(p2p_msg.data()), p2p_msg.size())<<endl;
        } else {
            int randNum = rand() % 100 + 1;
            if(randNum <10){
                usleep(3000000);
                string msg = "("+user_name+","+ip_addr+":ON)";
                memcpy(p2p_msg.data(), msg.data(), msg.size());
                p2p_tx.send(p2p_msg);
                cout<<"p2p-sed::==>> " << msg<<endl;
            } else{
                usleep(3000000);
                string msg = "("+user_name+","+ip_addr+":OFF)";
                memcpy(p2p_msg.data(), msg.data(), msg.size());
                p2p_tx.send(p2p_msg);
                cout<<"p2p-send::==>> " << msg<<endl;
            }
        }
    }
    cout<<"closing p2p chatting program"<<endl;
    db_client_socket.close();
    p2p_rx.close();
    p2p_tx.close();
    db_client_context.~context_t();
    relay_client.~context_t();

    return 0;
}

string get_local_ip(){
    struct sockaddr_in sock_addr;
    int sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(80);
    sock_addr.sin_addr.s_addr = inet_addr("8.8.8.8");
    try{
        connect(sock,(struct sockaddr *)&sock_addr, sizeof(sock_addr));
        socklen_t size = sizeof(sock_addr);

        return to_string(getsockname(sock,(struct sockaddr *)&sock_addr, &size));
    } catch(char*){
        return "127.0.0.1";
    } 
    close(sock);
}

string search_nameserver(string ip_mask, string local_ip_addr, string port_nameserver){
    zmq::context_t context;
    zmq::socket_t req = zmq::socket_t(context, zmq::socket_type::sub);

    zmq::message_t msg;
    for (int i=0; i<255; i++){
        string target_ip_addr = "tcp://"+ip_mask+"." + to_string(i) + port_nameserver;
        if (target_ip_addr != local_ip_addr || target_ip_addr == local_ip_addr){
            req.connect(target_ip_addr);
        }
        req.setsockopt(ZMQ_RCVTIMEO, 2000);
        req.setsockopt(ZMQ_SUBSCRIBE, "NAMESERVER");
    }

    req.recv(msg);
    string res = string(static_cast<char*>(msg.data()), msg.size());
    bool chk = true;
    string nameserver = "NAMESERVER";
    if(res.size() >9){
        for (int i=0; i<10; i++){
        if(res[i] != nameserver[i])
            chk = false;
        }
    } else{
        return "";
    }
    if (chk){
        return res;
    }
    return "";

}

void beacon_nameserver(string local_ip_addr, string port_nameserver){
    zmq::context_t context;
    zmq::socket_t socket(context, zmq::socket_type::pub);
    zmq::message_t msg;
    socket.bind("tcp://"+local_ip_addr+":"+port_nameserver);
    while(1){
        usleep(1000000);
        string message = "NAMESERVER:"+local_ip_addr;
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
        cout<<"user registration "<< user_req;
        memcpy(msg.data(), sendMsg.data(), sendMsg.size());
        socket.send(msg);
    }
}

void relay_server_nameserver(string local_ip_addr, string port_chat_publisher, string port_chat_collector){
    zmq::context_t context;
    zmq::socket_t publisher(context, zmq::socket_type::pub);
    publisher.bind("tcp://"+local_ip_addr+":"+port_chat_collector);
    zmq::socket_t collector(context, zmq::socket_type::pull);
    collector.bind("tcp://"+local_ip_addr+":"+port_chat_collector);
    cout<<"local p2p relay server activated at tcp://" << local_ip_addr<<":"<<port_chat_publisher<<port_chat_collector<<endl;
    zmq::message_t msg;
    while(1){
        collector.recv(msg);
        string message = string(static_cast<char*>(msg.data()), msg.size());
        cout<<"p2p-relay:<==>"<< message<<endl;
        message = "REALY:" + message;
        memcpy(msg.data(), message.data(), message.size());
        publisher.send(msg);
    }

}