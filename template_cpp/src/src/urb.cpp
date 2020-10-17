#include "urb.hpp"

UniformBroadcast::UniformBroadcast(Parser::Host localhost, std::vector<Parser::Host> peers, std::function<void(Packet)> urbDeliver){
    std::function<void(Packet)> callback = [this](Packet pkt){this->bebDeliver(pkt);};
    beb = new BeBroadcast(localhost, peers, callback);   
    this->urbDeliver = urbDeliver;
    for(auto &&peer: peers){
        correctProcesses.insert(peer.id);
    }
}


void UniformBroadcast::crash(size_t processID){
    correctProcesses.erase(processID);
}

void UniformBroadcast::broadcast(Packet pkt){
    addToForwarded(pkt);
    beb->bebBroadcast(pkt);
}

void UniformBroadcast::bebDeliver(Packet pkt){
   storeAck(pkt);
   if(!isForwarded(pkt)){
       addToForwarded(pkt);
       beb->bebBroadcast(pkt);
   }
   tryDelivery(pkt);
}

bool UniformBroadcast::isForwarded(Packet pkt){
    std::vector<Packet>::iterator match = std::find_if(forwarded.begin(), forwarded.end(), [pkt](Packet p){return pkt.peerID == p.peerID && pkt.payload == p.payload;});
    return match != forwarded.end();
}

void UniformBroadcast::addToForwarded(Packet pkt){
    forwarded.push_back(pkt);
}
void UniformBroadcast::addToDelivered(Packet pkt){
    delivered.push_back(pkt);
}
bool UniformBroadcast::isDelivered(Packet pkt){
    std::vector<Packet>::iterator match = std::find_if(delivered.begin(), delivered.end(), [pkt](Packet p){return pkt.peerID == p.peerID && pkt.payload == p.payload;});
    return match != delivered.end();
}
void UniformBroadcast::storeAck(Packet pkt){
    acks.push_back(pkt);
}

bool UniformBroadcast::receivedAllAcks(Packet pkt){
    std::vector<size_t> senders;
    for (auto &&ackPacket : acks){
        if(ackPacket.peerID == pkt.peerID && ackPacket.payload == pkt.payload){
            senders.push_back(ackPacket.senderID);
        }
    }
    bool receivedAll = true;
    for(auto &&processID: correctProcesses){
        std::vector<size_t>::iterator match = std::find(senders.begin(), senders.end(), processID);
        receivedAll = receivedAll && match != senders.end();
    }
    return receivedAll;
}

void UniformBroadcast::tryDelivery(Packet pkt){
    if(receivedAllAcks(pkt) && isForwarded(pkt) && !isDelivered(pkt)){
        this->urbDeliver(pkt);
    }
}
