#include "fifob.hpp"

FIFOBroadcast::FIFOBroadcast(Parser::Host localhost, std::vector<Parser::Host> peers, std::function<void(Packet)> fifoDeliver){
    std::function<void(Packet)> callback = [this](Packet pkt){this->urbDeliver(pkt);};
    this->urb = new UniformBroadcast(localhost, peers, callback);
    this->fifoDeliver = fifoDeliver;
    for(auto &&peer: peers){
        nextToDeliver.insert(std::pair<int, int>(peer.id, 1));
        pendingSorted.insert(std::pair<int, std::vector<Packet>>(peer.id, std::vector<Packet>()));
    }

    nextToDeliver.insert(std::pair<int, int>(localhost.id, 1));

}

void FIFOBroadcast::broadcast(Packet pkt){

    nextToDeliver.insert(std::pair<unsigned long, int>(pkt.peerID, nextToDeliver.at(pkt.peerID) + 1));
        //std::cout << "insert " << pkt.peerID << std::endl;

    //addToPending(pkt);
        //std::cout << "pending " << pkt.peerID << std::endl;

    urb->broadcast(pkt);
        //std::cout << "broadcasr " << pkt.peerID << std::endl;

}

void FIFOBroadcast::addToPending(Packet pkt){
    if(pendingSorted.at(pkt.peerID).size() == 0){
        pendingSorted.at(pkt.peerID).push_back(pkt);
    }else{
        for(size_t i = 0; i < pendingSorted.at(pkt.peerID).size() ; i++){ 
                if(pkt.payload < pendingSorted.at(pkt.peerID).at(i).payload){
                    std::vector<Packet>::iterator it = pendingSorted.at(pkt.peerID).begin();
                    //std::cout << "Inserted " << pkt.payload << " at " << i << std::endl;
                    pendingSorted.at(pkt.peerID).insert(it + (i ), pkt);
                    break;
                }
        }
    }    
}

std::vector<Packet>  FIFOBroadcast::pendingOrdered(int processID){
    return pendingSorted.at(processID);
}

void FIFOBroadcast::urbDeliver(Packet pkt){
    //std::cout << "Deliver " << pkt.peerID << std::endl;
    addToPending(pkt);
    int seqNb = nextToDeliver.at(pkt.peerID);
    

    for(size_t i = 0; i < pendingSorted.at(pkt.peerID).size(); i++){ 
        //std::cout << "try Deliver " << pkt.peerID << " " << pkt.payload << " " << seqNb << std::endl;

        Packet pendingPacket = pendingSorted.at(pkt.peerID).at((i ));
        //std::cout << "Payload" << pendingPacket.payload <<  std::endl;
        if(pendingPacket.payload ==seqNb){
            seqNb += 1;

            pendingSorted.at(pkt.peerID).erase(pendingSorted.at(pkt.peerID).begin());
            i--;
            fifoDeliver(pendingPacket);
        }else{
            break;
        }
    }
    nextToDeliver[pkt.peerID] = seqNb;
}

