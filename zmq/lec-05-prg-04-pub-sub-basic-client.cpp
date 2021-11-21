#include <string>
#include <iostream>
#include <zmq.hpp>
#include <vector>

using namespace std;

vector<string> split (string input, char delimiter);


int main(int argc, char *argv[]){
    zmq::context_t context;
    cout<<"Collecting updates from weather server..."<< endl;
    zmq::socket_t client(context, zmq::socket_type::sub);

    client.connect("tcp://127.0.0.1:5556");
    
    const char *filter = (argc > 1) ? argv[1]: "10001";
    client.setsockopt(ZMQ_SUBSCRIBE, filter, strlen(filter));
    
    zmq::message_t reply;
    long totalTemp = 0;
    vector<string> received;
    string zipcode;
    for (int i = 0; i < 20; i++){
        client.recv(reply);
        std::string rpl = std::string(static_cast<char*>(reply.data()), reply.size());
        received = split(rpl, ' ');
        zipcode = received[0];
        cout<<"Receive temperature for zipcode '"<< zipcode <<"' was "<< received[1]<<" F" << endl;
        totalTemp += stoi(received[1]);
    }
    cout<<"Average temperature for zipcode '"<< zipcode <<"' was "<< totalTemp/20 << " F"<<endl;
    return 0;
}

vector<string> split (string input, char delimiter){
    vector<string> answer;
    stringstream ss(input);
    string temp;

    while(getline(ss, temp, delimiter)) {
        answer.push_back(temp);
    }
    return answer;
}