#include "fifob.hpp"

FIFOBroadcast::FIFOBroadcast(Parser::Host localhost, std::vector<Parser::Host> peers, std::function<void(Packet)> fifoDeliver){
    std::function<void(Packet)> callback = [this](Packet pkt){this->urbDeliver(pkt);};
    this->urb = new UniformBroadcast(localhost, peers, callback);
    this->fifoDeliver = fifoDeliver;
    for(auto &&peer: peers){
        nextToDeliver.insert(std::pair<int, int>(peer.id, 1));
        pendingSorted.insert(std::pair<int, std::vector<Packet>>(peer.id, std::vector<Packet>()));
    }
        pendingSorted.insert(std::pair<int, std::vector<Packet>>(localhost.id, std::vector<Packet>()));
    nextToDeliver.insert(std::pair<int, int>(localhost.id, 1));
}

void FIFOBroadcast::broadcast(Packet pkt){
    urb->broadcast(pkt);
}

void FIFOBroadcast::addToPending(Packet pkt){
   /* std::vector<Packet>::iterator match = std::find_if(pendingSorted.at(pkt.peerID).begin(), pendingSorted.at(pkt.peerID).end(), [pkt](Packet p){return pkt.peerID == p.peerID && pkt.payload < p.payload;});
    if( pendingSorted.at(pkt.peerID).size() == 0){
        pendingSorted.at(pkt.peerID).push_back(pkt);

    }else{
        if(match != pendingSorted.at(pkt.peerID).end()){
            pendingSorted.at(pkt.peerID).insert(match, pkt);
        }else{
            pendingSorted.at(pkt.peerID).push_back(pkt);

        }
    }
*/
    size_t i = 0;
    if(pendingSorted.at(pkt.peerID).size() == 0){
       pendingSorted.at(pkt.peerID).push_back(pkt);
    }else{
        bool inserted = false;
        for(i= 0; i < pendingSorted.at(pkt.peerID).size()  ; i++){ 
                if(pkt.payload < pendingSorted.at(pkt.peerID).at(i).payload){
                    std::vector<Packet>::iterator it = pendingSorted.at(pkt.peerID).begin();
                    pendingSorted.at(pkt.peerID).insert(it + (i ), pkt);
                    inserted= true;
                    break;
                }
        }
       if(!inserted){
           pendingSorted.at(pkt.peerID).push_back(pkt);
       }
    }  
     
}

std::vector<Packet>  FIFOBroadcast::pendingOrdered(int processID){
    return pendingSorted.at(processID);
}

void FIFOBroadcast::urbDeliver(Packet pkt){
    lock.lock();
    addToPending(pkt);
    int seqNb = nextToDeliver.at(pkt.peerID);
    for(size_t i = 0; i < pendingSorted.at(pkt.peerID).size() &&  pendingSorted.at(pkt.peerID).size() > 0; i++){ 
        Packet pendingPacket = pendingSorted.at(pkt.peerID).at(i);
        if(pendingPacket.peerID != pkt.peerID){
            //std::cerr <<  "Invalid id " << pendingPacket.peerID << " " << pkt.peerID << std::endl;
            //throw std::invalid_argument("Invalid id ");

        }
        if(pendingPacket.payload ==seqNb){
            seqNb += 1;
            fifoDeliver(pendingPacket);
  
        }
    }
    
    nextToDeliver[pkt.peerID] = seqNb;
    lock.unlock();
}


