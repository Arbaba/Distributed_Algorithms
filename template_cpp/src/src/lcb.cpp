#include "lcb.hpp"
#include "packet.hpp"
LCBroadcast::LCBroadcast(Parser::Host localhost,
                                std::vector<Parser::Host> peers, 
                                std::function<void(Packet)> lcbDeliver,
                                std::function<void(Packet)> broadcastCB_,
                                Coordinator* coordinator){
    this->lcbDeliver = lcbDeliver;
    //upon fifo delivery trigger LCB delivery
    std::function<void(Packet)> fifoCB = [this](Packet pkt){this->deliver(pkt);};
    this->localhost = localhost;
    //upon fifobroadcast deliver and then call broadcastCB
    this->broadcastCB = [this, broadcastCB_](Packet pkt){
        broadcastCB_(pkt);
        this->lcbDeliver(pkt);
        std::cerr << "Deliver " << pkt.toString() << std::endl;
        for(size_t i = 0; i < fifoController->vectorClock.size(); i++){
            std::cerr << i + 1 << " " << fifoController->vectorClock[i] << " " << pkt.vectorClock[i] << std::endl;
        }
        fifoController->vectorClock[pkt.peerID - 1] += 1;
    };
    this->fifoController = new FIFOController(localhost, peers, fifoCB, broadcastCB, coordinator);
    this->lcbDeliver = lcbDeliver;
    //Initialize vector clock including self (+ 1)
    for(size_t i = 0; i < peers.size() + 1; i++){
        this->fifoController->vectorClock.push_back(0);
    }
    //initialize pending
    for(auto& peer: peers){
        this->pending.insert(std::pair<int, std::vector<Packet>>(peer.id, std::vector<Packet>()));
    }
}
void LCBroadcast::broadcast(unsigned long n){
    fifoController->broadcast(n);
}


void LCBroadcast::deliver(Packet pkt){
    if(pkt.peerID != localhost.id){
        std::cout << "deliver " <<  pkt.toString() << std::endl;
        pending.at(pkt.peerID).push_back(pkt);
        bool checkPending = true;
        while(checkPending){
            checkPending = false;
            for(auto it = pending[pkt.peerID].begin(); it!= pending[pkt.peerID].end();){
                bool canDeliver = true;
                for(size_t i = 0; i < fifoController->vectorClock.size(); i++){
                    if(fifoController->vectorClock[i] < pkt.vectorClock[i]){
                        canDeliver = false;
                        break;
                    }
                }

                if(canDeliver){
                    Packet toDeliver = *it;
                    pending[pkt.peerID].erase(it);
                    std::cerr << "Deliver " << pkt.toString() << std::endl;
                    for(size_t i = 0; i < fifoController->vectorClock.size(); i++){
                        std::cerr << i << " " << fifoController->vectorClock[i] << " " << pkt.vectorClock[i] << std::endl;
                    }
             
                    lcbDeliver(toDeliver);
                    fifoController->vectorClock[pkt.peerID - 1] += 1;
                    checkPending = true;
                }else {
                    it++;
                }
                checkPending |= canDeliver;
            }
        }
    }
}

void LCBroadcast::stop(){
    fifoController->stop();
}