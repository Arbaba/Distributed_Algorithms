#include "urb.hpp"

UniformBroadcast::UniformBroadcast(in_addr_t ip, in_port_t port){

}


void UniformBroadcast::crash(int processID){

}

int UniformBroadcast::broadcast(const Packet *msg){

}

int UniformBroadcast::bebDeliver(Packet *pck){

}

bool UniformBroadcast::receivedAllAcks(int originID, int messageID){

}
bool UniformBroadcast::tryDelivery(int originID, int messageID){
    if(receivedAllAcks(originID, messageID)){
        return 
    }
}
int UniformBroadcast::deliver(Packet *pck){
    
}